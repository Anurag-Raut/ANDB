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

    Parser(const std::vector<Token>& t) : tokens(t) {}

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
                }
                // else if (token.value == "DROP") {
                //     return parseDropStatement();
                // } else if (token.value == "ALTER") {
                //     return parseAlterStatement();
                // }
                else {
                    std::cout << "Syntax error: Unexpected keyword '" << token.value << "'\n";
                    return {};
                }
            } else {
                std::cout << "Syntax error: Unexpected token '" << token.value << "'\n";
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

    void consume(TokenType type, string error_message) {
        if (tokens[currentTokenIndex].type == type) {
            currentTokenIndex++;
            return;
        } else {
            __throw_runtime_error("error");
        }
    }

    unique_ptr<SelectStatement> parseSelectStatement() {
        string command;
        string table_name;
        vector<string> columns;
        shared_ptr<Expr> where_condition;
        command = tokens[currentTokenIndex].value;
        consume(TokenType::KEYWORD, "EXPecTED KEyWORD SELECT");
        columns.push_back(tokens[currentTokenIndex].value);
        consume(TokenType::IDENTIFIER, "EXPECTED A IDENTIFIER");
        while (match({TokenType::COMMA})) {
            consume(TokenType::COMMA, "EXpected comma");
            columns.push_back(tokens[currentTokenIndex].value);
            consume({TokenType::IDENTIFIER}, "Expected a identifier");
        }
        if (match({TokenType::KEYWORD}) && tokens[currentTokenIndex].value == "FROM") {
            consume(TokenType::KEYWORD, "EXPECTE kEY WORD FROM");
            table_name = tokens[currentTokenIndex].value;
            consume(TokenType::IDENTIFIER, "EXPECTE a table name");

        } else {
            // tokens[currentTokenIndex].value
            __throw_runtime_error("ERROR EXPECTED from ");
        }

        if (match({TokenType::KEYWORD}) && tokens[currentTokenIndex].value == "WHERE") {
            consume(TokenType::KEYWORD, "EXPECTE kEY WORD WHERE");
            where_condition = parseExpression();
        }
        SelectStatement statement(table_name, columns, where_condition);

        return make_unique<SelectStatement>(statement);

        // if(match({TokenType::KEYWORD}) && tokens[currentTokenIndex].value=="where"){

        // }
    }

    shared_ptr<Expr> parseExpression() { return AND(); }
    shared_ptr<Expr> AND() {
        shared_ptr<Expr> left = OR();
        if (match({TokenType::EQUAL_EQUAL, TokenType::BANG_EQUAL})) {
            Token op = tokens[currentTokenIndex];
            currentTokenIndex++;
            shared_ptr<Expr> right = AND();

            return make_shared<ANDExpr>((left), (right));
        }

        return left;
    }

    shared_ptr<Expr> OR() {
        shared_ptr<Expr> left = comparison();
        if (match({TokenType::EQUAL_EQUAL, TokenType::BANG_EQUAL})) {
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
            // cout << "OPPP " << op.lexeme << endl;
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
        // cout<<tokens[currentTokenIndex].lexeme<<endl;

        if (match({TokenType::LITERAL})) {
            currentTokenIndex++;
            return std::make_shared<LiteralExpr>(tokens[currentTokenIndex - 1].value);
        }
        //    if (match({TokenType::IDENTIFIER})) {
        //     currentTokenIndex++;
        //     return std::make_shared<LiteralExpr>("false");
        // }

        // if (match({TokenType::NUMBER, TokenType::STRING})) {
        //     // cout << "Number " << tokens[currentTokenIndex].lexeme << endl;
        //     currentTokenIndex++;
        //     return std::make_shared<LiteralExpr>(tokens[currentTokenIndex - 1].value);
        // }
        // if (match({TokenType::LEFT_PAREN})) {
        //     shared_ptr<Expr> expr = parseExpression();
        //     consume(RIGHT_PAREN, "Expect ')' after parseExpression.");
        //     return std::make_shared<GroupingExpr>((expr));
        // }
        if (match({TokenType::IDENTIFIER})) {
            // cout<<"hellp "<<tokens[currentTokenIndex].lexeme<<endl;
            currentTokenIndex++;
            string identifier = tokens[currentTokenIndex - 1].value;
            // cout << "identier " << identifier << endl;
            // cout << tokens[currentTokenIndex].lexeme << endl;

            return make_shared<IdentifierExpr>((tokens[currentTokenIndex - 1].value));
            // if(variables.find(tokens[currentTokenIndex].lexeme)!=variables.end()){
            //   currentTokenIndex++;
            //   cout<<"got hrerere"<<endl;
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
        cout << "BHAII" << endl;
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
            cout << "ONUS" << endl;
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

        vector<string> columns;

        return make_unique<InsertStatement>(table_name, columns, values);
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

        newColumnValues.push_back({columnName,newColumnValue});

        while (match({TokenType::COMMA})) {
            consume({TokenType::COMMA}, "EXPECTED COMMA");
            columnName = tokens[currentTokenIndex].value;
            consume(TokenType::IDENTIFIER, "EXPECTED Column  NAME");
            consume(TokenType::EQUAL, "EXPECTED = ");
            newColumnValue = tokens[currentTokenIndex].value;

            consume(TokenType::LITERAL, "EXPECTED Literal ");
            newColumnValues.push_back({columnName,newColumnValue});

        }

        if (match({TokenType::KEYWORD}) && tokens[currentTokenIndex].value == "WHERE") {
            consume(TokenType::KEYWORD, "EXPECTE kEY WORD WHERE");
            where_condition = parseExpression();
        }

        return make_unique<UpdateStatement>(table_name,newColumnValues, where_condition);
    }
    // optional<Statement> parseDropStatement() {}
    // optional<Statement> parseAlterStatement() {}
};