#pragma once
#include "./include/statement.hpp"

#include <iostream>

#include "../cli-table-cpp/src/Table.cpp"
using namespace std;

SelectStatement::SelectStatement(string table_name, vector<string> columns, string where_condition) {
    this->table_name = table_name;
    this->columns = columns;
    this->where_condition = where_condition;
}
void SelectStatement::print() const {
    cout << "SELECT statement:" << endl;
    cout << "Table: " << table_name << endl;
    cout << "Columns: ";
    for (const auto& col : columns) {
        cout << col << " ";
    }
    cout << endl;
    if (!where_condition.empty()) {
        cout << "Where: " << where_condition << endl;
    }
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

    // if(where_condition!=""){
    //     table->indexes
    // }

    string key = "key2";
    vector<vector<string>> data = table->RangeQuery(NULL, &key, requestedColums);
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
