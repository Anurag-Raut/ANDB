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
