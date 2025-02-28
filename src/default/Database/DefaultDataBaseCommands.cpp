#include "DefaultDataBaseCommands.h"
#include <iostream>

namespace DefaultDataBaseCommands {

bool createTable(PostgreSQLConnection& conn)
{
    if (!conn.isConnected()) {
        std::cerr << "Not connected. Cannot create table." << std::endl;
        return false;
    }

    std::string query = file.read("init.sql");

    // Reuse PostgreSQLConnection::executeQuery
    return conn.executeQuery(query);
}


} // namespace CommonDBCommands
