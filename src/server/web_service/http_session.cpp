#include "http_session.h"
#include "http_error_handlers.h"
#include <boost/asio.hpp>
#include <boost/beast/core.hpp>
#include <boost/shared_ptr.hpp>
#include <functional>
#include <map>
#include <string>
#include <utility>

http_session::http_session(
    const boost::shared_ptr<web_service_context> &ctx,
    tcp::socket socket,
    std::function<void(boost::system::error_code, const char *)> error_handler,
    const std::map<std::string, RouteHandler> &route_handlers)
    : socket_(std::move(socket)),
      error_handler_(std::move(error_handler)),
      route_handlers_(route_handlers),
      strand_(boost::asio::make_strand(socket_.get_executor())),
      timer_(socket_.get_executor()),
      queue_(*this),
      ctx_(ctx)
{
}

void http_session::run()
{
  on_timer({});
  do_read();
}

void http_session::do_read()
{
  timer_.expires_after(std::chrono::seconds(15));

  http::async_read(socket_, buffer_, req_,
                   boost::asio::bind_executor(
                       strand_,
                       std::bind(
                           &http_session::on_read,
                           shared_from_this(),
                           std::placeholders::_1)));
}

void http_session::on_timer(boost::system::error_code ec)
{
  if (ec && ec != boost::asio::error::operation_aborted)
    return error_handler_(ec, "timer");

  if (timer_.expiry() <= std::chrono::steady_clock::now())
  {
    socket_.shutdown(tcp::socket::shutdown_both, ec);
    socket_.close(ec);
    return;
  }

  timer_.async_wait(
      boost::asio::bind_executor(
          strand_,
          std::bind(
              &http_session::on_timer,
              shared_from_this(),
              std::placeholders::_1)));
}

void http_session::on_read(boost::system::error_code ec)
{
  if (ec == boost::asio::error::operation_aborted)
    return;

  if (ec == http::error::end_of_stream)
    return do_close();

  if (ec)
    return error_handler_(ec, "read");

  handle_request(std::move(req_), queue_);

  if (!queue_.is_full())
    do_read();
}

void http_session::on_write(boost::system::error_code ec, bool close)
{
  if (ec == boost::asio::error::operation_aborted)
    return;

  if (ec)
    return error_handler_(ec, "write");

  if (close)
  {
    return do_close();
  }

  if (queue_.on_write())
  {
    do_read();
  }
}

void http_session::do_close()
{
  boost::system::error_code ec;
  socket_.shutdown(tcp::socket::shutdown_send, ec);
}

template <class Body, class Allocator, class Send>
void http_session::handle_request(
    http::request<Body, http::basic_fields<Allocator>> &&req,
    Send &&send)
{
  BOOST_LOG_TRIVIAL(info) << req.method() << "\t" << req.target().to_string();

  auto auth_it = req.base().find(http::field::authorization);
  std::string auth_header;

  if (auth_it != req.base().end())
  {
    auth_header = auth_it->value().to_string();
  }

  if (!ctx_->get_authorization()->is_authorized(auth_header))
  {
    return send(http_error_handlers::unauthorized());
  }

  if (req.method() != http::verb::get &&
      req.method() != http::verb::head &&
      req.method() != http::verb::post)
    return send(http_error_handlers::bad_request("Unknown HTTP-method"));

  std::string path = req.target().to_string();
  std::map<std::string, std::string> queryParams = util::http::getQueryParameters(path);
  std::string route = util::http::getRoute(path);
  auto iter = route_handlers_.find(route);

  if (iter != route_handlers_.end())
  {
    std::string s = req.body();
    return send(iter->second(ctx_, queryParams, s));
  }

  return send(http_error_handlers::not_found(req.target()));
}
