#include <iostream>
#include <sstream>
#include <vector>

#include "./include/token.hpp"

using namespace std;

class Tokenizer {
   private:
    vector<Token> tokens;

   public:
    Tokenizer(string query) {
        // cout<<"EMII::::"<<query<<endl;
        stringstream ss(query);
        string token;
        vector<Token> tokens;
        while (ss >> token) {
            if (token == "+") {
                Token newToken("+", "", TokenType::OPERATOR);
                tokens.push_back(newToken);
            } else if (token == "-") {
                Token newToken("-", "", TokenType::OPERATOR);
                tokens.push_back(newToken);
            } else if (token == "*") {
                Token newToken("*", "", TokenType::OPERATOR);
                tokens.push_back(newToken);
            } else if (token == "/") {
                Token newToken("/", "", TokenType::OPERATOR);
                tokens.push_back(newToken);
            } else if (isalpha(token[0])) {
                if (keywordMap.find(token) != keywordMap.end()) {
                    tokens.push_back(Token(token, token, keywordMap[token]));
                } else {
                    tokens.push_back(Token(token, token, TokenType::IDENTIFIER));
                }

            } else if (isdigit(token[0])) {
                Token newToken(token, "", TokenType::LITERAL);
                tokens.push_back(newToken);
            } else if (token[0] == '\'') {
                tokens.push_back(Token(token, "", TokenType::LITERAL));
            }
            else if (token[0] == '\"') {
                tokens.push_back(Token(token, "", TokenType::LITERAL));
            }
        }

        this->tokens=tokens;
    }

    void print(){
        cout<<"TOKENS: "<<endl;
        for(auto token:tokens){
            cout<<token.lexeme<<" "<<token.tokenType<<endl;
        }

    }

    vector<Token> getTokens() const { return tokens; }
};
