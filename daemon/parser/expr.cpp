// #include <any>
#include "./include/expr.hpp"
#include <unordered_set>
#include <iostream>
#include <memory>
#include <stdexcept>
#include "../storage/include/table.hpp"

using namespace std;

ANDExpr::ANDExpr(shared_ptr<Expr> left, shared_ptr<Expr> right) {
    this->left = (left);
    this->right = (right);
}

vector<pair<vector<string>,pair<uint64_t,uint16_t>>> ANDExpr::execute(Transaction* tx ,vector<Column> requestedColums, Table* table) { 
    vector<pair<vector<string>,pair<uint64_t,uint16_t>>> left_rows,right_rows,response;

    left_rows=left->execute(tx,table->columns,table);
        right_rows=right->execute(tx,table->columns,table);
        cout<<"LESS GOO: "<<left_rows.size()<<" "<<right_rows.size()<<endl;

        for(auto left_row:left_rows){
            for(auto right_row:right_rows){
                
                if(left_row.first[table->primary_key_index]==right_row.first[table->primary_key_index]){
                    cout<<left_row.first[table->primary_key_index]<<right_row.first[table->primary_key_index]<<endl;
                    vector<string> row=table->Deconstruct(right_row.first,requestedColums);

                    response.push_back({row,right_row.second});
                    break;
                }
            }
        }



    return response;
}

ORExpr::ORExpr(shared_ptr<Expr> left, shared_ptr<Expr> right) {
    this->left = (left);
    this->right = (right);
}

vector<pair<vector<string>,pair<uint64_t,uint16_t>>> ORExpr::execute(Transaction* tx, vector<Column> requestedColumns, Table* table) { 
    vector<pair<vector<string>,pair<uint64_t,uint16_t>>> left_rows, right_rows, response;
    unordered_set<string> added_primary_keys;

    left_rows = left->execute(tx, table->columns, table);
    right_rows = right->execute(tx, table->columns, table);

    for (auto& left_row : left_rows) {

        response.push_back({table->Deconstruct(left_row.first,requestedColumns),left_row.second});
        added_primary_keys.insert(left_row.first[table->primary_key_index]);
    }

    for (auto& right_row : right_rows) {
        if (added_primary_keys.find(right_row.first[table->primary_key_index]) == added_primary_keys.end()) {
            response.push_back({table->Deconstruct(right_row.first,requestedColumns),right_row.second});
            added_primary_keys.insert(right_row.first[table->primary_key_index]);
        }
    }

    return response;
}

IdentifierExpr::IdentifierExpr(string name) { this->name = name; }
vector<pair<vector<string>,pair<uint64_t,uint16_t>>> IdentifierExpr::execute(Transaction* tx ,vector<Column> requestedColums, Table* table) { return {{}}; }
LiteralExpr::LiteralExpr(string literal) { this->literal = literal; }
vector<pair<vector<string>,pair<uint64_t,uint16_t>>> LiteralExpr::execute(Transaction* tx ,vector<Column> requestedColums, Table* table) { return {{}}; }

BinaryExpr::BinaryExpr(shared_ptr<Expr> left, Token op, shared_ptr<Expr> right) {
    this->left = (left);
    this->right = (right);
    this->op = op;
}

vector<pair<vector<string>,pair<uint64_t,uint16_t>>> BinaryExpr::execute(Transaction* tx ,vector<Column> requestedColums, Table* table) {
    shared_ptr<LiteralExpr> Literal;
    if (isLeftIdentifierExpr()) {
        Literal = dynamic_pointer_cast<LiteralExpr>(this->right);

    } else {
        Literal = dynamic_pointer_cast<LiteralExpr>(this->left);
    }

    if (op.value == "<") {
        return tx->RangeQuery(NULL, &Literal->literal, requestedColums, true, false,table);

    } else if (op.value == "<=") {
        return tx->RangeQuery(NULL, &Literal->literal, requestedColums, true, true,table);

    } else if (op.value == ">") {
        return tx->RangeQuery(&Literal->literal, NULL, requestedColums, false, true,table);

    } else if (op.value == ">=") {
        return tx->RangeQuery(&Literal->literal, NULL, requestedColums, true, true,table);
    }
    else if (op.value == "=") {
        return tx->RangeQuery(&Literal->literal, &Literal->literal, requestedColums, true, true,table);
    }

}

// vector<pair<vector<string>,pair<uint64_t,uint16_t>>> BinaryExpr::execute(){
//     if(isLeftIdentifierExpr() && !isRightIdentifierExpr()){

//         return Tabe

//     }
// }

// string BinaryExpr::evaluate(Environment *env) {
//   string left = this->left->evaluate(env);
//   string right = this->right->evaluate(env);

