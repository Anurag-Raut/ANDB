
#include <iostream>
#include <memory>  // for std::unique_ptr
#include <optional>
#include <sstream>
#include <string>
#include <vector>

#include "./include/expr.hpp"
#include "./include/token.hpp"
#include "./statement.cpp"

using namespace std;

class Parser {
   public:
    size_t currentTokenIndex = 0;
    vector<Token> tokens;
    Database* database;

    Parser(const std::vector<Token>& t,Database* db) : tokens(t),database(db) {}

    vector<unique_ptr<Statement>> parse() {
        vector<unique_ptr<Statement>> stmts;
        while (currentTokenIndex < tokens.size()) {
            Token token = tokens[currentTokenIndex];
            if (token.type == TokenType::KEYWORD) {
                if (token.value == "SELECT") {
                    stmts.push_back(parseSelectStatement());
                } else if (token.value == "INSERT") {
                    stmts.push_back(parseInsertStatement());

                } else if (token.value == "UPDATE") {
                    stmts.push_back(parseUpdateStatement());
                } else if (token.value == "DELETE") {
                    stmts.push_back(parseDeleteStatement());
                } else if (token.value == "CREATE") {
                    stmts.push_back(parseCreateStatement());
                } else if (token.value == "BEGIN") {
                    stmts.push_back(parseBeginStatement());
                } else if (token.value == "COMMIT") {
                    stmts.push_back(parseCommitStatement());
                } else if (token.value == "ROLLBACK") {
                    stmts.push_back(parseRollbackStatement());
                }
                // else if (token.value == "DROP") {
                //     return parseDropStatement();
                // } else if (token.value == "ALTER") {
                //     return parseAlterStatement();
                // }
                else {
                    return {};
                }
            } else {
                return {};
            }
        }
        return stmts;
    }

   private:
    bool match(vector<TokenType> types) {
        for (auto type : types) {
            if (tokens[currentTokenIndex].type == type) {
                return true;
            }
        }

        return false;
    }

    void consume(TokenType type, const string& error_message) {
    if (currentTokenIndex < tokens.size() && tokens[currentTokenIndex].type == type) {
        currentTokenIndex++;
    } else {
        throw runtime_error(error_message);
    }
}

    unique_ptr<SelectStatement> parseSelectStatement() {
        string command;
        string table_name;
        vector<string> columns;
        shared_ptr<Expr> where_condition;
        command = tokens[currentTokenIndex].value;
            consume(TokenType::KEYWORD, "EXPecTED KEyWORD SELECT");
        if (match({TokenType::OPERATOR}) && tokens[currentTokenIndex].value == "*") {
            consume(TokenType::OPERATOR, "EXPecTED * ");
            
        } else {
            columns.push_back(tokens[currentTokenIndex].value);
            consume(TokenType::IDENTIFIER, "EXPECTED A IDENTIFIER");
            while (match({TokenType::COMMA})) {
                consume(TokenType::COMMA, "EXpected comma");
                columns.push_back(tokens[currentTokenIndex].value);
                consume({TokenType::IDENTIFIER}, "Expected a identifier");
            }
        }
        if (match({TokenType::KEYWORD}) && tokens[currentTokenIndex].value == "FROM") {
            consume(TokenType::KEYWORD, "EXPECTE kEY WORD FROM");
            table_name = tokens[currentTokenIndex].value;
            consume(TokenType::IDENTIFIER, "EXPECTE a table name");

        } else {
            __throw_runtime_error("ERROR EXPECTED from ");
        }

        if (match({TokenType::KEYWORD}) && tokens[currentTokenIndex].value == "WHERE") {
            consume(TokenType::KEYWORD, "EXPECTE kEY WORD WHERE");
            where_condition = parseExpression();
        }
        SelectStatement statement(table_name, columns, where_condition);

        return make_unique<SelectStatement>(statement);
    }

    shared_ptr<Expr> parseExpression() { return AND(); }
    shared_ptr<Expr> AND() {
        shared_ptr<Expr> left = OR();
        if (match({TokenType::AND})) {
            Token op = tokens[currentTokenIndex];
            currentTokenIndex++;
            shared_ptr<Expr> right = AND();

            return make_shared<ANDExpr>((left), (right));
        }

        return left;
    }

    shared_ptr<Expr> OR() {
        shared_ptr<Expr> left = comparison();
        if (match({TokenType::OR})) {
            Token op = tokens[currentTokenIndex];
            currentTokenIndex++;
            shared_ptr<Expr> right = OR();

            return make_shared<ORExpr>((left), (right));
        }

        return left;
    }

