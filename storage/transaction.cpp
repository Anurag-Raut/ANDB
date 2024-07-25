
#include "./include/transaction.hpp"

#include <unistd.h>  // for fsync

#include <algorithm>
#include <cstdint>

#include "./include/database.hpp"
#include "./include/globals.hpp"
#include "string"

using namespace std;

Transaction::Transaction(Database* database) {
    this->database = database;
    // cout << "TRANSACTION_ID: " << TRANSACTION_ID << endl;
    this->transaction_id = TRANSACTION_ID;
    active_transactions.push_back(TRANSACTION_ID);
    snapshot = new Snapshot{active_transactions : active_transactions};
    // cout<<"PROGRESS: "<<static_cast<int>(TRANSACTION_STATUS::IN_PROGRESS)<<endl;
    database->UpdateTransactionLog(TRANSACTION_ID, TRANSACTION_STATUS::IN_PROGRESS);
    TRANSACTION_ID++;
}

void Transaction::Insert(vector<string> args, Table* table) { table->Insert(args, transaction_id, database->wal_file); }
void Transaction::Update(vector<string> args, Table* table) { table->Update(args, transaction_id, database->wal_file); }
string Transaction::Search(string key, string column_name, Table* table) { return table->Search(key, column_name, transaction_id); }
void Transaction::Delete(string key, Table* table) { table->Delete(key, transaction_id, database->wal_file); }
void Transaction::CreateIndex(string column_name, Table* table) { table->CreateIndex(column_name, transaction_id); }
vector<vector<string>> Transaction::RangeQuery(string* key1, string* key2, vector<Column> types, bool includeKey1, bool includeKey2, Table* table,
                                               string column_name) {
    vector<vector<string>> rows = table->RangeQuery(key1, key2, types,transaction_id, includeKey1, includeKey2, column_name);
    // cout<<"ROWS: "<<rows.size()<<endl;
   

    return rows;
}

Table* Transaction::CreateTable(string table_name, vector<string> types, vector<string> names, int primary_key_index) {
    Table* newTable = new Table(table_name, types, names, database, database->data_file, database->page_file, primary_key_index);
    *database->metadata_file << table_name << " ";
    // cout<<"TYPE:"<<endl;
    for (int i = 0; i < types.size(); i++) {
        string type = types[i];
        *database->metadata_file << type;

        if (i != types.size() - 1) {
            *database->metadata_file << ",";
        }
    }
    *database->metadata_file << " ";

    for (int i = 0; i < names.size(); i++) {
        string name = names[i];

        *database->metadata_file << name;

        if (i != names.size() - 1) {
            *database->metadata_file << ",";
        }
    }
    *database->metadata_file << endl;
    database->metadata_file->flush();
    database->tables[table_name] = (newTable);
    return newTable;
}

Table* Transaction::GetTable(string table_name) {
    Table* table = database->tables[table_name];
    return table;
}

void Transaction::Commit() {
    auto it = std::find(active_transactions.begin(), active_transactions.end(), uint64_t(this->transaction_id));

    if (it != active_transactions.end()) {
        active_transactions.erase(it);
    }
    database->UpdateTransactionLog(transaction_id, TRANSACTION_STATUS::COMMITED);
    WAL wal(OPERATION::COMMIT, transaction_id, NULL, NULL);
    wal.write(database->wal_file);
    database->data_file->flush();
    // database->metadata_file->seekp(0, ios::beg);
    // // cout<<"LSN  : "<<wal.LSN<<endl;
    // database->metadata_file->write(reinterpret_cast<const char*>(&wal.LSN), sizeof(wal.LSN));
    // database->metadata_file->flush();
}



void Transaction::Rollback() { this->database->UpdateTransactionLog(transaction_id, TRANSACTION_STATUS::ABORTED); }