/**
 * @Authors         Sebastiaan den Hertog
 * @Date created    15-08-2024
 * @Date updated    24-08-2024 (By: Sebastiaan den Hertog)
 * @Description     constuctor, destructor and methods for the webServer class
 */

#ifndef WEBSERVER_H
#define WEBSERVER_H

#include <httpserver.h>
#include <string>
#include <memory>
#include "Configuration.h"

void setup_server(bool secure, const std::string &cert, const std::string &key, uint16_t port, int threads, bool use_server);

#endif // WEBSERVER_H
