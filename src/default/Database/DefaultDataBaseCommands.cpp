#include "DefaultDataBaseCommands.h"
#include <iostream>

namespace DefaultDataBaseCommands {

bool createTable(PostgreSQLConnection& conn)
{
    if (!conn.isConnected()) {
        std::cerr << "Not connected. Cannot create table." << std::endl;
        return false;
    }

    std::string query = R"(
        CREATE TABLE IF NOT EXISTS my_table (
            id SERIAL PRIMARY KEY,
            info TEXT NOT NULL
        )
    )";

    // Reuse PostgreSQLConnection::executeQuery
    return conn.executeQuery(query);
}

bool insertRecord(PostgreSQLConnection& conn, const std::string& value)
{
    if (!conn.isConnected()) {
        std::cerr << "Not connected. Cannot insert record." << std::endl;
        return false;
    }

    // For security, consider using parameterized queries (PQexecParams)
    // This is just a simple string example:
    std::string query = "INSERT INTO my_table (info) VALUES ('" + value + "');";
    return conn.executeQuery(query);
}

bool selectAll(PostgreSQLConnection& conn)
{
    if (!conn.isConnected()) {
        std::cerr << "Not connected. Cannot run SELECT." << std::endl;
        return false;
    }

    std::string query = "SELECT id, info FROM my_table;";

    // Execute query
    if (!conn.executeQuery(query)) {
        return false;
    }

    // If you need to process the returned rows, you can do it here.
    // The current executeQuery() does minimal work, but you could
    // modify PostgreSQLConnection::executeQuery to return a PGresult*
    // and process the rows in a new method.

    return true;
}

bool deleteRecord(PostgreSQLConnection& conn, int id)
{
    if (!conn.isConnected()) {
        std::cerr << "Not connected. Cannot delete record." << std::endl;
        return false;
    }

    // Again, parameterized queries are recommended. This is just an example.
    std::string query = "DELETE FROM my_table WHERE id = " + std::to_string(id) + ";";
    return conn.executeQuery(query);
}

} // namespace CommonDBCommands