//   switch (op.type) {
//     case TokenType::PLUS:
//       if (checkType<string>({left, right})) {
//         return string(any_cast<string>(left.value) +
//                        any_cast<string>(right.value));
//       } else if (checkType<double>({left, right})) {
//         return string(any_cast<double>(left.value) +
//                        any_cast<double>(right.value));
//       } else {
//         runtime_error("both operands not same type");
//       }

//       break;
//     case TokenType::MINUS:
//       if (checkType<double>({left, right})) {
//         return string(any_cast<double>(left.value) -
//                        any_cast<double>(right.value));
//       } else {
//         runtime_error("both operands not double");
//       }

//       break;
//     case TokenType::STAR:
//       if (checkType<double>({left, right})) {
//         return string(any_cast<double>(left.value) *
//                        any_cast<double>(right.value));
//       } else {
//         runtime_error("both operands not double");
//       }

//       break;

//     case TokenType::SLASH:
//       if (checkType<double>({left, right})) {
//         // cout << "double" << endl;

//         return string(any_cast<double>(left.value) /
//                        any_cast<double>(right.value));
//       } else {
//         runtime_error("both operands not double");
//       }

//       break;

//     case TokenType::EQUAL_EQUAL:
//       if (checkType<bool>({left, right})) {
//         if (any_cast<bool>(left.value) == any_cast<bool>(right.value)) {
//           return string(true);
//         } else {
//           return string(false);
//         }
//       } else if (checkType<double>({left, right})) {
//         if (any_cast<double>(left.value) == any_cast<double>(right.value)) {
//           return string(true);
//         } else {
//           return string(false);
//         }
//       } else if (checkType<string>({left, right})) {
//         if (any_cast<string>(left.value) == any_cast<string>(right.value)) {
//           return string(true);
//         } else {
//           return string(false);
//         }
//       }

//       break;

//     case TokenType::BANG_EQUAL:
//       if (checkType<bool>({left, right})) {
//         if (any_cast<bool>(left.value) != any_cast<bool>(right.value)) {
//           return string(true);
//         } else {
//           return string(false);
//         }
//       }

//       break;

//     case TokenType::GREATER:
//       if (checkType<double>({left, right})) {
//         if (any_cast<double>(left.value) > any_cast<double>(right.value)) {
//           return string(true);
//         } else {
//           return string(false);
//         }
//       }
//       break;
//     case TokenType::GREATER_EQUAL:
//       if (checkType<double>({left, right})) {
//         if (any_cast<double>(left.value) >= any_cast<double>(right.value)) {
//           return string(true);
//         } else {
//           return string(false);
//         }
//       }
//       break;
//     case TokenType::LESS:
//       if (checkType<double>({left, right})) {
//         if (any_cast<double>(left.value) < any_cast<double>(right.value)) {
//           return string(true);
//         } else {
//           return string(false);
//         }
//       }
//       break;
//     case TokenType::LESS_EQUAL:
//       if (checkType<double>({left, right})) {
//         if (any_cast<double>(left.value) <= any_cast<double>(right.value)) {
//           return string(true);
//         } else {
//           return string(false);
//         }
//       }

//       break;
//     default:
//       runtime_error("No matching operation defined");
//       break;
//   }
// }

// UnaryExpr::UnaryExpr(Token op, std::shared_ptr<Expr> right) {
//   this->right = (right);
//   this->op = op;
// }

// string UnaryExpr::evaluate(Environment *env) {
//   string right = this->right->evaluate(env);

//   switch (op.type) {
//     case TokenType::BANG:
//       return !any_cast<bool>(right.value);
//       break;

//     default:
//       break;
//   }
// }

// LiteralExpr::LiteralExpr() { this->literal = string(); }

// string LiteralExpr::evaluate(Environment *env) {
//   // literal.printLiteral();
//   // cout<<"gegeg"<<endl;

//   return literal;
// }

// string IdentifierExpr::evaluate(Environment *env) {
//   // literal.printLiteral();
//   // cout<<"sad "<<name<<endl;
//   // variables[name].printLiteral();

//   return env->getVariable(name, env);
// }

// GroupingExpr::GroupingExpr(std::shared_ptr<Expr> expr) {
//   this->expr = (expr);
// }

// // string GroupingExpr::evaluate(Environment *env) { return expr->evaluate(env); }

// // CallExpr::CallExpr(string identidier, shared_ptr<vector<shared_ptr<Expr>>> args) {
// //   this->identidier = identidier;
// //   this->args = (args);
// // }

// string CallExpr::evaluate(Environment *env) {
//     shared_ptr<FuncDecl> func= env->getFunction(this->identidier,env);

//     string val=func->executeArgs((this->args),env);
//     // cout<<"received call value for "<<identidier<<" is :";
//     // val.printLiteral();

//     return val;
// }
