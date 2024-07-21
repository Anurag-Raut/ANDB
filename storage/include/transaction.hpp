#pragma once
#include "./database.hpp"
#include "./table.hpp"

using namespace std;

struct Snapshot {
    vector<uint64_t> active_transactions;
};
class Table;


class Transaction {
    Database* database;

   public:
    uint64_t transaction_id;
    Snapshot* snapshot;

    Transaction(Database* database);
    void Insert(vector<string> args, Table* table);
    void Update(vector<string> args, Table* table);
    string Search(string key, string column_name, Table* table);
    void Delete(string key, Table* table);
    void CreateIndex(string column_name, Table* table);
    vector<vector<string>> RangeQuery(string* key1, string* key2, vector<Column> types, bool includeKey1, bool includeKey2, Table* table,
                                      string column_name = "");
    Table* CreateTable(string table_name, vector<string> types, vector<string> names, int primary_key_indexs);
    Table* GetTable(string table_name);

    void Commit();
    bool IsVisible(uint64_t t_ins,uint64_t t_del);
    void Rollback();

};