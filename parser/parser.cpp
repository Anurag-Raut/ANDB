#include "./include/token.hpp"
#include <vector>
#include <string>

using namespace std;
class Parser{
    public:
    size_t currentTokenIndex = 0;
    vector<Token> tokens;

    Parser(const std::vector<Token>& t) : tokens(t) {}


    void parse( ) {
        while (currentTokenIndex < tokens.size()) {
            Token token = tokens[currentTokenIndex];
            if (token.type == TokenType::KEYWORD) {
                if (token.value == "SELECT") {
                    parseSelectStatement();
                } else if (token.value == "INSERT") {
                    parseInsertStatement();
                } else if (token.value == "UPDATE") {
                    parseUpdateStatement();
                } else if (token.value == "DELETE") {
                    parseDeleteStatement();
                } else if (token.value == "CREATE") {
                    parseCreateStatement();
                } else if (token.value == "DROP") {
                    parseDropStatement();
                } else if (token.value == "ALTER") {
                    parseAlterStatement();
                } else {
                    std::cout << "Syntax error: Unexpected keyword '" << token.value << "'\n";
                    return;
                }
            } else {
                std::cout << "Syntax error: Unexpected token '" << token.value << "'\n";
                return;
            }
        }
    }

    



};