#pragma once
#include <string>
#include <unordered_map>

using namespace std;

enum TokenType {
    IDENTIFIER,
    KEYWORD,
    OPERATOR,
    LITERAL,
    COMMA,
    LEFT_PAREN,
    RIGHT_PAREN,
    LEFT_BRACE,
    RIGHT_BRACE,
    DOT,
    SEMICOLON,
    SLASH,
    STAR,
    BANG,
    BANG_EQUAL,
    EQUAL,
    EQUAL_EQUAL,
    GREATER,
    GREATER_EQUAL,
    LESS,
    LESS_EQUAL,
    STRING,
    NUMBER,
    AND,
    CLASS,
    ELSE,
    FALSE,
    TRUE,
    PLUS,
    MINUS,

    OR,

};
extern unordered_map<string, TokenType> keywordMap = {
    {"SELECT", TokenType::KEYWORD}, {"FROM", TokenType::KEYWORD},   {"WHERE", TokenType::KEYWORD},    {"INSERT", TokenType::KEYWORD},
    {"INTO", TokenType::KEYWORD},   {"VALUES", TokenType::KEYWORD}, {"UPDATE", TokenType::KEYWORD},   {"SET", TokenType::KEYWORD},
    {"DELETE", TokenType::KEYWORD}, {"CREATE", TokenType::KEYWORD}, {"TABLE", TokenType::KEYWORD},    {"DROP", TokenType::KEYWORD},
    {"ALTER", TokenType::KEYWORD},  {"ADD", TokenType::KEYWORD},    {"JOIN", TokenType::KEYWORD},     {"INNER", TokenType::KEYWORD},
    {"LEFT", TokenType::KEYWORD},   {"RIGHT", TokenType::KEYWORD},  {"FULL", TokenType::KEYWORD},     {"ON", TokenType::KEYWORD},
    {"GROUP", TokenType::KEYWORD},  {"BY", TokenType::KEYWORD},     {"ORDER", TokenType::KEYWORD},    {"HAVING", TokenType::KEYWORD},
    {"UNION", TokenType::KEYWORD},  {"ALL", TokenType::KEYWORD},    {"DISTINCT", TokenType::KEYWORD}, {"AS", TokenType::KEYWORD},
    {"and", TokenType::KEYWORD},    {"or", TokenType::KEYWORD},     {"not", TokenType::KEYWORD},      {"NULL", TokenType::KEYWORD},
    {"IS", TokenType::KEYWORD},     {"IN", TokenType::KEYWORD},     {"BETWEEN", TokenType::KEYWORD},  {"LIKE", TokenType::KEYWORD},
    {"LIMIT", TokenType::KEYWORD},  {"OFFSET", TokenType::KEYWORD},
    {"INT",TokenType::KEYWORD},
    {"STRING",TokenType::KEYWORD},
    {"BEGIN", TokenType::KEYWORD},    {"COMMIT", TokenType::KEYWORD},    {"ROLLBACK", TokenType::KEYWORD},
    {"BOOL",TokenType::KEYWORD},

    };

class Token {
   public:
    TokenType type;
    string value;
    Token()=default;
    Token(string value, TokenType type) {
        this->type = type;
        this->value = value;
    }
};
