#include <iostream>
#include <sstream>
#include <string>
#include <sys/stat.h> 
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include "parser/interpreter.cpp"

using namespace std;

void databaserepl(Database *db, int client_socket) { Interpreter interpreter(db, client_socket); }

void repl(int client_socket) {
    int n;
    char buffer[200000];
    std::string input;
    const char *welcome_message =
        "ANDB\n"
        "Available Commands:\n"
        "------------------\n"
        "CONNECT <database_name>\n"
        "    Connects to the specified database. If the database does not exist, it will be created.\n"
        "\n"
        "EXIT\n"
        "    Terminates the current session and shuts down the daemon.\n"
        "\n";
    write(client_socket, welcome_message, strlen(welcome_message));

    while ((n = read(client_socket, buffer, sizeof(buffer) - 1)) > 0) {
        buffer[n] = '\0';  // Null-terminate the buffer

        std::istringstream iss(buffer);
        std::string command, key;
        iss >> command;

        if (command == "EXIT") {
            break;
        } else if (command == "CONNECT") {
            iss >> key;
            if (!key.empty()) {
                // Assuming Database and databaserepl are defined elsewhere
                Database *db = new Database(key);
                string database_connect="Successfully connected to Database: "+ key;
                write(client_socket, database_connect.c_str(), database_connect.size());
                databaserepl(db, client_socket);
                
            } else {
                const char *error = "Error: No database name provided.\n";
                write(client_socket, error, strlen(error));
            }
        } 
    }
}
