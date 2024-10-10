#ifndef WEBSERVER_H
#define WEBSERVER_H

#include <httpserver.h>
#include <string>
#include <memory>
#include "Configuration.h"

void setup_server(bool secure, const std::string &cert, const std::string &key, uint16_t port, int threads);

#endif // WEBSERVER_H
