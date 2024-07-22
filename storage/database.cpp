#include "./include/database.hpp"

#include <fstream>
#include <iostream>
#include <string>

#include "./include/globals.hpp"
#include "./table.cpp"

using namespace std;

Database::Database(string name) {
    string dataFilePath = getDatabaseFilePath(name);
    string metadataFilePath = getMetadataFilePath(name);
    string pageFilePath = getPageFilePath(name);
    string transaction_logPath = getTransactionLogFilePath(name);
    string walFilePath=getWALFilePath(name);
    this->name = name;
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

    data_file->open(dataFilePath, ios::out | ios::trunc | ios::in);
    metadata_file->open(metadataFilePath, ios::out | ios::trunc | ios::in);

    page_file->open(pageFilePath, ios::out | ios::trunc | ios::in);
    transaction_log->open(transaction_logPath, ios::out | ios::trunc | ios::in);
    wal_file->open(walFilePath, ios::out | ios::trunc | ios::in);


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
        cout << "Page file CREATED." << std::endl;
    } else {
        cerr << "Error: Failed to open metadata file: " << metadataFilePath << endl;
        throw std::runtime_error("Failed to create metadata file: " + metadataFilePath);
    }
    if (transaction_log->is_open()) {
        cout << "Page file CREATED." << std::endl;
    } else {
        cerr << "Error: Failed to open metadata file: " << metadataFilePath << endl;
        throw std::runtime_error("Failed to create metadata file: " + metadataFilePath);
    }
    uint64_t zero=0;
    metadata_file->write(reinterpret_cast<char*>(&zero),sizeof(zero));

}

Table* Database::CreateTable(string table_name, vector<string> types, vector<string> names, int primary_key_index) {
    Table* newTable = new Table(table_name, types, names, this->name, data_file, page_file, primary_key_index);
    metadata_file->seekp(0,ios::end);
    *metadata_file << table_name << " ";
    // cout<<"TYPE:"<<endl;
    for (int i = 0; i < types.size(); i++) {
        string type = types[i];
        *metadata_file << type;

        if (i != types.size() - 1) {
            *metadata_file << ",";
        }
    }
    *metadata_file << " ";

    for (int i = 0; i < names.size(); i++) {
        string name = names[i];

        *metadata_file << name;

        if (i != name.size() - 1) {
            *metadata_file << ",";
        }
    }
    *metadata_file << endl;
    metadata_file->flush();
    tables[table_name] = (newTable);
    cout << "TABLE " + table_name + " CREATED" << endl;
    return newTable;
}

Database::~Database() { data_file->close(); }

void Database::UpdateTransactionLog(uint64_t transaction_id, TRANSACTION_STATUS status) {
    cout<<"WOOOOAOOOH  status: "<<static_cast<int>(status)<<endl;
    transaction_log->seekp(transaction_id * sizeof(status), std::ios::beg);
    char buffer[sizeof(transaction_id)+sizeof(uint8_t)];
    memcpy(buffer,&transaction_id,sizeof(transaction_id));
    size_t offset=sizeof(transaction_id);
    memcpy(buffer+offset,&status,sizeof(status));

    transaction_log->write(buffer, sizeof(buffer));
    if (!transaction_log->good()) {
        throw std::runtime_error("Failed to write to transaction log.");
    }

}

TRANSACTION_STATUS Database::ReadTransactionLog(uint64_t transaction_id) {
    transaction_log->seekg(transaction_id * sizeof(uint8_t), std::ios::beg);
    char buffer[sizeof(transaction_id)+sizeof(uint8_t)];
    TRANSACTION_STATUS status;
    transaction_log->read(buffer, sizeof(buffer));
    if (!transaction_log->good()) {
        throw std::runtime_error("Failed to read from transaction log.");
    }
    memcpy(&status,buffer+sizeof(transaction_id),sizeof(status));
    cout<<"status_int: "<<int(status)<<endl;
    return status;
}





