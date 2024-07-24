#include <cassert>
#include <chrono>  
#include <iostream>
#include <string>
#include <thread> 
#include <filesystem>
#include "storage/include/globals.hpp"
#include "storage/database.cpp"
#include "storage/transaction.cpp"
// #include "repl.cpp"
#include "btree_tests.cpp"

int main(){

    //statup

     namespace fs = std::filesystem;
    if (!fs::exists(BASE_DIRECTORY)) {
        if (!fs::create_directory(BASE_DIRECTORY)) {
            throw runtime_error("Failed to create directory: " + BASE_DIRECTORY);
        }
    }
    
    test();
    // repl();
    
    


    return 0;
}