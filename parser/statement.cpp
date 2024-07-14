#pragma once
#include "./include/statement.hpp"

#include <iostream>
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
