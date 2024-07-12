#include "../storage/table.cpp"
#include "../include/globals.hpp"
#include "../include/database.hpp"
#include <string>
#include <fstream>
#include <iostream>

using namespace std;



Database::Database(string name) {
    string dataFilePath = getDatabaseFilePath(name);
    string metadataFilePath = getMetadataFilePath(name);
    string pageFilePath = getPageFilePath(name);
    this->name=name;
    // Debugging output
    cout << "Attempting to create/open files at: " << dataFilePath << " and " << metadataFilePath << endl;

    // Ensure directories exist
    std::filesystem::path dataFileDir = std::filesystem::path(dataFilePath).parent_path();
    std::filesystem::path metadataFileDir = std::filesystem::path(metadataFilePath).parent_path();
    std::filesystem::path pageFileDir = std::filesystem::path(pageFilePath).parent_path();

    if (!std::filesystem::exists(dataFileDir)) {
        std::filesystem::create_directories(dataFileDir);
    }
    if (!std::filesystem::exists(metadataFileDir)) {
        std::filesystem::create_directories(metadataFileDir);
    }
      if (!std::filesystem::exists(pageFileDir)) {
        std::filesystem::create_directories(pageFileDir);
    }

    data_file->open(dataFilePath, ios::out | ios::trunc| ios::in);
    metadata_file->open(metadataFilePath, ios::out | ios::trunc| ios::in);

    this->page_file->open(pageFilePath, ios::out | ios::trunc| ios::in);



    // Check if files are opened
    if (data_file->is_open()) {
        cout << "DATABASE CREATED." << std::endl;
    } else {
        cerr << "Error: Failed to open data file: " << dataFilePath << endl;
        throw std::runtime_error("Failed to create database file: " + dataFilePath);
    }
  

    if (metadata_file->is_open()) {
        cout << "Metadata file CREATED." << std::endl;
    } else {
        cerr << "Error: Failed to open metadata file: " << metadataFilePath << endl;
        throw std::runtime_error("Failed to create metadata file: " + metadataFilePath);
    }

     if (page_file->is_open()) {
        cout << "Metadata file CREATED." << std::endl;
    } else {
        cerr << "Error: Failed to open metadata file: " << metadataFilePath << endl;
        throw std::runtime_error("Failed to create metadata file: " + metadataFilePath);
    }
}



Table* Database::CreateTable(string table_name,vector<string> types,vector<string> names){

    Table *newTable=new Table(table_name,types,names,this->name,data_file,page_file);
    *metadata_file<<table_name;
    metadata_file->flush();
    tables.push_back(newTable);       
    return newTable;

    

}




Database::~Database(){
    data_file->close();
}