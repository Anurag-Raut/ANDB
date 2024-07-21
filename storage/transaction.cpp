
#include "./include/transaction.hpp"

#include <algorithm>
#include <cstdint>

#include "./include/globals.hpp"
#include "string"

using namespace std;

Transaction::Transaction(Database* database) {
    this->database = database;
    cout << "TRANSACTION_ID: " << TRANSACTION_ID << endl;
    this->transaction_id = TRANSACTION_ID;
    active_transactions.push_back(TRANSACTION_ID);
    snapshot = new Snapshot{active_transactions : active_transactions};
    TRANSACTION_ID++;
}

void Transaction::Insert(vector<string> args, Table* table) { table->Insert(args, transaction_id); }
void Transaction::Update(vector<string> args, Table* table) { table->Update(args, transaction_id); }
string Transaction::Search(string key, string column_name, Table* table) { return table->Search(key, column_name, transaction_id); }
void Transaction::Delete(string key, Table* table) { table->Delete(key, transaction_id); }
void Transaction::CreateIndex(string column_name, Table* table) { table->CreateIndex(column_name, transaction_id); }
vector<vector<string>> Transaction::RangeQuery(string* key1, string* key2, vector<Column> types, bool includeKey1, bool includeKey2, Table* table,
                                               string column_name) {
    vector<pair<vector<string>, pair<uint64_t, uint64_t>>> rows = table->RangeQuery(key1, key2, types, includeKey1, includeKey2, column_name);
    vector<vector<string>> transactionVisibleRows;
    for (auto row : rows) {
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

        if (i != name.size() - 1) {
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
}

bool Transaction::IsVisible(uint64_t t_ins, uint64_t t_del) {
    cout << "TX_INSERTED DELTED: " << t_ins <<" "<<t_del<< endl;
    cout << "CURRENT TRNASCTION: " << this->transaction_id << endl;
    // for()
    cout << "ACTIVE TRNASCTIONS : " << endl;

    for (auto act : active_transactions) {
        cout << act << " ";
    }
    cout << endl;
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
    
    if (!isInsertedByActiveTransaction) {
        isVisible = true;
    }

    if (!isDeletedByActiveTransaction && t_del!=0) {
        isVisible = false;
    }

    return isVisible;
}
