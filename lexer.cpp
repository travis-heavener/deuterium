#include <iostream>
#include <string>
#include <vector>

#include "lexer.hpp"
#include "toolbox.hpp"
#include "errors.hpp"

// true if the token is of TYPE_INT, TYPE_BOOL, etc.
bool isTokenPrimitiveType(const TokenType type) {
    return type == TYPE_BOOL || type == TYPE_CHAR || type == TYPE_DOUBLE || type == TYPE_INT || type == TYPE_STR;
}

// true if the token is an unary operator (ex. ~, !)
bool isTokenUnaryOp(const TokenType type) {
    return type == OP_BOOL_NOT || type == OP_ADD || type == OP_SUB || type == OP_BIT_NOT || type == OP_INC || type == OP_DEC;
}

// true if the token is a binary operator (ex. *, /, ==)
bool isTokenBinaryOp(const TokenType type) {
    return type == OP_LT || type == OP_LTE || type == OP_GT || type == OP_GTE ||
           type == OP_LSHIFT || type == OP_RSHIFT || type == OP_ADD ||
           type == OP_SUB || type == OP_MUL || type == OP_DIV || type == OP_MOD ||
           type == OP_BIT_OR || type == OP_BIT_AND || type == OP_BIT_XOR ||
           type == OP_BOOL_OR || type == OP_BOOL_AND || type == OP_EQ || type == OP_NEQ;
}

// true if the token is a literal
bool isTokenLiteral(const TokenType type) {
    return type == LIT_BOOL || type == LIT_CHAR || type == LIT_DOUBLE || type == LIT_INT || type == LIT_NULL || type == LIT_STR;
}

// true if the token is a comparison operator
bool isTokenCompOp(const TokenType type) {
    return type == OP_LT || type == OP_LTE || type == OP_GT || type == OP_GTE ||  type == OP_BIT_OR ||
           type == OP_BIT_AND || type == OP_BIT_XOR || type == OP_BOOL_OR || type == OP_BOOL_AND ||
           type == OP_EQ || type == OP_NEQ;
}

// true if the token is an assignment operator
bool isTokenAssignOp(const TokenType type) {
    return type == ASSIGN || type == ASSIGN_ADD || type == ASSIGN_SUB || type == ASSIGN_MUL ||
           type == ASSIGN_DIV || type == ASSIGN_MOD || type == ASSIGN_LSHIFT ||
           type == ASSIGN_RSHIFT || type == ASSIGN_BIT_OR || type == ASSIGN_BIT_AND ||
           type == ASSIGN_BIT_NOT || type == ASSIGN_BIT_XOR;
};

