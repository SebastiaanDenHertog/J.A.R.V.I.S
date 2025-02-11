#ifndef POSTGRESQL_CONNECTION_H
#define POSTGRESQL_CONNECTION_H

#include <string>
#include <iostream>
#include <libpq-fe.h>

struct PostgreSQLConnection {
protected:
    PGconn* conn;  // Protected so derived classes can use it

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

    // Optionally, provide a method to execute a simple query
    // (for demonstration purposes)
    bool executeQuery(const std::string& query) {
        if (!isConnected()) {
            std::cerr << "Cannot execute query: Not connected to the database."
                      << std::endl;
            return false;
        }

        PGresult* res = PQexec(conn, query.c_str());
        if (PQresultStatus(res) != PGRES_COMMAND_OK &&
            PQresultStatus(res) != PGRES_TUPLES_OK) {
            std::cerr << "Query failed: " << PQerrorMessage(conn) << std::endl;
            PQclear(res);
            return false;
        }

        // If you need to process results, you can do so here
        // For now, just show how many rows are returned (if applicable)
        int rows = PQntuples(res);
        std::cout << "Query executed, " << rows << " rows returned." << std::endl;

        PQclear(res);
        return true;
    }
};

#endif // POSTGRESQL_CONNECTION_H
