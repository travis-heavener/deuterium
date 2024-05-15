#ifndef __LEXER_HPP
#define __LEXER_HPP

#include <string>
#include <vector>

enum TokenType {
    RETURN,
    SEMICOLON,
    IDENTIFIER,
    IF, ELIF, ELSE,
    WHILE, FOR,
    LPAREN, RPAREN, // ex. ()
    LBRACKET, RBRACKET, // ex. []
    LBRACE, RBRACE, // ex. {}
    TYPE_INT, TYPE_DOUBLE, TYPE_CHAR, TYPE_BOOL, TYPE_STR, // ex. int, double, char, bool, string
    LIT_INT, // ex. 1, -24, 0
    LIT_DOUBLE, // ex. 1.4, -24.9, 0.12
    LIT_BOOL, // ex. true, false
    LIT_NULL, // ex. null
    LIT_CHAR, // ex. 'a', 'f'
    LIT_STR, // ex. "Hello World"
    COMMENT, // #

    // operators
    OP_DOT, OP_COMMA, // ., ,
    OP_LT, OP_LTE, // <, <=
    OP_GT, OP_GTE, // >, >=
    OP_LSHIFT, OP_RSHIFT, // <<, >>
    OP_INC, OP_DEC, // ++, --
    OP_ADD, OP_SUB, OP_MUL, OP_DIV, OP_MOD, // +, -, *, /, %
    OP_BIT_OR, OP_BIT_AND, OP_BIT_NOT, OP_BIT_XOR, // |, &, ~, ^
    OP_BOOL_OR, OP_BOOL_AND, OP_BOOL_NOT, // ||, &&, !
    OP_EQ, OP_NEQ, // ==, !=

    // assignments
    ASSIGN, // =
    ASSIGN_ADD, ASSIGN_SUB, ASSIGN_MUL, ASSIGN_DIV, ASSIGN_MOD, // +=, -=, *=, /=, %=
    ASSIGN_LSHIFT, ASSIGN_RSHIFT, // <<=, >>=
    ASSIGN_BIT_OR, ASSIGN_BIT_AND, ASSIGN_BIT_NOT, ASSIGN_BIT_XOR, // |=, &=, ~=, ^=
};

struct Token {
    TokenType type;
    std::string raw;
};

void tokenize(const std::string&, std::vector<Token>&);

#endif