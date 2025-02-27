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


} // namespace CommonDBCommands