    shared_ptr<Expr> comparison() {
        shared_ptr<Expr> left = primary();
        if (match({TokenType::GREATER, TokenType::GREATER_EQUAL, TokenType::LESS, TokenType::LESS_EQUAL, TokenType::EQUAL})) {
            Token op = tokens[currentTokenIndex];
            currentTokenIndex++;
            shared_ptr<Expr> right = comparison();
            return std::make_shared<BinaryExpr>((left), op, (right));
        }

        return left;
    }

    // shared_ptr<Expr> term() {
    //     shared_ptr<Expr> left = factor();
    //     if (match({TokenType::PLUS, TokenType::MINUS})) {
    //         Token op = tokens[currentTokenIndex];
    //         currentTokenIndex++;
    //         shared_ptr<Expr> right = term();

    //         return std::make_shared<BinaryExpr>((left), op, (right));
    //     }

    //     return left;
    // }

    // shared_ptr<Expr> factor() {
    //     shared_ptr<Expr> left = unary();
    //     if (match({TokenType::STAR, TokenType::SLASH})) {
    //         Token op = tokens[currentTokenIndex];
    //         currentTokenIndex++;
    //         shared_ptr<Expr> right = factor();

    //         return std::make_shared<BinaryExpr>((left), op, (right));
    //     }

    //     return left;
    // }

    // shared_ptr<Expr> unary() {
    //     // Expr left = unary();
    //     if (match({TokenType::BANG})) {
    //         Token op = tokens[currentTokenIndex];
    //         currentTokenIndex++;
    //         shared_ptr<Expr> right = unary();

    //         return std::make_shared<UnaryExpr>((op), (right));
    //     }
    //     return primary();
    // }

    shared_ptr<Expr> primary() {

        if (match({TokenType::LITERAL})) {
            currentTokenIndex++;
            return std::make_shared<LiteralExpr>(tokens[currentTokenIndex - 1].value);
        }
        //    if (match({TokenType::IDENTIFIER})) {
        //     currentTokenIndex++;
        //     return std::make_shared<LiteralExpr>("false");
        // }

        // if (match({TokenType::NUMBER, TokenType::STRING})) {
        //     currentTokenIndex++;
        //     return std::make_shared<LiteralExpr>(tokens[currentTokenIndex - 1].value);
        // }
        // if (match({TokenType::LEFT_PAREN})) {
        //     shared_ptr<Expr> expr = parseExpression();
        //     consume(RIGHT_PAREN, "Expect ')' after parseExpression.");
        //     return std::make_shared<GroupingExpr>((expr));
        // }
        if (match({TokenType::IDENTIFIER})) {
            currentTokenIndex++;
            string identifier = tokens[currentTokenIndex - 1].value;

            return make_shared<IdentifierExpr>((tokens[currentTokenIndex - 1].value));
            // if(variables.find(tokens[currentTokenIndex].lexeme)!=variables.end()){
            //   currentTokenIndex++;
            //   return std::make_shared<LiteralExpr>(variables[tokens[currentTokenIndex -
            //   1].lexeme]);
            // }
            // else{
            //   runtime_error("Undefined varaible");
            // }
        }
    }
    // optional<Statement> parseInsertStatement() {}
    // optional<Statement> parseUpdateStatement() {}
    // optional<Statement> parseDeleteStatement() {}
    unique_ptr<CreateStatement> parseCreateStatement() {
        string table_name;
        consume(TokenType::KEYWORD, "EXPECTED CREATE");
        if (tokens[currentTokenIndex].value != "TABLE") {
            throw runtime_error("EXPECTED KEYWORD TABLE");
        }
        consume(TokenType::KEYWORD, "EXPECTED TABLE");
        table_name = tokens[currentTokenIndex].value;
        consume(TokenType::IDENTIFIER, "EXPECTED table name");

        consume(TokenType::LEFT_PAREN, "EXPecTED LEFT PARENTHESIS");
        vector<Column> columns;

        while (!match({TokenType::RIGHT_PAREN})) {
            string name, type;
            name = tokens[currentTokenIndex].value;
            consume(TokenType::IDENTIFIER, "EXPECTED A IDENTIFIER");
            type = tokens[currentTokenIndex].value;
            consume(TokenType::KEYWORD, "EXPECTED A IDENTIFIER");

            transform(type.begin(), type.end(), type.begin(), ::tolower);
            columns.push_back(Column{name, type});

            if (match({TokenType::COMMA})) {
                consume(TokenType::COMMA, "EXPECTED COMMA");
            } else {
                consume(TokenType::RIGHT_PAREN, "EXPECTED RIGHT PARENTHESIS");
                break;
            }
        }
        return make_unique<CreateStatement>(table_name, columns);
    }

