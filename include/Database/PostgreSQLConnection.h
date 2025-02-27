#ifndef POSTGRESQL_CONNECTION_H
#define POSTGRESQL_CONNECTION_H

#include <string>
#include <iostream>
#include <libpq-fe.h>
#include "Configuration.h"

struct PostgreSQLConnection {
protected:
    PGconn* conn;  

public:
    // Constructor: attempts to connect to the PostgreSQL database
    explicit PostgreSQLConnection(const std::string& conninfo)
        : conn(nullptr)
    {
        conn = PQconnectdb(conninfo.c_str());
        if (PQstatus(conn) != CONNECTION_OK) {
            std::cerr << "Connection to database failed: " 
                      << PQerrorMessage(conn) << std::endl;
            PQfinish(conn);
            conn = nullptr;
            // You may want to throw an exception or handle error differently
        } else {
            std::cout << "Connected to PostgreSQL successfully." << std::endl;
        }
    }

    // Virtual destructor: ensures proper cleanup if inherited
    virtual ~PostgreSQLConnection() {
        if (conn) {
            PQfinish(conn);
            conn = nullptr;
        }
    }

    // Check if the connection is valid/open
    bool isConnected() const {
        return conn && PQstatus(conn) == CONNECTION_OK;
    }

};

#endif // POSTGRESQL_CONNECTION_H
