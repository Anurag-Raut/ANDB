#pragma once
#include <memory>
#include <vector>
#include <string>
#include "../../storage/include/transaction.hpp"
#include "token.hpp"

using namespace std;

class Expr {
public:
    virtual ~Expr() = default; // Ensure Expr is polymorphic with a virtual destructor
    virtual vector<vector<string>> execute(Transaction* tx ,vector<Column> requestedColums,Table * table){}

};

class ANDExpr : public Expr {
public:
    std::shared_ptr<Expr> left;
    std::shared_ptr<Expr> right;

    ANDExpr(std::shared_ptr<Expr> left, std::shared_ptr<Expr> right);
    vector<vector<string>> execute(Transaction* tx ,vector<Column> requestedColums,Table * table) override;

};

class ORExpr : public Expr {
public:
    std::shared_ptr<Expr> left;
    std::shared_ptr<Expr> right;

    ORExpr(std::shared_ptr<Expr> left, std::shared_ptr<Expr> right);
        vector<vector<string>> execute(Transaction* tx ,vector<Column> requestedColums,Table * table) override;

};

class LiteralExpr : public Expr {
public:
    string literal;
    LiteralExpr(string literal);
    LiteralExpr() = default;
    vector<vector<string>> execute(Transaction* tx ,vector<Column> requestedColums,Table * table) override;

};

class IdentifierExpr : public Expr {
public:
    string name;
    IdentifierExpr() = default;
    IdentifierExpr(string name) ;
    vector<vector<string>> execute(Transaction* tx ,vector<Column> requestedColums,Table * table) override;

};

class BinaryExpr : public Expr {
public:
    shared_ptr<Expr> left;
    shared_ptr<Expr> right;
    Token op;

    BinaryExpr(shared_ptr<Expr> left, Token op, shared_ptr<Expr> right);

    bool isLeftIdentifierExpr() const {
        return std::dynamic_pointer_cast<IdentifierExpr>(left) != nullptr;
    }

    bool isRightIdentifierExpr() const {
        return std::dynamic_pointer_cast<IdentifierExpr>(right) != nullptr;
    }

    vector<vector<string>> execute(Transaction* tx ,vector<Column> requestedColums,Table * table) override;
};
