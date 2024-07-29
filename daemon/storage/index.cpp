#include <string>
#include "./include/index.hpp"
#include "../globals.hpp"
using namespace std;

Index::Index(string database_name,string table_name,string column_name){
    this->table_name=table_name;
    this->columnName=column_name;
    this->database_name=database_name;
    string primaryIndexFile = getIndexFilePath(database_name, table_name, column_name);
    this->file=new fstream();
    createFile(primaryIndexFile);
    file->open(primaryIndexFile, ios::out  | ios::in);
    this->btree = new Btree(this->file);

}