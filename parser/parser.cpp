#include <iostream>
#include <string>
#include <vector>
#include <optional>
#include <memory> // for std::unique_ptr


#include "./statement.cpp"
#include "./include/token.hpp"

using namespace std;
class Parser {
   public:
    size_t currentTokenIndex = 0;
    vector<Token> tokens;

    Parser(const std::vector<Token>& t) : tokens(t) {}

    unique_ptr<Statement> parse() {
        while (currentTokenIndex < tokens.size()) {
            Token token = tokens[currentTokenIndex];
            if (token.type == TokenType::KEYWORD) {
                if (token.value == "SELECT") {
                    return parseSelectStatement();
                } 
                // else if (token.value == "INSERT") {
                //     return &parseInsertStatement();
                // } else if (token.value == "UPDATE") {
                //     return &parseUpdateStatement();
                // } else if (token.value == "DELETE") {
                //     return parseDeleteStatement();
                // } else if (token.value == "CREATE") {
                //     return parseCreateStatement();
                // } else if (token.value == "DROP") {
                //     return parseDropStatement();
                // } else if (token.value == "ALTER") {
                //     return parseAlterStatement();
                // } 
                else {
                    std::cout << "Syntax error: Unexpected keyword '" << token.value << "'\n";
                    return nullptr;
                }
            } else {
                std::cout << "Syntax error: Unexpected token '" << token.value << "'\n";
                return nullptr;
            }
        }
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
        string where_condition;
        command = tokens[currentTokenIndex].value;
        consume(TokenType::KEYWORD, "EXPecTED KEyWORD SELECT");
        columns.push_back(tokens[currentTokenIndex].value);
        consume(TokenType::IDENTIFIER, "EXPECTED A IDENTIFIER");
        while (match({TokenType::COMMA})) {
            consume(TokenType::COMMA, "EXpected comma");
            consume({TokenType::IDENTIFIER}, "Expected a identifier");
        }
        if (match({TokenType::KEYWORD}) && tokens[currentTokenIndex].value == "from") {
            consume(TokenType::KEYWORD, "EXPECTE kEY WORD FROM");
            table_name = tokens[currentTokenIndex].value;
            consume(TokenType::IDENTIFIER, "EXPECTE a table name");

        } else {
            // tokens[currentTokenIndex].value
            __throw_runtime_error("ERROR EXPECTED from ");
        }
            SelectStatement statement(table_name, columns, where_condition);


    
        

        return make_unique<SelectStatement>(statement);

        // if(match({TokenType::KEYWORD}) && tokens[currentTokenIndex].value=="where"){

        // }
    }
    // optional<Statement> parseInsertStatement() {}
    // optional<Statement> parseUpdateStatement() {}
    // optional<Statement> parseDeleteStatement() {}
    // optional<Statement> parseCreateStatement() {}
    // optional<Statement> parseDropStatement() {}
    // optional<Statement> parseAlterStatement() {}
};