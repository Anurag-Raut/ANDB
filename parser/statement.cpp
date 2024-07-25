#pragma once
#include "./include/statement.hpp"

#include <iostream>
#include <memory>

#include "../cli-table-cpp/src/Table.cpp"
#include "../storage/include/transaction.hpp"
using namespace std;

SelectStatement::SelectStatement(string table_name, vector<string> columns, shared_ptr<Expr> where_condition) {
    this->table_name = table_name;
    this->columns = columns;
    this->where_condition = where_condition;
}

void SelectStatement::execute(Transaction* tx) const {
    Table* table = tx->GetTable(table_name);
    vector<Column> requestedColums;
    if (columns.empty()) {
        for (auto col : table->columns) {
            requestedColums.push_back(col);
            columns.push_back(col.name);
        }
    } else {
        // cout<<"COLUMNS SIZE: "<<columns.size()<<endl;
        for (auto column_name : columns) {
            bool found = 0;

            for (auto column : table->columns) {
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
    }

    vector<vector<string>> data;
    if (where_condition) {
        data = where_condition->execute(tx, requestedColums, table);
    } else {
        data = tx->RangeQuery(NULL, NULL, requestedColums, true, true, table);
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

CreateStatement::CreateStatement(string table_name, vector<Column> columns) {
    this->table_name = table_name;
    this->columns = columns;
}

void CreateStatement::execute(Transaction* tx) const {
    vector<string> types, names;
    for (auto column : columns) {
        types.push_back(column.type);
        names.push_back(column.name);
    }

    tx->CreateTable(table_name, types, names, 0);
}

InsertStatement::InsertStatement(string table_name, vector<string> columns, vector<string> values) {
    this->table_name = table_name;
    this->columns = columns;
    this->values = values;
}

void InsertStatement::execute(Transaction* tx) const {
    Table* table = tx->GetTable(table_name);

    tx->Insert(this->values, table);
}

DeleteStatement::DeleteStatement(string table_name, shared_ptr<Expr> where_condition) {
    this->table_name = table_name;
    this->where_condition = where_condition;
}

void DeleteStatement::execute(Transaction* tx) const {
    Table* table = tx->GetTable(table_name);
    vector<vector<string>> data;
    vector<Column> requestedColums = table->columns;

    vector<string> columns;

    for (auto column : requestedColums) {
        columns.push_back(column.name);
    }
    if (where_condition) {
        data = where_condition->execute(tx, requestedColums, table);
    } else {
        data = tx->RangeQuery(NULL, NULL, requestedColums, true, true, table);
    }
    // data = table->RangeQuery(NULL, NULL, {table->columns[table->primary_key_index]},true,true)
    for (auto item : data) {
        tx->Delete(item[table->primary_key_index], table);
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

UpdateStatement::UpdateStatement(string table_name, vector<pair<string, string>> newColumnValues, shared_ptr<Expr> where_condition) {
    this->table_name = table_name;
    this->newColumnValues = newColumnValues;
    this->where_condition = where_condition;
}
void UpdateStatement::execute(Transaction* tx) const {
    Table* table = tx->GetTable(table_name);
    vector<vector<string>> data;

    vector<Column> requestedColums = table->columns;
    map<string, int> getIndex;

    vector<string> columns;

    for (int i = 0; i < requestedColums.size(); i++) {
        auto column = requestedColums[i];
        columns.push_back(column.name);
        getIndex[column.name] = i;
    }

    if (where_condition) {
        cout<<"SIT DOWN BHAII"<<endl;
        data = where_condition->execute(tx, requestedColums, table);
        for(auto d:data){
            if(d)
        }
        cout<<data.size()<<endl;
    } else {
        data = tx->RangeQuery(NULL, NULL, requestedColums, true, true, table);
    }

    for (auto item : data) {
        for (auto newColumnValue : newColumnValues) {
            string columnName = newColumnValue.first;
            string newValue = newColumnValue.second;
            item[getIndex[columnName]] = newValue;
        }
        tx->Update(item, table);
    }

    CliTable::Options opt;
    CliTable::TableBody content;
    content.push_back(columns);

    for (auto row : data) {
        content.push_back(row);
    }
    CliTable::Table printTable(opt, content);
    printTable.generate();
}

BeginStatement::BeginStatement() {}
void BeginStatement::execute(Transaction* tx) const {}
CommitStatement::CommitStatement() {}

void CommitStatement::execute(Transaction* tx) const {}
RollbackStatement::RollbackStatement() {}

void RollbackStatement::execute(Transaction* tx) const {}
