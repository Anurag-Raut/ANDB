

#include "repl.cpp"
#include "btree_tests.cpp"
#include <filesystem>
#include "storage/include/globals.hpp"
#include "parser/interpreter.cpp"


int main(){

    //statup

    //  namespace fs = std::filesystem;
    // if (!fs::exists(BASE_DIRECTORY)) {
    //     if (!fs::create_directory(BASE_DIRECTORY)) {
    //         throw runtime_error("Failed to create directory: " + BASE_DIRECTORY);
    //     }
    // }
    
    // test();
    // repl();
    
    Interpreter interpreter;
    


    return 0;
}