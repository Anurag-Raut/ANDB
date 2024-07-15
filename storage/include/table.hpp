#pragma once
#include <string>
#include <vector>

#include "../include/btree.hpp"

using namespace std;
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


class Table {
   private:
    fstream* page_file;
    fstream* data_file;
    int primary_key_index=0;
   public:
    uint64_t rootPageNumber = 0;
    Btree* btree;
    vector<fstream*> indexes;

    vector<Column> columns;
    string table_name;
    Table(string table_name, vector<string> types, vector<string> names, string database_name, fstream* data_file, fstream* page_file,int primary_key_index);
    void Insert(vector<string> args);
    void Print();
    void Update(vector<string> args);
    vector<vector<string>> RangeQuery(string* key1,string* key2,vector<Column> types);
    vector<string> Deconstruct(string row,vector<Column> types);
    string Search(string key);
    Block writeData(string key, string value);
    optional<string> readValue(uint64_t pageNumber, uint16_t blockNumber);
    void Delete(string key);
};