
#include "./include/transaction.hpp"

#include <unistd.h>  // for fsync

#include <algorithm>
#include <cstdint>

#include "../globals.hpp"
#include "./include/database.hpp"
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
    // database->metadata_file->seekp(0,ios::beg);
    // database->metadata_file->write(reinterpret_cast<char*>(TRANSACTION_ID),sizeof(TRANSACTION_ID));
    // database->metadata_file->flush();
    database->writeTransactionId(TRANSACTION_ID);
    cout << "UPDATING TRANSACTION ID: " << TRANSACTION_ID << endl;
}

void Transaction::Insert(vector<string> args, Table* table) { table->Insert(args, transaction_id, database->wal_file); }
void Transaction::Update(vector<string> args, uint64_t page_number, uint16_t block_number, Table* table) {
    string key = args[table->primary_key_index];

    orderedLock.lock(transaction_id, rowLocks[key]);
    if (orderedLock.wasPrevTransactionId) {
        // orderedLock.unlock(false);
        throw TransactionException("Previous transaction ID detected in Update operation");
        return;
    }
    cout<<"UPDATINGGGG: "<<page_number<<" "<<block_number<<endl;
    table->Update(args, page_number, block_number, transaction_id, database->wal_file);
    // orderedLock.unlock(false);
}
string Transaction::Search(string key, string column_name, Table* table) { return table->Search(key, column_name, transaction_id); }
void Transaction::Delete(string key, Table* table) {
    vector<pair<vector<string>, pair<uint64_t, uint16_t>>> rowsToDelete =
        table->RangeQuery(&key, &key, table->columns, transaction_id, true, true, table->columns[table->primary_key_index].name);
    for (auto row : rowsToDelete) {
        table->Delete(key, row.second.first, row.second.second, transaction_id, database->wal_file);
    }
}
void Transaction::CreateIndex(string column_name, Table* table) { table->CreateIndex(column_name, transaction_id); }
vector<pair<vector<string>,pair<uint64_t,uint16_t>>> Transaction::RangeQuery(string* key1, string* key2, vector<Column> types, bool includeKey1, bool includeKey2, Table* table,
                                               string column_name) {
    vector<pair<vector<string>, pair<uint64_t, uint16_t>>> rows =
        table->RangeQuery(key1, key2, types, transaction_id, includeKey1, includeKey2, column_name);
    // cout<<"ROWS: "<<rows.size()<<endl;

    return rows;
}

Table* Transaction::CreateTable(string table_name, vector<string> types, vector<string> names, int primary_key_index) {
    // Move to the end of the metadata file to append
    database->metadata_file->seekp(0, ios::end);

    // Create the new table object
    Table* newTable = new Table(table_name, types, names, database, database->data_file, database->page_file, primary_key_index);

    // Write table name
    database->metadata_file->write(table_name.c_str(), table_name.size());
    database->metadata_file->write(" ", 1);

    // Write types
    for (int i = 0; i < types.size(); i++) {
        string type = types[i];
        database->metadata_file->write(type.c_str(), type.size());

        if (i != types.size() - 1) {
            database->metadata_file->write(",", 1);
        }
    }
    database->metadata_file->write(" ", 1);

    // Write names
    for (int i = 0; i < names.size(); i++) {
        string name = names[i];
        database->metadata_file->write(name.c_str(), name.size());

        if (i != names.size() - 1) {
            database->metadata_file->write(",", 1);
        }
    }

    // End of line
    database->metadata_file->write("\n", 1);

    // Flush the stream
    database->metadata_file->flush();

    // Add the table to the map
    database->tables[table_name] = newTable;

    return newTable;
}

Table* Transaction::GetTable(string table_name) {
    Table* table = database->tables[table_name];
    if (table == NULL) {
        throw runtime_error("TABLE " + table_name + " NOT FOUND");
    }
    return table;
}

void Transaction::Commit(bool isUpdate) {
    auto it = std::find(active_transactions.begin(), active_transactions.end(), uint64_t(this->transaction_id));

    if (it != active_transactions.end()) {
        active_transactions.erase(it);
    }
    database->UpdateTransactionLog(transaction_id, TRANSACTION_STATUS::COMMITED);
    WAL wal(OPERATION::COMMIT, transaction_id, NULL, NULL);
    wal.write(database->wal_file);
    database->data_file->flush();
    cout << "HMM" << endl;

    orderedLock.unlock(isUpdate);
    cout << "SADGE" << endl;

    // database->metadata_file->seekp(0, ios::beg);
    // // cout<<"LSN  : "<<wal.LSN<<endl;
    // database->metadata_file->write(reinterpret_cast<const char*>(&wal.LSN), sizeof(wal.LSN));
    // database->metadata_file->flush();
}

void Transaction::Rollback() { this->database->UpdateTransactionLog(transaction_id, TRANSACTION_STATUS::ABORTED); }