#include "./include/database.hpp"

#include <fstream>
#include <iostream>
#include <string>

#include "./include/globals.hpp"
#include "./table.cpp"

using namespace std;
void LoadTables(Database* database) {
    database->metadata_file->seekg(0);
    string line;
    getline(*(database->metadata_file), line);
    while (getline(*(database->metadata_file), line)) {
        istringstream iss(line);
        string table_name;
        vector<string> types;
        vector<string> names;
        string types_str, names_str;

        if (!(iss >> table_name >> types_str >> names_str)) {
            cerr << "Error: Failed to parse metadata file line: " << line << endl;
            continue;
        }

        // Parse types
        stringstream types_ss(types_str);
        string type;
        while (getline(types_ss, type, ',')) {
            types.push_back(type);
        }

        // Parse names
        stringstream names_ss(names_str);
        string name;
        while (getline(names_ss, name, ',')) {
            names.push_back(name);
        }

        int primary_key_index = 0;
        // cout << "tableName: " << table_name << endl;
        Table* table = new Table(table_name, types, names, database->name, database->data_file, database->page_file, primary_key_index);
        database->tables[table_name] = table;
    }
}

Database::Database(string name) {
    string dataFilePath = getDatabaseFilePath(name);
    string metadataFilePath = getMetadataFilePath(name);
    string pageFilePath = getPageFilePath(name);
    string transaction_logPath = getTransactionLogFilePath(name);
    string walFilePath = getWALFilePath(name);
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
    createFile(dataFilePath);
    createFile(metadataFilePath);
    createFile(pageFilePath);
    createFile(transaction_logPath);
    createFile(walFilePath);

    data_file->open(dataFilePath, ios::in | ios::out);
    metadata_file->open(metadataFilePath, ios::in | ios::out | ios::app);

    page_file->open(pageFilePath, ios::in | ios::out);
    transaction_log->open(transaction_logPath, ios::in | ios::out | ios::app);
    wal_file->open(walFilePath, ios::in | ios::out | ios::app);

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
    uint64_t zero = 0;
    metadata_file->seekg(0, std::ios::end);
    std::streampos fileSize = metadata_file->tellg();
    if (fileSize == 0) {
        uint64_t zero = 0;
        metadata_file->seekp(0, ios::beg);
        metadata_file->write(reinterpret_cast<char*>(&zero), sizeof(zero));
        *metadata_file << endl;
        metadata_file->flush();

    } else {
        LoadTables(this);
    }

    metadata_file->seekg(0, std::ios::beg);
}

Table* Database::CreateTable(string table_name, vector<string> types, vector<string> names, int primary_key_index) {
    Table* newTable = new Table(table_name, types, names, this->name, data_file, page_file, primary_key_index);
    metadata_file->seekp(0, ios::end);
    metadata_file->seekg(0, ios::end);

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

        if (i != names.size() - 1) {
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
    // cout<<"WOOOOAOOOH  status: "<<static_cast<int>(status)<<endl;
    transaction_log->seekp(transaction_id * sizeof(status), std::ios::beg);
    char buffer[sizeof(transaction_id) + sizeof(uint8_t)];
    memcpy(buffer, &transaction_id, sizeof(transaction_id));
    size_t offset = sizeof(transaction_id);
    memcpy(buffer + offset, &status, sizeof(status));

    transaction_log->write(buffer, sizeof(buffer));
    if (!transaction_log->good()) {
        throw std::runtime_error("Failed to write to transaction log.");
    }
    transaction_log->flush();
}

TRANSACTION_STATUS Database::ReadTransactionLog(uint64_t transaction_id) {
    transaction_log->seekg(transaction_id * sizeof(uint8_t), std::ios::beg);
    char buffer[sizeof(transaction_id) + sizeof(uint8_t)];
    TRANSACTION_STATUS status;
    transaction_log->read(buffer, sizeof(buffer));
    if (!transaction_log->good()) {
        throw std::runtime_error("Failed to read from transaction log.");
    }
    memcpy(&status, buffer + sizeof(transaction_id), sizeof(status));
    // cout<<"status_int: "<<int(status)<<endl;
    return status;
}
