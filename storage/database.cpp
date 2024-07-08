#include <string>
#include <fstream>
#include "../include/globals.hpp"
#include "../include/database.hpp"
#include <iostream>
#include "../storage/table.cpp"
using namespace std;



Database::Database(string name) {
    string dataFilePath = getDatabaseFilePath(name);
    string metadataFilePath = getMetadataFilePath(name);

    // Debugging output
    cout << "Attempting to create/open files at: " << dataFilePath << " and " << metadataFilePath << endl;

    // Ensure directories exist
    std::filesystem::path dataFileDir = std::filesystem::path(dataFilePath).parent_path();
    std::filesystem::path metadataFileDir = std::filesystem::path(metadataFilePath).parent_path();

    if (!std::filesystem::exists(dataFileDir)) {
        std::filesystem::create_directories(dataFileDir);
    }
    if (!std::filesystem::exists(metadataFileDir)) {
        std::filesystem::create_directories(metadataFileDir);
    }

    data_file.open(dataFilePath, ios::out);
    metadata_file.open(metadataFilePath, ios::out);

    // Check if files are opened
    if (data_file.is_open()) {
        cout << "DATABASE CREATED." << std::endl;
    } else {
        cerr << "Error: Failed to open data file: " << dataFilePath << endl;
        throw std::runtime_error("Failed to create database file: " + dataFilePath);
    }

    if (metadata_file.is_open()) {
        cout << "Metadata file CREATED." << std::endl;
    } else {
        cerr << "Error: Failed to open metadata file: " << metadataFilePath << endl;
        throw std::runtime_error("Failed to create metadata file: " + metadataFilePath);
    }
}



void Database::CreateTable(string table_name,vector<string> types,vector<string> names){

    Table *newTable=new Table(table_name,types,names);
    metadata_file<<table_name;
    metadata_file.flush();
    

}
