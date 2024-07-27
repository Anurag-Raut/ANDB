#include <string>
#include <iostream>
#include <sstream>
#include "parser/interpreter.cpp"

using namespace std;

void databaserepl(Database *db) {
    Interpreter interpreter(db, std::cin);

}

void repl() {
    std::string input;
    std::cout << "ANDB" << std::endl;
        std::cout << "Available Commands:\n"
          << "------------------\n"
          << "CONNECT <database_name>\n"
          << "    Connects to the specified database. If the database does not exist, it will be created.\n"
          << "\n"
          << "EXIT\n"
          << "    Terminates the current session and shuts down the daemon.\n"
          << "\n";
    
    while (true) {
        std::cout << "> ";
        std::getline(std::cin, input);
        std::istringstream iss(input);
        std::string command, key, value;

        iss >> command;

        if (command == "EXIT") {
            break;
        } else if (command == "CONNECT") {
            iss >> key;
            Database *db =new Database(key);
            databaserepl(db);
            
        } 
    }

    return ;
}
