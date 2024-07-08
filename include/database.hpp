#pragma once

#include <fstream>
#include <string>
#include<vector>
using namespace std;



class Database {
   private:
   fstream data_file;
   fstream metadata_file;
   

   public:
    Database(string database_name);
    void CreateTable(string table_name,vector<string> types,vector<string> names);


};
