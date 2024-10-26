#ifndef WEBSERVER_H
#define WEBSERVER_H

#include <httpserver.h>
#include <string>

void setup_server(bool secure, const std::string &cert, const std::string &key, uint16_t port, int threads);

#endif // WEB_SERVER_H
