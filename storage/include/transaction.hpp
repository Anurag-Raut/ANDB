#pragma once
#include "./table.hpp"
#include "./database.hpp"

using namespace std;


class Transaction{

    Database* database;
    public:
    uint64_t transaction_id;
        
        Transaction(Database* database);
        void Insert(vector<string> args,Table* table);
        void Update(vector<string> args,Table* table);
        string Search(string key,string column_name,Table* table);
        void Delete(string key,Table* table);
        void CreateIndex(string column_name,Table* table);
        vector<vector<string>> RangeQuery(string* key1,string* key2,vector<Column> types,bool includeKey1,bool includeKey2,Table* table,string column_name="");
        Table* CreateTable(string table_name,vector<string> types,vector<string> names,int primary_key_indexs);
        Table* GetTable(string table_name);

        void Commit();

};