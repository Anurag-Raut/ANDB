#include "./include/database.hpp"

#include <fstream>
#include <iostream>
#include <string>

#include "./table.cpp"

using namespace std;
void LoadTables(Database* database) {
    cout << "LOADINGGGG>>>" << endl;
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
        cout << "LOADING tableName: " << table_name << endl;
        Table* table = new Table(table_name, types, names, database, database->data_file, database->page_file, primary_key_index);
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
    // cout << "Attempting to create/open files at: " << dataFilePath << " and " << metadataFilePath << endl;

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
    metadata_file->open(metadataFilePath, ios::in | ios::out );

    page_file->open(pageFilePath, ios::in | ios::out);
    transaction_log->open(transaction_logPath, ios::in | ios::out);
    wal_file->open(walFilePath, ios::in | ios::out | ios::app);

    // Check if files are opened
    if (data_file->is_open()) {
        // cout << "DATABASE CREATED." << std::endl;
    } else {
        cerr << "Error: Failed to open data file: " << dataFilePath << endl;
        throw std::runtime_error("Failed to create database file: " + dataFilePath);
    }

    if (metadata_file->is_open()) {
        // cout << "Metadata file CREATED." << std::endl;
    } else {
        cerr << "Error: Failed to open metadata file: " << metadataFilePath << endl;
        throw std::runtime_error("Failed to create metadata file: " + metadataFilePath);
    }

    if (page_file->is_open()) {
        // cout << "Page file CREATED." << std::endl;
    } else {
        cerr << "Error: Failed to open metadata file: " << metadataFilePath << endl;
        throw std::runtime_error("Failed to create metadata file: " + metadataFilePath);
    }
    if (transaction_log->is_open()) {
        // cout << "Page file CREATED." << std::endl;
    } else {
        cerr << "Error: Failed to open metadata file: " << metadataFilePath << endl;
        throw std::runtime_error("Failed to create metadata file: " + metadataFilePath);
    }
    metadata_file->seekg(0, std::ios::end);
    std::streampos fileSize = metadata_file->tellg();
    if (fileSize == 0) {
        uint64_t zero = 1;
        metadata_file->seekp(0, ios::beg);
        metadata_file->write(reinterpret_cast<char*>(&zero), sizeof(zero));
        *metadata_file << endl;
        cout<<"ELON"<<endl;
        
        

    } else {
        TRANSACTION_ID=readTransactionId();
        cout<<"BUNCHH TRANSACTION IDDDDDD: "<<TRANSACTION_ID<<endl;
        LoadTables(this);
            //     metadata_file->seekg(0, ios::beg);
            //     cout<<"GG"<<endl;
            //     uint64_t imp;
            // metadata_file->read(reinterpret_cast<char*>(&imp), sizeof(TRANSACTION_ID));

            // cout<<imp<<"  TRANSACTION IDDDDDD: TIDD: "<<TRANSACTION_ID<<endl;
    }

    std::cout << "Successfully connected to database: \"" << name << "\"\n";
}

Table* Database::CreateTable(string table_name, vector<string> types, vector<string> names, int primary_key_index) {
    Table* newTable = new Table(table_name, types, names, this, data_file, page_file, primary_key_index);
    metadata_file->seekp(0, ios::end);
    cout<<"CREATE TABLE : "<<table_name<<endl;
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
    transaction_log->clear();
    cout<<"WRTING TRANSACTION ID: "<<transaction_id<<endl;
    cout<<"WOOOOAOOOH  status: "<<static_cast<int>(status)<<endl;
    transaction_log->seekp((transaction_id-1) * (sizeof(transaction_id) + sizeof(uint8_t)), std::ios::beg);
            cout<<"WRITE TP POINTER: "<<transaction_log->tellp()<<endl;

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
    transaction_log->clear();

    if(transaction_id==uint64_t(0)){
        return TRANSACTION_STATUS::ABORTED;
    }

    cout<<"transaction_id:  :  : "<<transaction_id<<endl;
    transaction_log->seekg((transaction_id-1) *( sizeof(uint8_t)+sizeof(transaction_id)), std::ios::beg);
        cout<<"READ TP POINTER: "<<transaction_log->tellg()<<endl;

    char buffer[sizeof(transaction_id) + sizeof(uint8_t)];
    TRANSACTION_STATUS status;
    transaction_log->read(buffer, sizeof(buffer));
    if (!transaction_log->good()) {
        throw std::runtime_error("Failed to read from transaction log.");
    }
    memcpy(&status, buffer + sizeof(transaction_id), sizeof(status));
    cout<<"status_int: "<<int(status)<<endl;
    return status;
}

