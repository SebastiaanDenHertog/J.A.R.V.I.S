#ifndef DEFAULT_DATABASE_COMMANDS_H
#define DEFAULT_DATABASE_COMMANDS_H

#include "PostgreSQLConnection.h"
#include <string>

namespace CommonDBCommands {

    // Create a table
    bool createTable(PostgreSQLConnection& conn);

    // Insert a record
    bool insertRecord(PostgreSQLConnection& conn, const std::string& value);

    // Select data
    bool selectAll(PostgreSQLConnection& conn);

    // Delete a record
    bool deleteRecord(PostgreSQLConnection& conn, int id);

} // namespace CommonDBCommands

#endif // COMMON_DB_COMMANDS_H
