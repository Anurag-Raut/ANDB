#pragma once
#include <string>
#include <vector>

#include "../include/btree.hpp"
#include "../include/index.hpp"

using namespace std;
class Database;
struct Column {
    string name;
    string type;
};
struct MetadataDataPage {
    uint16_t noOfBlocks;
    uint16_t begOffset;
    uint16_t endOffset;

    const int size() { return sizeof(noOfBlocks) + sizeof(begOffset) + sizeof(endOffset); }
};
// class Transaction;


class Table  {
   private:
    fstream* page_file;
    fstream* data_file;
   public:

    int primary_key_index=0;
    uint64_t rootPageNumber = 0;

    vector<Index*> indexes;

    vector<Column> columns;
    string table_name;
    Database *database;
    Table(string table_name, vector<string> types, vector<string> names, Database *database, fstream* data_file, fstream* page_file,int primary_key_index);
    void Insert(vector<string> args,uint64_t transaction_id,fstream * wal_file);
    void Update(vector<string> args,uint64_t transaction_id,fstream * wal_file);
    string Search(string key,string column_name,uint64_t transaction_id);
    void Delete(string key,uint64_t transaction_id,fstream * wal_file);
    void CreateIndex(string column_name,uint64_t transaction_id);
    vector<vector<string>>  RangeQuery(string* key1,string* key2,vector<Column> types,uint64_t transaction_id,bool includeKey1,bool includeKey2,string column_name);
    vector<string> Deconstruct(string row,vector<Column> types);
    vector<string> Deconstruct(vector<string>rowValues, vector<Column> types);

    
    pair<optional<string>,pair<uint64_t,uint64_t>>  readValue(uint64_t pageNumber, uint16_t blockNumber);
    Block writeData(string key, string value,uint64_t transaction_id);
    Index* getIndex(string column_name);
    void Print(string column_name);
    
};