void tokenize(const std::string& src, std::vector<Token>& tokens, const trace lineNum, const int fileIndex) {
    // break src string into tokens
    size_t len = src.size();
    std::string buffer;
    for (size_t i = 0; i < len; i++) {
        buffer.clear();
        const ErrInfo errInfo = {lineNum, i+1, fileIndex};

        if (std::isspace(src[i])) { // whitespace
            continue;
        } else if ((src[i] >= '0' && src[i] <= '9') || src[i] == '.') { // int/double literals
            TokenType tokenType = TokenType::LIT_INT;
            buffer = src[i];
            while (++i < len && ((src[i] >= '0' && src[i] <= '9') || src[i] == '.')) {
                if (src[i] == '.') tokenType = TokenType::LIT_DOUBLE;
                buffer.push_back(src[i]);
            }
            if (i != len) i--;
            tokens.push_back({tokenType, buffer, errInfo});
            continue;
        } else if (std::isalpha(src[i])) { // keywords
            switch (src[i]) {
                case 'b':
                    if (src.find("bool", i) == i) {
                        i += 3;
                        tokens.push_back({TokenType::TYPE_BOOL, "bool", errInfo}); continue;
                    }
                    break; // not found
                case 'c':
                    if (src.find("char", i) == i) {
                        i += 3;
                        tokens.push_back({TokenType::TYPE_CHAR, "char", errInfo}); continue;
                    }
                    break; // not found
                case 'd':
                    if (src.find("double", i) == i) {
                        i += 5;
                        tokens.push_back({TokenType::TYPE_DOUBLE, "double", errInfo}); continue;
                    }
                    break; // not found
                case 'e':
                    if (src.find("elif", i) == i) {
                        i += 3;
                        tokens.push_back({TokenType::ELIF, "elif", errInfo}); continue;
                    } else if (src.find("else", i) == i) {
                        i += 3;
                        tokens.push_back({TokenType::ELSE, "else", errInfo}); continue;
                    }
                    break; // not found
                case 'f':
                    if (src.find("for", i) == i) {
                        i += 2;
                        tokens.push_back({TokenType::FOR, "for", errInfo}); continue;
                    } else if (src.find("false", i) == i) {
                        i += 4;
                        tokens.push_back({TokenType::LIT_BOOL, "false", errInfo}); continue;
                    } 
                    break; // not found
                case 'i':
                    if (src.find("if", i) == i) {
                        i++;
                        tokens.push_back({TokenType::IF, "if", errInfo}); continue;
                    } else if (src.find("int", i) == i) {
                        i += 2;
                        tokens.push_back({TokenType::TYPE_INT, "int", errInfo}); continue;
                    }
                    break; // not found
                case 'n':
                    if (src.find("null", i) == i) {
                        i += 3;
                        tokens.push_back({TokenType::LIT_NULL, "null", errInfo}); continue;
                    }
                    break; // not found
                case 'r':
                    if (src.find("return", i) == i) {
                        i += 5;
                        tokens.push_back({TokenType::RETURN, "return", errInfo}); continue;
                    } 
                    break; // not found
                case 's':
                    if (src.find("string", i) == i) {
                        i += 5;
                        tokens.push_back({TokenType::TYPE_STR, "string", errInfo}); continue;
                    } 
                    break; // not found
                case 't':
                    if (src.find("true", i) == i) {
                        i += 3;
                        tokens.push_back({TokenType::LIT_BOOL, "true", errInfo}); continue;
                    } 
                    break; // not found
                case 'w':
                    if (src.find("while", i) == i) {
                        i += 4;
                        tokens.push_back({TokenType::WHILE, "while", errInfo}); continue;
                    }
                    break; // not found
            }
        } else { // misc. characters & operators
            switch (src[i]) {
                case '"': // strings
                    while (++i < len && src[i] != '"') {
                        if (src[i] == '\\') i++;
                        buffer.push_back(src[i]);
                    }
                    if (i == len) // unclosed string
                        throw DTSyntaxException(errInfo, "\"");
                    tokens.push_back({TokenType::LIT_STR, buffer, errInfo});
                    continue;
                case '\'': // characters
                    if (i + 2 < len && src[i+2] == '\'') { // normal single-digit char
                        tokens.push_back({TokenType::LIT_CHAR, src.substr(i+1, 1), errInfo});
                        i += 2;
                        continue;
                    } else if (i + 3 < len && src[i+1] == '\\' && src[i+3] == '\'') { // escaped char
                        tokens.push_back({TokenType::LIT_CHAR, escapeChar(src.substr(i+1, 2)) + "", errInfo});
                        i += 3;
                        continue;
                    }
                    // unclosed char
                    throw DTSyntaxException(errInfo, src.substr(i, 1));
                    break;
                case ';': tokens.push_back({TokenType::SEMICOLON, ";", errInfo}); continue;
                case '(': tokens.push_back({TokenType::LPAREN, "(", errInfo}); continue;
                case ')': tokens.push_back({TokenType::RPAREN, ")", errInfo}); continue;
                case '[': tokens.push_back({TokenType::LBRACKET, "[", errInfo}); continue;
                case ']': tokens.push_back({TokenType::RBRACKET, "]", errInfo}); continue;
                case '{': tokens.push_back({TokenType::LBRACE, "{", errInfo}); continue;
                case '}': tokens.push_back({TokenType::RBRACE, "}", errInfo}); continue;
                case '#': { // handle comments
                    while (i < len && src[i] != '\n') i++;
                    continue;
                }
                case '.': tokens.push_back({TokenType::DOT, ".", errInfo}); continue;
                case ',': tokens.push_back({TokenType::COMMA, ",", errInfo}); continue;
                case '<':
                    if (src.find("<<=", i) == i) {
                        i += 2;
                        tokens.push_back({TokenType::ASSIGN_LSHIFT, "<<=", errInfo}); continue;
                    } else if (src.find("<<", i) == i) {
                        i++;
                        tokens.push_back({TokenType::OP_LSHIFT, "<<", errInfo}); continue;
                    } else if (src.find("<=", i) == i) {
                        i++;
                        tokens.push_back({TokenType::OP_LTE, "<=", errInfo}); continue;
                    }
                    tokens.push_back({TokenType::OP_LT, "<", errInfo}); continue;
                case '>':
                    if (src.find(">>=", i) == i) {
                        i += 2;
                        tokens.push_back({TokenType::ASSIGN_RSHIFT, ">>=", errInfo}); continue;
                    } else if (src.find(">>", i) == i) {
                        i++;
                        tokens.push_back({TokenType::OP_RSHIFT, ">>", errInfo}); continue;
                    } else if (src.find(">=", i) == i) {
                        i++;
                        tokens.push_back({TokenType::OP_GTE, ">=", errInfo}); continue;
                    }
                    tokens.push_back({TokenType::OP_GT, ">", errInfo}); continue;
                case '+':
                    if (src.find("++", i) == i) {
                        i++;
                        tokens.push_back({TokenType::OP_INC, "++", errInfo}); continue;
                    } else if (src.find("+=", i) == i) {
                        i++;
                        tokens.push_back({TokenType::ASSIGN_ADD, "+=", errInfo}); continue;
                    }
                    tokens.push_back({TokenType::OP_ADD, "+", errInfo}); continue;
                case '-':
                    if (src.find("--", i) == i) {
                        i++;
                        tokens.push_back({TokenType::OP_DEC, "--", errInfo}); continue;
                    } else if (src.find("-=", i) == i) {
                        i++;
                        tokens.push_back({TokenType::ASSIGN_SUB, "-=", errInfo}); continue;
                    }
                    tokens.push_back({TokenType::OP_SUB, "-", errInfo}); continue;
                case '*':
                    if (src.find("*=", i) == i) {
                        i++;
                        tokens.push_back({TokenType::ASSIGN_MUL, "*=", errInfo}); continue;
                    }
                    tokens.push_back({TokenType::OP_MUL, "*", errInfo}); continue;
                case '/':
                    if (src.find("/=", i) == i) {
                        i++;
                        tokens.push_back({TokenType::ASSIGN_DIV, "/=", errInfo}); continue;
                    }
                    tokens.push_back({TokenType::OP_DIV, "/", errInfo}); continue;
                case '%':
                    if (src.find("%=", i) == i) {
                        i++;
                        tokens.push_back({TokenType::ASSIGN_MOD, "%=", errInfo}); continue;
                    }
                    tokens.push_back({TokenType::OP_MOD, "%", errInfo}); continue;
                case '|':
                    if (src.find("|=", i) == i) {
                        i++;
                        tokens.push_back({TokenType::ASSIGN_BIT_OR, "|=", errInfo}); continue;
                    } else if (src.find("||", i) == i) {
                        i++;
                        tokens.push_back({TokenType::OP_BOOL_OR, "||", errInfo}); continue;
                    }
                    tokens.push_back({TokenType::OP_BIT_OR, "|", errInfo}); continue;
                case '&':
                    if (src.find("&=", i) == i) {
                        i++;
                        tokens.push_back({TokenType::ASSIGN_BIT_AND, "&=", errInfo}); continue;
                    } else if (src.find("&&", i) == i) {
                        i++;
                        tokens.push_back({TokenType::OP_BOOL_AND, "&&", errInfo}); continue;
                    }
                    tokens.push_back({TokenType::OP_BIT_AND, "&", errInfo}); continue;
                case '~':
                    if (src.find("~=", i) == i) {
                        i++;
                        tokens.push_back({TokenType::ASSIGN_BIT_NOT, "~=", errInfo}); continue;
                    }
                    tokens.push_back({TokenType::OP_BIT_NOT, "~", errInfo}); continue;
                case '^':
                    if (src.find("^=", i) == i) {
                        i++;
                        tokens.push_back({TokenType::ASSIGN_BIT_XOR, "^=", errInfo}); continue;
                    }
                    tokens.push_back({TokenType::OP_BIT_XOR, "^", errInfo}); continue;
                case '!':
                    if (src.find("!=", i) == i) {
                        i++;
                        tokens.push_back({TokenType::OP_NEQ, "!=", errInfo}); continue;
                    }
                    tokens.push_back({TokenType::OP_BOOL_NOT, "!", errInfo}); continue;
                case '=':
                    if (src.find("==", i) == i) {
                        i++;
                        tokens.push_back({TokenType::OP_EQ, "==", errInfo}); continue;
                    }
                    tokens.push_back({TokenType::ASSIGN, "=", errInfo}); continue;
            }
        }

        // base case, handle as identifier
        while (i < len && (std::isalnum(src[i]) || src[i] == '$' || src[i] == '_'))
            buffer.push_back(src[i++]);
        if (i != len) i--;
        tokens.push_back({TokenType::IDENTIFIER, buffer, errInfo});
    }
}