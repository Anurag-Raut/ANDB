#pragma once

#include <fstream>
#include <string>
#include<unordered_map>
#include<vector>
#include "../include/table.hpp"

using namespace std;

enum TRANSACTION_STATUS :uint8_t {
    COMMITED,
    IN_PROGRESS,
    ABORTED,


};


class Database {

   public:
    string name;
    unordered_map<string,Table *> tables; 
   fstream *data_file=new fstream();
   fstream *metadata_file=new fstream();
    fstream *page_file=new fstream();
    fstream *transaction_log=new fstream();


    Database(string database_name);
    ~Database();
    Table* CreateTable(string table_name,vector<string> types,vector<string> names,int primary_key_indexs);
    void UpdateTransactionLog(uint64_t transaction_id,TRANSACTION_STATUS status);
    TRANSACTION_STATUS ReadTransactionLog(uint64_t transaction_id);


};
