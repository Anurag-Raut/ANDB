#pragma once
#include <string>
#include <unordered_map>

using namespace std;

enum TokenType {
    IDENTIFIER,
    KEYWORD,
    OPERATOR,
    LITERAL,

};
extern unordered_map<string, TokenType> keywordMap = {
    {"SELECT", TokenType::KEYWORD}, {"FROM", TokenType::KEYWORD},   {"WHERE", TokenType::KEYWORD},    {"INSERT", TokenType::KEYWORD},
    {"INTO", TokenType::KEYWORD},   {"VALUES", TokenType::KEYWORD}, {"UPDATE", TokenType::KEYWORD},   {"SET", TokenType::KEYWORD},
    {"DELETE", TokenType::KEYWORD}, {"CREATE", TokenType::KEYWORD}, {"TABLE", TokenType::KEYWORD},    {"DROP", TokenType::KEYWORD},
    {"ALTER", TokenType::KEYWORD},  {"ADD", TokenType::KEYWORD},    {"JOIN", TokenType::KEYWORD},     {"INNER", TokenType::KEYWORD},
    {"LEFT", TokenType::KEYWORD},   {"RIGHT", TokenType::KEYWORD},  {"FULL", TokenType::KEYWORD},     {"ON", TokenType::KEYWORD},
    {"GROUP", TokenType::KEYWORD},  {"BY", TokenType::KEYWORD},     {"ORDER", TokenType::KEYWORD},    {"HAVING", TokenType::KEYWORD},
    {"UNION", TokenType::KEYWORD},  {"ALL", TokenType::KEYWORD},    {"DISTINCT", TokenType::KEYWORD}, {"AS", TokenType::KEYWORD},
    {"AND", TokenType::KEYWORD},    {"OR", TokenType::KEYWORD},     {"NOT", TokenType::KEYWORD},      {"NULL", TokenType::KEYWORD},
    {"IS", TokenType::KEYWORD},     {"IN", TokenType::KEYWORD},     {"BETWEEN", TokenType::KEYWORD},  {"LIKE", TokenType::KEYWORD},
    {"LIMIT", TokenType::KEYWORD},  {"OFFSET", TokenType::KEYWORD}};

class Token {
   public:
    TokenType tokenType;
    string literal;
    string lexeme;

    Token(string literal, string lexeme, TokenType tokenType) {
        this->tokenType = tokenType;
        this->lexeme = lexeme;
        this->literal = literal;
    }
};
