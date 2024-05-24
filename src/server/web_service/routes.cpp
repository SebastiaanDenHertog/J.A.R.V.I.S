#include "stdafx.h"

#include <boost/beast/core/buffers_to_string.hpp>

#include "common.h"
#include "http_auth_client.h"
#include "routes.h"

namespace routes
{
  http::response<http::string_body> home(const boost::shared_ptr<web_service_context> &ctx, QueryParameters queryParameters, const std::string &body)
  {
    http::response<http::string_body> res;
    res.version(HTTP_11);
    res.result(http::status::ok);
    res.set(http::field::server, HTTP_SERVER_VERSION);
    res.set(http::field::content_type, "text/html");
    res.body() = "Welcome Home!";
    res.prepare_payload();
    return res;
  }

  http::response<http::string_body> livecheck_handler(const boost::shared_ptr<web_service_context> &ctx, QueryParameters queryParameters, const std::string &body)
  {
    http::response<http::string_body> res;
    res.version(HTTP_11);
    res.result(http::status::ok);
    res.set(http::field::server, HTTP_SERVER_VERSION);
    res.set(http::field::content_type, "text/html");
    res.body() = "Livecheck OK!";
    res.prepare_payload();
    return res;
  }

  http::response<http::string_body> file_handler(const boost::shared_ptr<web_service_context> &ctx, QueryParameters queryParameters, const std::string &body)
  {
    http::response<http::string_body> res;
    res.version(HTTP_11);
    res.result(http::status::ok);
    res.set(http::field::server, HTTP_SERVER_VERSION);

    res.set(http::field::content_type, "text/html");

    // Read the contents of the file
    std::ifstream file("./webserver/private/pages/home.html");
    if (file.is_open())
    {
      std::stringstream buffer;
      buffer << file.rdbuf();
      res.body() = buffer.str();
      file.close();
    }
    else
    {
      res.body() = "Error: File not found";
    }

    res.prepare_payload();
    return res;
  }

  http::response<http::string_body> api_client_handler(const boost::shared_ptr<web_service_context> &ctx, QueryParameters queryParameters, const std::string &body)
  {
    auto const host = "raw.githubusercontent.com";
    auto const port = "443";
    auto const api_key = "somehash";
    auto const target = "/mitant/cppweb/master/README.md";
    std::string auth_header;
    auth_header += api_key;

    http_auth_client client(ctx, host, port, auth_header);

    http::response<http::string_body> res;
    res.version(HTTP_11);
    res.result(http::status::ok);
    res.set(http::field::server, HTTP_SERVER_VERSION);
    res.set(http::field::content_type, "text/plain");
    res.body() = buffers_to_string(client.get(target).body().data());
    res.prepare_payload();
    return res;
  }
  http::response<http::string_body> no_page_found(const boost::shared_ptr<web_service_context> &ctx, QueryParameters queryParameters, const std::string &body)
  {
    http::response<http::string_body> res;
    res.version(HTTP_11);
    res.result(http::status::ok);
    res.set(http::field::server, HTTP_SERVER_VERSION);
    res.set(http::field::content_type, "text/html");
    res.body() = "No page found!";
    res.prepare_payload();
    return res;
  }

}