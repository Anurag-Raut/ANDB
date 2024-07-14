
#pragma once;
#include <filesystem>
#include <string>
#include <cstdint> 
#include <iostream>

using namespace std;
extern const unsigned int PAGE_SIZE = 200;  
extern const string BASE_DIRECTORY = "base/";

extern const int HEADER_SIZE = sizeof(uint32_t) ; // Page ID 
extern const int OFFSET_SIZE=sizeof(uint16_t);



std::string getDatabaseFilePath(std::string database_name) {
    string filepath=BASE_DIRECTORY+database_name;
    namespace fs = std::filesystem;
    if (!fs::exists(filepath)) {
        if (!fs::create_directory(filepath)) {
            throw runtime_error("Failed to create directory: " + filepath);
        }
    }
    return BASE_DIRECTORY + database_name  +  "/data";
}
std::string getMetadataFilePath(std::string database_name) {
        string filepath=BASE_DIRECTORY+database_name;

    namespace fs = std::filesystem;
    if (!fs::exists(filepath)) {
        if (!fs::create_directory(filepath)) {
            throw runtime_error("Failed to create directory: " + filepath);
        }
    }
    return BASE_DIRECTORY + database_name  +  "/metadata";
}
std::string getPageFilePath(std::string database_name) {
        string filepath=BASE_DIRECTORY+database_name;

    namespace fs = std::filesystem;
    if (!fs::exists(filepath)) {
        if (!fs::create_directory(filepath)) {
            throw runtime_error("Failed to create directory: " + filepath);
        }
    }
    return BASE_DIRECTORY + database_name  +  "/page";
}

std::string getIndexFilePath(std::string database_name,std::string table_name,std::string index_name){

    string filepath=BASE_DIRECTORY+database_name+"/"+table_name+"/indexes";

    namespace fs = std::filesystem;
    if (!fs::exists(filepath)) {
        if (!fs::create_directory(filepath)) {
            throw runtime_error("Failed to create directory: " + filepath);
        }
    }
    return filepath + "/"+ index_name;
}



struct Page {
    uint32_t no_of_rows;
    uint32_t *row_offsets;
    char *data;
};