void Database::PrintAllTransactions() {
    transaction_log->clear();
    transaction_log->seekg(0, std::ios::beg);
    cout<<"PRINT POINTER: "<<transaction_log->tellg()<<endl;
    uint64_t transaction_id = 0;
    char buffer[sizeof(transaction_id) + sizeof(uint8_t)];
    TRANSACTION_STATUS status;
    
    while (transaction_log->read(buffer, sizeof(buffer))) {
        if(!transaction_log->good()){
            cout<<"NOT GOOD"<<endl;
            break;
        }
        memcpy(&transaction_id, buffer, sizeof(transaction_id));
        memcpy(&status, buffer + sizeof(transaction_id), sizeof(status));

        std::cout << "Transaction ID: " << transaction_id << ", Status: ";
        switch (status) {
            case TRANSACTION_STATUS::IN_PROGRESS:
                std::cout << "IN PROGRESS";
                break;
            case TRANSACTION_STATUS::COMMITED:
                std::cout << "COMMITTED";
                break;
            case TRANSACTION_STATUS::ABORTED:
                std::cout << "ABORTED";
                break;
            default:
                std::cout << "UNKNOWN";
        }
        std::cout << std::endl;

        transaction_id++;
    }

    if (transaction_log->eof()) {
        std::cout << "End of transaction log reached." << std::endl;
        return;
    } 
}


bool Database::IsVisible(uint64_t t_ins, uint64_t t_del, uint64_t transaction_id) {
    cout<<"t_ins: "<<t_ins<<" t_del: "<<t_del<<" CURRENT TRNASACTION: "<<transaction_id<<endl;
    bool isVisible = false;
    if (t_ins == transaction_id) {
        isVisible = true;
    }
    if (t_del == transaction_id) {
        isVisible = false;
    }
    if (t_del == transaction_id) {
        return isVisible;
    }
    bool isInsertedByActiveTransaction = (std::find(active_transactions.begin(), active_transactions.end(), t_ins) != active_transactions.end());
    bool isDeletedByActiveTransaction = (std::find(active_transactions.begin(), active_transactions.end(), t_del) != active_transactions.end());
    cout<<"t isInsert: "<<int(isInsertedByActiveTransaction)<<" "<<"t_is del: "<<int(isDeletedByActiveTransaction)<<endl;
    TRANSACTION_STATUS del_status = this->ReadTransactionLog(t_del);
    TRANSACTION_STATUS ins_status = this->ReadTransactionLog(t_ins);
    cout<<"INSERTED_STATUS: " << int(ins_status)<<endl;
        cout<<"DELETED_STATUS: "<< int(del_status)<<endl;
    
    if (!isInsertedByActiveTransaction && ins_status!=TRANSACTION_STATUS::IN_PROGRESS) {
        cout<<"is not InsertedByActiveTransaction"<<endl;
        isVisible = true;
    }

    if (!isDeletedByActiveTransaction && t_del != 0) {
                cout<<"is not DeletedByActiveTransaction"<<endl;

        isVisible = false;
    }

    if (del_status == TRANSACTION_STATUS::ABORTED  && t_del!=0) {
        cout<<"DEL STATUS ABORTED"<<endl;
        isVisible = true;
    }
    if (ins_status == TRANSACTION_STATUS::ABORTED) {
                cout<<"INSERT STATUS ABORTED"<<endl;

        isVisible = false;
    }
    if(ins_status==TRANSACTION_STATUS::COMMITED){
        isVisible=true;
    }
    if(del_status==TRANSACTION_STATUS::COMMITED){
        isVisible=false;
    }

    return isVisible;
}


void Database::writeTransactionId( uint64_t transactionId) {
    metadata_file->clear();
    metadata_file->seekp(0, std::ios::beg);
        cout<<"WRITE POINTER "<<metadata_file->tellp()<<endl;

    char buffer[sizeof(transactionId)];
    memcpy(&buffer,&transactionId,sizeof(transactionId));
    metadata_file->write(buffer, sizeof(buffer));
    metadata_file->flush();
    
    // Check if write was successful
    if (!metadata_file) {
        std::cerr << "Error writing to file!" << std::endl;
    }


    cout<<"checking again"<<endl;
    cout<<"MEMORY: "<<readTransactionId()<<endl;
}

uint64_t Database::readTransactionId() {
    metadata_file->clear();
    uint64_t transactionId = 0;
        char buffer[sizeof(transactionId)];

    metadata_file->seekg(0, std::ios::beg);
    cout<<"READ POINTER "<<metadata_file->tellg()<<endl;
    metadata_file->read(buffer, sizeof(buffer));
    memcpy(&transactionId,buffer,sizeof(transactionId));
        cout<<"READING TRANSACTION ID: "<<transactionId<<endl;
    // Check if read was successful
    if (!metadata_file) {
        std::cerr << "Error reading from file!" << std::endl;
    }
    
    return transactionId;
}