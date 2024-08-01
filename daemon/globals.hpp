
#pragma once;
#include <filesystem>
#include <string>
#include <cstdint> 
#include <iostream>
#include<vector>
#include<fstream>
#include <mutex>
#include <map>
#include "utils/orderedLock.cpp"

using namespace std;
extern const unsigned int PAGE_SIZE = 200;  

const std::string BASE_DIRECTORY = std::filesystem::current_path().string() + "/base/";

extern const int HEADER_SIZE = sizeof(uint32_t) ; // Page ID 
extern const int OFFSET_SIZE=sizeof(uint16_t);
extern const size_t Block_HEADER_SIZE=(2*sizeof(uint16_t)) + sizeof(uint64_t)+sizeof(uint64_t);
extern uint64_t TRANSACTION_ID=1;
vector<uint64_t> active_transactions;
map<string,vector<uint64_t>> rowLocks;
map<string,uint64_t> prevTransactionId;
OrderedLock orderedLock;


class TransactionException : public std::runtime_error {
public:
    TransactionException(const std::string& message) : std::runtime_error(message) {}
};

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
std::string getTransactionLogFilePath(std::string database_name) {
        string filepath=BASE_DIRECTORY+database_name;

    namespace fs = std::filesystem;
    if (!fs::exists(filepath)) {
        if (!fs::create_directory(filepath)) {
            throw runtime_error("Failed to create directory: " + filepath);
        }
    }
    return BASE_DIRECTORY + database_name  +  "/transaction_log";
}
std::string getWALFilePath(std::string database_name) {
        string filepath=BASE_DIRECTORY+database_name;

    namespace fs = std::filesystem;
    if (!fs::exists(filepath)) {
        if (!fs::create_directory(filepath)) {
            throw runtime_error("Failed to create directory: " + filepath);
        }
    }
    return BASE_DIRECTORY + database_name  +  "/wal";
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

vector<string> splitString(const string& str, char delimiter) {
    vector<string> tokens;
    string token;
    stringstream ss(str);

    while (getline(ss, token, delimiter)) {
        tokens.push_back(token);
    }

    return tokens;
}
void createFile(const std::string& fileName) {
    // Try to open the file for reading
    std::ifstream infile(fileName);
    
    if (infile.is_open()) {
        // File exists and is open for reading
        // std::cout << "File already exists: " << fileName << std::endl;
        infile.close();
    } else {
        // File does not exist, so create it
        std::ofstream outfile(fileName);
        if (outfile.is_open()) {
            // std::cout << "File created successfully: " << fileName << std::endl;
            outfile.close();
        } else {
            std::cerr << "Failed to create the file: " << fileName << std::endl;
        }
    }
}





struct Page {
    uint32_t no_of_rows;
    uint32_t *row_offsets;
    char *data;
};