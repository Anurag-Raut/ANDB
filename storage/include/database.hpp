#pragma once

#include <fstream>
#include <string>
#include <unordered_map>
#include <vector>
#include <cstring>
#include "../include/table.hpp"

using namespace std;

enum TRANSACTION_STATUS : uint8_t {
    COMMITED,
    IN_PROGRESS,
    ABORTED,

};
enum class OPERATION : uint8_t {
    INSERT,
    DELETE,
    COMMIT,
    ROLLBACK,

};

struct WAL {
    uint64_t LSN;
    OPERATION operation;
    uint64_t transaction_id;
    size_t keySize;
    string key = "";
    size_t valueSize;
    string value = "";

    // Constructor to initialize the structure
    WAL(OPERATION op, uint64_t tx_id, const string *k, const string *v) {
        this->operation = op;
        this->transaction_id = tx_id;
        if (k) {
            this->keySize = k->size();
            this->key = *k;
        }
        if (v) {
            this->valueSize = v->size();
            this->value = *v;
        }
    }

    int size() {
        return sizeof(uint64_t) + sizeof(uint8_t) + sizeof(transaction_id) + (key.size() ? key.size() + sizeof(keySize) : 0) +
               (value.size() ? value.size() + sizeof(valueSize) : 0);
    }

    void write(fstream *wal_file) {
        wal_file->seekg(0, std::ios::end);
        wal_file->seekp(0, std::ios::end);

        // Get the current position in bytes
        std::streampos LSN = wal_file->tellp();
        this->LSN=uint64_t(LSN);
        char buffer[size()];
        size_t offset = 0;
        memcpy(buffer + offset, &(LSN), sizeof(uint64_t));
        offset += sizeof(uint64_t);
        memcpy(buffer + offset, &operation, sizeof(uint8_t));
        offset += sizeof(uint8_t);
        memcpy(buffer + offset, &transaction_id, sizeof(uint64_t));
        offset += sizeof(uint64_t);
        if (key.size() > 0) {
            memcpy(buffer + offset, &keySize, sizeof(keySize));
            offset += sizeof(keySize);
            memcpy(buffer + offset, &key, key.size());
            offset += key.size();
        }
        if (value.size() > 0) {
            memcpy(buffer + offset, &valueSize, sizeof(valueSize));
            offset += value.size();
            memcpy(buffer + offset, &value, value.size());
            offset += value.size();
        }

        wal_file->write(buffer,sizeof(buffer));
        wal_file->flush();
    }
};

class Database {
   public:
    string name;
    unordered_map<string, Table *> tables;
    fstream *data_file = new fstream();
    fstream *metadata_file = new fstream();
    fstream *page_file = new fstream();
    fstream *transaction_log = new fstream();
    fstream *wal_file = new fstream();

    Database(string database_name);
    ~Database();
    Table *CreateTable(string table_name, vector<string> types, vector<string> names, int primary_key_indexs);
    void UpdateTransactionLog(uint64_t transaction_id, TRANSACTION_STATUS status);
    TRANSACTION_STATUS ReadTransactionLog(uint64_t transaction_id);

    void writeWAL(OPERATION operation, uint64_t transaction_id, const string *key, const string *value);
};
