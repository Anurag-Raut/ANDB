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
    private:
    fstream * page_file;
    fstream* data_file;

   public:
   uint64_t rootPageNumber=0;
    Btree* btree;
    vector<fstream*> indexes;

    vector<Column> columns;
    string table_name;
    Table(string table_name, vector<string> types, vector<string> names,string database_name, fstream* data_file,fstream* page_file);
    void Insert(vector<string> args);
    void Print();
    string Search(string key);
    Block writeData(string key,string value);

};