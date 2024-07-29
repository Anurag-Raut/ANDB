#pragma once
#include <fstream>
#include "./btree.hpp"

using namespace std;
class Index {
   public:
    string columnName,table_name,database_name;
    Btree* btree;
    fstream* file;
    Index(string database_name,string table_name,string columnName);
};


