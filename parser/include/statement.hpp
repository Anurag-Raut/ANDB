#pragma once
#include <string>
#include <vector>

using namespace std;
class Statement {
   public:
    string operation;

    // virtual void parse() = 0;

    virtual void print() const = 0;
    
};

class SelectStatement : public Statement {
   public:
    string table_name;
    vector<string> columns;
    string where_condition;

    SelectStatement(string table_name, vector<string> columns, string where_condition);
   void print()const override;
};