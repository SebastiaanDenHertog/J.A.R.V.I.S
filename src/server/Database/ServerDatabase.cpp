

#include "PostgreSQLConnection.h"
#include <string>
#include <iostream>

struct ServerDatabase : public PostgreSQLConnection {
public:
    explicit ServerDatabase(const std::string& conninfo)
        : PostgreSQLConnection(conninfo)
    {
        // Additional server-side setup can go here
        if (isConnected()) {
            std::cout << "ServerDatabase is ready to run queries." << std::endl;
        }
    }

    // Example specialized method
    void createTable() {
        // Some server logic
        const std::string query = R"(
            CREATE TABLE IF NOT EXISTS my_table (
                id SERIAL PRIMARY KEY,
                name TEXT NOT NULL
            )
        )";
        if (!executeQuery(query)) {
            std::cerr << "Failed to create table." << std::endl;
        } else {
            std::cout << "Table created/exists already." << std::endl;
        }
    }
};
