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

    // virtual void print() const = 0;
    virtual void execute(Database* database) const =0;
    
};

class SelectStatement : public Statement {
   public:
    string table_name;
    vector<string> columns;
    shared_ptr<Expr> where_condition;

    SelectStatement(string table_name, vector<string> columns, shared_ptr<Expr> where_condition);
//    void print()const override;

    void execute(Database* database) const override;

};

class CreateStatement : public Statement {
   public:
    string table_name;
    vector<Column> columns;

    CreateStatement(string table_name, vector<Column> columns);
    // void print() const override;

    void execute(Database* database) const override;

};

class InsertStatement : public Statement {
   public:
    string table_name;
    vector<string> columns;
    vector<string> values;

    InsertStatement(string table_name, vector<string> columns,vector<string> values);
    // void print() const override;

    void execute(Database* database) const override;

};

class DeleteStatement : public Statement {
   public:
    string table_name;
    shared_ptr<Expr> where_condition;

    DeleteStatement(string table_name,shared_ptr<Expr>  where_condition);
    // void print() const override;

    void execute(Database* database) const override;

};


class UpdateStatement : public Statement {
   public:
    string table_name;
    vector<pair<string,string>> newColumnValues;
    shared_ptr<Expr> where_condition;

    UpdateStatement(string table_name,vector<pair<string,string>> newColumnValues,shared_ptr<Expr>  where_condition);
    // void print() const override;

    void execute(Database* database) const override;

};
