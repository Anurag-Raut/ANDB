#pragma once

#include <fstream>
#include <string>
#include<vector>
#include "../include/table.hpp"

using namespace std;



class Database {
    vector<Table *> tables; 
   

   public:
   fstream *data_file=new fstream();
   fstream *metadata_file=new fstream();
    Database(string database_name);
    ~Database();
    Table* CreateTable(string table_name,vector<string> types,vector<string> names);


};
