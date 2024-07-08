#include <string>
#include <iostream>
#include <sstream>
#include "storage/database.cpp"

using namespace std;

void databaserepl(Database *db) {
    std::string input;
    std::cout << "ANDB" << std::endl;
    std::cout << "Commands: SET key value, GET key, DEL key, EXIT" << std::endl;
   
    while (true) {
        std::cout << "> ";
        std::getline(std::cin, input);
        std::istringstream iss(input);
        std::string command, key, value;

        iss >> command;

        if (command == "EXIT") {
            break;
        } else if (command == "SET") {
            iss >> key;
            std::getline(iss >> std::ws, value);
            db->insert(key, value);
            std::cout << "OK" << std::endl;
        } else if (command == "GET") {
            iss >> key;
            std::cout << db->search(key) << std::endl;
        } else if (command == "DEL") {
            iss >> key;
            // db->remove(key);
            std::cout << "OK" << std::endl;
        } else {
            std::cout << "Unknown command" << std::endl;
        }
    }

    return ;
}

void repl() {
    std::string input;
    std::cout << "ANDB" << std::endl;
    std::cout << "Commands: SET key value, GET key, DEL key, EXIT" << std::endl;
    while (true) {
        std::cout << "> ";
        std::getline(std::cin, input);
        std::istringstream iss(input);
        std::string command, key, value;

        iss >> command;

        if (command == "EXIT") {
            break;
        } else if (command == "CREATE") {
            iss >> key;
            Database *db =new Database(key);
            databaserepl(db);
            
        } 
    }

    return ;
}
