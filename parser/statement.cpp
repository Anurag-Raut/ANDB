#pragma once
#include "./include/statement.hpp"
#include "../cli-table-cpp/src/Table.cpp"

#include <iostream>
#include <memory>
using namespace std;

SelectStatement::SelectStatement(string table_name, vector<string> columns, shared_ptr<Expr> where_condition) {
    this->table_name = table_name;
    this->columns = columns;
    this->where_condition = where_condition;
}


void SelectStatement::execute(Database* db) const {
    Table* table = db->GetTable(table_name);
    vector<Column> requestedColums;
    if (columns.size() == 1 && columns[0] == "*") {
        requestedColums = table->columns;
    }

    for (auto column_name : columns) {
        bool found = 0;
        // cout<<"HEHE"<<endl;
        // cout<<column_name<<endl;
        for (auto column : table->columns) {
            // cout<<column.name<<endl;
            if (column.name == column_name) {
                found = 1;
                requestedColums.push_back(column);
            }
        }
        if (found == 0) {
            cout << "ERORR WHILE PARSING COLUMNS DATA";
            return;
        }
    }
    vector<vector<string>> data;
    if (where_condition) {
        data = where_condition->execute(table, requestedColums);
    } else {
        cout<<"YOOO"<<endl;
        data = table->RangeQuery(NULL, NULL, requestedColums,true,true);
    }

    // cout<<"YOOO: "<<data.size()<<endl;

    CliTable::Options opt;
    // Contructing the table structure
    CliTable::TableBody content;
    content.push_back(columns);

    for (auto row : data) {
        content.push_back(row);
    }
    CliTable::Table printTable(opt, content);
    printTable.generate();
}


CreateStatement::CreateStatement(string table_name, vector<Column> columns){

    this->table_name=table_name;
    this->columns=columns;


}

void CreateStatement::execute(Database* database) const {
    
    vector<string > types,names;
    for(auto column:columns){
        types.push_back(column.type);
        names.push_back(column.name);
        
    }
    
    database->CreateTable(table_name,types,names,0);

}

InsertStatement::InsertStatement(string table_name, vector<string> columns,vector<string> values){

    this->table_name=table_name;
    this->columns=columns;
    this->values=values;

}

void InsertStatement::execute(Database* database) const {
    
    
  
    Table* table=database->GetTable(table_name);
    
    table->Insert(this->values);

}


DeleteStatement::DeleteStatement(string table_name, shared_ptr<Expr> where_condition){

    this->table_name=table_name;
    this->where_condition=where_condition;

}

void DeleteStatement::execute(Database* database) const {
    
    
  
    Table* table=database->GetTable(table_name);
    vector<vector<string>> data;
    vector<Column> requestedColums=table->columns;

    vector<string> columns;
    for(auto column:requestedColums){
        columns.push_back(column.name);
    }
    if (where_condition) {
        data = where_condition->execute(table, requestedColums);
    } else {
        data = table->RangeQuery(NULL, NULL, requestedColums,true,true);
    }
    // data = table->RangeQuery(NULL, NULL, {table->columns[table->primary_key_index]},true,true)
    for(auto item:data){
        table->Delete(item[table->primary_key_index]);
    }

    CliTable::Options opt;
    // Contructing the table structure
    CliTable::TableBody content;
    content.push_back(columns);

    for (auto row : data) {
        content.push_back(row);
    }
    CliTable::Table printTable(opt, content);
    printTable.generate();


}

