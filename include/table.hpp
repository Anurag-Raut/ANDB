#pragma once
#include <string>
#include <vector>

#include "../include/btree.hpp"

using namespace std;
struct Column {
    string name;
    string type;
};

class Table {

   public:
   uint64_t rootPageNumber=0;
    Btree* btree;
    vector<Column> columns;
    string table_name;
    Table(string table_name, vector<string> types, vector<string> names, fstream* fp);
    void Insert(vector<string> args);
    void Print();
    string Search(string key);

};