    unique_ptr<InsertStatement> parseInsertStatement() {
        string table_name;
        consume(TokenType::KEYWORD, "EXPECTED INSERT");
        if (tokens[currentTokenIndex].value != "INTO") {
            throw runtime_error("EXPECTED KEYWORD TABLE");
        }

        consume(TokenType::KEYWORD, "EXPECTED INTO");
        table_name = tokens[currentTokenIndex].value;
        consume(TokenType::IDENTIFIER, "EXPECTED TABLE NAME");

        consume(TokenType::KEYWORD, "EXPECTED VALUES");

        consume(TokenType::LEFT_PAREN, "EXPECTED LEFT PAREN");
        vector<string> values;
        while (!match({TokenType::RIGHT_PAREN})) {
            string value;
            value = tokens[currentTokenIndex].value;
            consume(TokenType::LITERAL, "EXPECTED A LITERAL");

            values.push_back(value);

            if (match({TokenType::COMMA})) {
                consume(TokenType::COMMA, "EXPECTED COMMA");
            } else {
                consume(TokenType::RIGHT_PAREN, "EXPECTED RIGHT PARENTHESIS");
                break;
            }
        }

        
        Table* table=database->tables[table_name];
        vector<string >cols;
        for(auto col:table->columns){
            cols.push_back(col.name);
        }
      

        return make_unique<InsertStatement>(table_name, cols, values);
    }

    unique_ptr<DeleteStatement> parseDeleteStatement() {
        string table_name;
        shared_ptr<Expr> where_condition;
        consume(TokenType::KEYWORD, "EXPECTED DELETE");
        if (tokens[currentTokenIndex].value != "FROM") {
            throw runtime_error("EXPECTED KEYWORD TABLE");
        }

        consume(TokenType::KEYWORD, "EXPECTED FROM");
        table_name = tokens[currentTokenIndex].value;
        consume(TokenType::IDENTIFIER, "EXPECTED TABLE NAME");

        if (match({TokenType::KEYWORD}) && tokens[currentTokenIndex].value == "WHERE") {
            consume(TokenType::KEYWORD, "EXPECTE kEY WORD WHERE");
            where_condition = parseExpression();
        }

        return make_unique<DeleteStatement>(table_name, where_condition);
    }

    unique_ptr<UpdateStatement> parseUpdateStatement() {
        string table_name;
        shared_ptr<Expr> where_condition;
        consume(TokenType::KEYWORD, "EXPECTED UPDATE");

        table_name = tokens[currentTokenIndex].value;
        consume(TokenType::IDENTIFIER, "EXPECTED TABLE NAME");

        consume(TokenType::KEYWORD, "EXPECTED SET");
        vector<pair<string, string>> newColumnValues;
        string columnName;
        string newColumnValue;
        columnName = tokens[currentTokenIndex].value;
        consume(TokenType::IDENTIFIER, "EXPECTED Column  NAME");
        consume(TokenType::EQUAL, "EXPECTED = ");
        newColumnValue = tokens[currentTokenIndex].value;

        consume(TokenType::LITERAL, "EXPECTED Literal ");

        newColumnValues.push_back({columnName, newColumnValue});

        while (match({TokenType::COMMA})) {
            consume({TokenType::COMMA}, "EXPECTED COMMA");
            columnName = tokens[currentTokenIndex].value;
            consume(TokenType::IDENTIFIER, "EXPECTED Column  NAME");
            consume(TokenType::EQUAL, "EXPECTED = ");
            newColumnValue = tokens[currentTokenIndex].value;

            consume(TokenType::LITERAL, "EXPECTED Literal ");
            newColumnValues.push_back({columnName, newColumnValue});
        }

        if (match({TokenType::KEYWORD}) && tokens[currentTokenIndex].value == "WHERE") {
            consume(TokenType::KEYWORD, "EXPECTE kEY WORD WHERE");
            where_condition = parseExpression();
        }

        return make_unique<UpdateStatement>(table_name, newColumnValues, where_condition);
    }

    unique_ptr<BeginStatement> parseBeginStatement() {
        consume(TokenType::KEYWORD, "EXPECTED KEYWORD BEGIN");
        return make_unique<BeginStatement>();
    }
    unique_ptr<CommitStatement> parseCommitStatement() {
        consume(TokenType::KEYWORD, "EXPECTED KEYWORD BEGIN");
        return make_unique<CommitStatement>();
    }
    unique_ptr<RollbackStatement> parseRollbackStatement() {
        consume(TokenType::KEYWORD, "EXPECTED KEYWORD BEGIN");
        return make_unique<RollbackStatement>();
    }

    // optional<Statement> parseDropStatement() {}
    // optional<Statement> parseAlterStatement() {}
};