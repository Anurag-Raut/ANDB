#pragma once
#include <string>
#include <vector>
#include "../../storage/include/database.hpp"
#include "../../storage/include/transaction.hpp"

#include "./expr.hpp"

using namespace std;
class Statement {
   public:
    string operation;

    // virtual void parse() = 0;

    // virtual void print() const = 0;
    virtual string execute(Transaction* tx) const = 0;
    
};

class SelectStatement : public Statement {
   public:
    string table_name;
    mutable vector<string> columns;

    shared_ptr<Expr> where_condition;

    SelectStatement(string table_name, vector<string> columns, shared_ptr<Expr> where_condition);

    string execute(Transaction* tx) const override;

};

class CreateStatement : public Statement {
   public:
    string table_name;
    vector<Column> columns;

    CreateStatement(string table_name, vector<Column> columns);


    string execute(Transaction* tx) const override;

};

class InsertStatement : public Statement {
   public:
    string table_name;
    vector<string> columns;
    vector<string> values;

    InsertStatement(string table_name, vector<string> columns,vector<string> values);


    string execute(Transaction* tx) const override;

};

class DeleteStatement : public Statement {
   public:
    string table_name;
    shared_ptr<Expr> where_condition;

    DeleteStatement(string table_name,shared_ptr<Expr>  where_condition);


    string execute(Transaction* tx) const override;

};


class UpdateStatement : public Statement {
   public:
    string table_name;
    vector<pair<string,string>> newColumnValues;
    shared_ptr<Expr> where_condition;

    UpdateStatement(string table_name,vector<pair<string,string>> newColumnValues,shared_ptr<Expr>  where_condition);


    string execute(Transaction* tx) const override;

};

class BeginStatement : public Statement {
   public:
    string table_name;
    vector<pair<string,string>> newColumnValues;
    shared_ptr<Expr> where_condition;

    BeginStatement();


    string execute(Transaction* tx) const override;

};

class CommitStatement : public Statement {
   public:
    string table_name;
    vector<pair<string,string>> newColumnValues;
    shared_ptr<Expr> where_condition;

    CommitStatement();


    string execute(Transaction* tx) const override;

};
class RollbackStatement : public Statement {
   public:
    string table_name;
    vector<pair<string,string>> newColumnValues;
    shared_ptr<Expr> where_condition;

    RollbackStatement();


    string execute(Transaction* tx) const override;

};
