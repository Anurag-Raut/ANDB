
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
    vector<pair<vector<string>, pair<uint64_t, uint64_t>>> rows = table->RangeQuery(key1, key2, types, includeKey1, includeKey2, column_name);
    vector<vector<string>> transactionVisibleRows;
    // cout<<"ROWS: "<<rows.size()<<endl;
    for (auto row : rows) {
        // cout<<"row first : "<<row.first[1]<<endl;
        if (this->IsVisible(row.second.first, row.second.second)) {
            transactionVisibleRows.push_back(row.first);
        }
    }

    return transactionVisibleRows;
}

Table* Transaction::CreateTable(string table_name, vector<string> types, vector<string> names, int primary_key_index) {
    Table* newTable = new Table(table_name, types, names, database->name, database->data_file, database->page_file, primary_key_index);
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

bool Transaction::IsVisible(uint64_t t_ins, uint64_t t_del) {
    // cout << "TX_INSERTED DELTED: " << t_ins << " " << t_del << endl;
    // cout << "CURRENT TRNASCTION: " << this->transaction_id << endl;
    // // for()
    // cout << "ACTIVE TRNASCTIONS : " << endl;

    // for (auto act : active_transactions) {
    //     cout << act << " ";
    // }
    // cout << endl;
    bool isVisible = false;
    if (t_ins == this->transaction_id) {
        isVisible = true;
    }
    if (t_del == this->transaction_id) {
        isVisible = false;
    }
    if (t_del == this->transaction_id) {
        return isVisible;
    }
    bool isInsertedByActiveTransaction = (std::find(active_transactions.begin(), active_transactions.end(), t_ins) != active_transactions.end());
    bool isDeletedByActiveTransaction = (std::find(active_transactions.begin(), active_transactions.end(), t_del) != active_transactions.end());
    // cout<<"t del: "<<t_del<<" "<<"t_ins: "<<t_ins<<endl;
    TRANSACTION_STATUS del_status = database->ReadTransactionLog(t_del);
    TRANSACTION_STATUS ins_status = database->ReadTransactionLog(t_ins);
    // cout<<"INSERTED_STATUS: " << uint8_t(ins_status)<<endl;
    //     cout<<"DELETED_STATUS: "<< uint8_t(del_status)<<endl;

    if (!isInsertedByActiveTransaction) {
        isVisible = true;
    }

    if (!isDeletedByActiveTransaction && t_del != 0) {
        isVisible = false;
    }

    if (del_status == TRANSACTION_STATUS::ABORTED) {
        // cout<<"SASAS"<<endl;
        isVisible = true;
    }
    if (ins_status == TRANSACTION_STATUS::ABORTED) {
        isVisible = false;
    }

    return isVisible;
}

void Transaction::Rollback() { this->database->UpdateTransactionLog(transaction_id, TRANSACTION_STATUS::ABORTED); }