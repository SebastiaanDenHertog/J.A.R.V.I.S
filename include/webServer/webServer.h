#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include <string>

void setup_server(bool secure, const std::string &cert, const std::string &key, uint16_t port, int threads);

#endif // WEB_SERVER_H
