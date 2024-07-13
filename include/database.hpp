#pragma once

#include <fstream>
#include <string>
#include<vector>
#include "../include/table.hpp"

using namespace std;



class Database {
    vector<Table *> tables; 
    string name;

   public:
   fstream *data_file=new fstream();
   fstream *metadata_file=new fstream();
    fstream *page_file=new fstream();

    Database(string database_name);
    ~Database();
    Table* CreateTable(string table_name,vector<string> types,vector<string> name0,int primary_key_indexs);


};
