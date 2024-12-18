#include <iostream>
#include <sstream>
#include <vector>

#include "./include/token.hpp"

using namespace std;

class Tokenizer {
   public:
    vector<Token> tokens;
    Tokenizer(string query) {

        int i = 0;
        while (i < query.size()) {
            if (isspace(query[i])) {
                // Skip whitespace
                ++i;
                continue;
            }

            if (query[i] == '+') {
                tokens.push_back(Token(string(1, query[i]), TokenType::OPERATOR));
                ++i;
            } else if (query[i] == '-') {
                tokens.push_back(Token(string(1, query[i]), TokenType::OPERATOR));
                ++i;
            } else if (query[i] == '*') {
                tokens.push_back(Token(string(1, query[i]), TokenType::OPERATOR));
                ++i;
            } else if (query[i] == '/') {
                tokens.push_back(Token(string(1, query[i]), TokenType::OPERATOR));
                ++i;
            } else if (query[i] == ',') {
                tokens.push_back(Token(string(1, query[i]), TokenType::COMMA));
                ++i;
            }else if (query[i] == '=') {
                tokens.push_back(Token(string(1, query[i]), TokenType::EQUAL));
                ++i;
            }
            else if (query[i] == '(') {
                tokens.push_back(Token(string(1, query[i]), TokenType::LEFT_PAREN));
                ++i;
            }
            else if (query[i] == ')') {
                tokens.push_back(Token(string(1, query[i]), TokenType::RIGHT_PAREN));
                ++i;
            }
             else if (isalpha(query[i])) {
                string token;
                while (i < query.size() && (isalpha(query[i]) || isdigit(query[i]) || query[i] == '_')) {
                    token += query[i];
                    ++i;
                }
                if (keywordMap.find(token) != keywordMap.end()) {
                    tokens.push_back(Token(token, keywordMap[token]));
                } else {
                    tokens.push_back(Token(token, TokenType::IDENTIFIER));
                }
            } else if (isdigit(query[i])) {
                string token;
                while (i < query.size() && isdigit(query[i])) {
                    token += query[i];
                    ++i;
                }
                tokens.push_back(Token(token, TokenType::LITERAL));
            } else if (query[i] == '\'') {
                string token;
                token += query[i];
                ++i;
                while (i < query.size() && query[i] != '\'') {
                    token += query[i];
                    ++i;
                }
                if (i < query.size()) {
                    token += query[i];
                    ++i;
                }
                tokens.push_back(Token(token, TokenType::LITERAL));
            } else if (query[i] == '\"') {
                string token;
                // token += query[i];
                ++i;
                while (i < query.size() && query[i] != '\"') {
                    token += query[i];
                    ++i;
                }

                tokens.push_back(Token(token, TokenType::LITERAL));
                i++;
            } else if (query[i] == '<') {
                if (i + 1 < query.size() && query[i + 1] == '=') {
                    tokens.push_back(Token("<=", TokenType::LESS_EQUAL));
                    i += 2;
                } else {
                    tokens.push_back(Token("<", TokenType::LESS));
                    ++i;
                }
            } else if (query[i] == '>') {
                if (i + 1 < query.size() && query[i + 1] == '=') {
                    tokens.push_back(Token(">=", TokenType::GREATER_EQUAL));
                    i += 2;
                } else {
                    tokens.push_back(Token(">", TokenType::GREATER));
                    ++i;
                }
            } else {
                ++i;  // Skip unrecognized characters
            }
        }
    }

    void print() {
        for (auto token : tokens) {
        }
    }

    vector<Token> getTokens() const { return tokens; }
};
