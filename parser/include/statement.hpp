#pragma once
#include <string>
#include <vector>
#include "../../storage/include/database.hpp"
#include "./expr.hpp"

using namespace std;
class Statement {
   public:
    string operation;

    // virtual void parse() = 0;

    virtual void print() const = 0;
    virtual void execute(Database* database) const =0;
    
};

class SelectStatement : public Statement {
   public:
    string table_name;
    vector<string> columns;
    shared_ptr<Expr> where_condition;

    SelectStatement(string table_name, vector<string> columns, shared_ptr<Expr> where_condition);
   void print()const override;

    void execute(Database* database) const override;

};