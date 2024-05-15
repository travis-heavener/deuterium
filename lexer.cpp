#include <iostream>
#include <string>
#include <vector>

#include "lexer.hpp"
#include "toolbox.hpp"

void tokenize(const std::string& src, std::vector<Token>& tokens) {
    // break src string into tokens
    size_t len = src.size();
    std::string buffer;
    for (size_t i = 0; i < len; i++) {
        buffer.clear();

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
            tokens.push_back({tokenType, buffer});
            continue;
        } else if (std::isalpha(src[i])) { // keywords
            switch (src[i]) {
                case 'b':
                    if (src.find("bool") == i) {
                        i += 3;
                        tokens.push_back({TokenType::TYPE_BOOL, "bool"}); continue;
                    }
                    break; // not found
                case 'c':
                    if (src.find("char") == i) {
                        i += 3;
                        tokens.push_back({TokenType::TYPE_CHAR, "char"}); continue;
                    }
                    break; // not found
                case 'd':
                    if (src.find("double") == i) {
                        i += 5;
                        tokens.push_back({TokenType::TYPE_DOUBLE, "double"}); continue;
                    }
                    break; // not found
                case 'e':
                    if (src.find("elif") == i) {
                        i += 3;
                        tokens.push_back({TokenType::ELIF, "elif"}); continue;
                    } else if (src.find("else") == i) {
                        i += 3;
                        tokens.push_back({TokenType::ELSE, "else"}); continue;
                    }
                    break; // not found
                case 'f':
                    if (src.find("for") == i) {
                        i += 2;
                        tokens.push_back({TokenType::FOR, "for"}); continue;
                    } else if (src.find("false") == i) {
                        i += 4;
                        tokens.push_back({TokenType::LIT_BOOL, "false"}); continue;
                    } 
                    break; // not found
                case 'i':
                    if (src.find("if") == i) {
                        i++;
                        tokens.push_back({TokenType::IF, "if"}); continue;
                    } else if (src.find("int") == i) {
                        i += 2;
                        tokens.push_back({TokenType::TYPE_INT, "int"}); continue;
                    }
                    break; // not found
                case 'n':
                    if (src.find("null") == i) {
                        i += 3;
                        tokens.push_back({TokenType::LIT_NULL, "null"}); continue;
                    }
                    break; // not found
                case 'r':
                    if (src.find("return") == i) {
                        i += 5;
                        tokens.push_back({TokenType::RETURN, "return"}); continue;
                    } 
                    break; // not found
                case 's':
                    if (src.find("string") == i) {
                        i += 5;
                        tokens.push_back({TokenType::TYPE_STR, "string"}); continue;
                    } 
                    break; // not found
                case 't':
                    if (src.find("true") == i) {
                        i += 3;
                        tokens.push_back({TokenType::LIT_BOOL, "true"}); continue;
                    } 
                    break; // not found
                case 'w':
                    if (src.find("while") == i) {
                        i += 4;
                        tokens.push_back({TokenType::WHILE, "while"}); continue;
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
                    if (i == len) break; // unclosed string
                    tokens.push_back({TokenType::LIT_STR, buffer});
                    continue;
                case '\'': // characters
                    if (i + 2 < len && src[i+2] == '\'') { // normal single-digit char
                        i += 2;
                        tokens.push_back({TokenType::LIT_CHAR, src[i+1] + ""});
                        continue;
                    } else if (i + 3 < len && src[i+1] == '\\' && src[i+3] == '\'') { // escaped char
                        i += 3;
                        tokens.push_back({TokenType::LIT_CHAR, escapeChar(src.substr(i+1, 2)) + ""});
                        continue;
                    }
                    break; // unclosed char
                case ';': tokens.push_back({TokenType::SEMICOLON, ";"}); continue;
                case '(': tokens.push_back({TokenType::LPAREN, "("}); continue;
                case ')': tokens.push_back({TokenType::RPAREN, ")"}); continue;
                case '[': tokens.push_back({TokenType::LBRACKET, "["}); continue;
                case ']': tokens.push_back({TokenType::RBRACKET, "]"}); continue;
                case '{': tokens.push_back({TokenType::LBRACE, "{"}); continue;
                case '}': tokens.push_back({TokenType::RBRACE, "}"}); continue;
                case '#': tokens.push_back({TokenType::COMMENT, "#"}); continue;
                case '.': tokens.push_back({TokenType::OP_DOT, "."}); continue;
                case ',': tokens.push_back({TokenType::OP_COMMA, ","}); continue;
                case '<':
                    if (src.find("<<=") == i) {
                        i += 2;
                        tokens.push_back({TokenType::ASSIGN_LSHIFT, "<<="}); continue;
                    } else if (src.find("<<") == i) {
                        i++;
                        tokens.push_back({TokenType::OP_LSHIFT, "<<"}); continue;
                    } else if (src.find("<=") == i) {
                        i++;
                        tokens.push_back({TokenType::OP_LTE, "<="}); continue;
                    }
                    tokens.push_back({TokenType::OP_LT, "<"}); continue;
                case '>':
                    if (src.find(">>=") == i) {
                        i += 2;
                        tokens.push_back({TokenType::ASSIGN_RSHIFT, ">>="}); continue;
                    } else if (src.find(">>") == i) {
                        i++;
                        tokens.push_back({TokenType::OP_RSHIFT, ">>"}); continue;
                    } else if (src.find(">=") == i) {
                        i++;
                        tokens.push_back({TokenType::OP_GTE, ">="}); continue;
                    }
                    tokens.push_back({TokenType::OP_GT, ">"}); continue;
                case '+':
                    if (src.find("++") == i) {
                        i++;
                        tokens.push_back({TokenType::OP_INC, "++"}); continue;
                    } else if (src.find("+=") == i) {
                        i++;
                        tokens.push_back({TokenType::ASSIGN_ADD, "+="}); continue;
                    }
                    tokens.push_back({TokenType::OP_ADD, "+"}); continue;
                case '-':
                    if (src.find("--") == i) {
                        i++;
                        tokens.push_back({TokenType::OP_DEC, "--"}); continue;
                    } else if (src.find("-=") == i) {
                        i++;
                        tokens.push_back({TokenType::ASSIGN_SUB, "-="}); continue;
                    }
                    tokens.push_back({TokenType::OP_SUB, "-"}); continue;
                case '*':
                    if (src.find("*=") == i) {
                        i++;
                        tokens.push_back({TokenType::ASSIGN_MUL, "*="}); continue;
                    }
                    tokens.push_back({TokenType::OP_MUL, "*"}); continue;
                case '/':
                    if (src.find("/=") == i) {
                        i++;
                        tokens.push_back({TokenType::ASSIGN_DIV, "/="}); continue;
                    }
                    tokens.push_back({TokenType::OP_DIV, "/"}); continue;
                case '%':
                    if (src.find("%=") == i) {
                        i++;
                        tokens.push_back({TokenType::ASSIGN_MOD, "%="}); continue;
                    }
                    tokens.push_back({TokenType::OP_MOD, "%"}); continue;
                case '|':
                    if (src.find("|=") == i) {
                        i++;
                        tokens.push_back({TokenType::ASSIGN_BIT_OR, "|="}); continue;
                    } else if (src.find("||") == i) {
                        i++;
                        tokens.push_back({TokenType::OP_BOOL_OR, "||"}); continue;
                    }
                    tokens.push_back({TokenType::OP_BIT_OR, "|"}); continue;
                case '&':
                    if (src.find("&=") == i) {
                        i++;
                        tokens.push_back({TokenType::ASSIGN_BIT_AND, "&="}); continue;
                    } else if (src.find("&&") == i) {
                        i++;
                        tokens.push_back({TokenType::OP_BOOL_AND, "&&"}); continue;
                    }
                    tokens.push_back({TokenType::OP_BIT_AND, "&"}); continue;
                case '~':
                    if (src.find("~=") == i) {
                        i++;
                        tokens.push_back({TokenType::ASSIGN_BIT_NOT, "~="}); continue;
                    }
                    tokens.push_back({TokenType::OP_BIT_NOT, "~"}); continue;
                case '^':
                    if (src.find("^=") == i) {
                        i++;
                        tokens.push_back({TokenType::ASSIGN_BIT_XOR, "^="}); continue;
                    }
                    tokens.push_back({TokenType::OP_BIT_XOR, "^"}); continue;
                case '!':
                    if (src.find("!=") == i) {
                        i++;
                        tokens.push_back({TokenType::OP_NEQ, "!="}); continue;
                    }
                    tokens.push_back({TokenType::OP_BOOL_NOT, "!"}); continue;
                case '=':
                    if (src.find("==") == i) {
                        i++;
                        tokens.push_back({TokenType::OP_EQ, "=="}); continue;
                    }
                    tokens.push_back({TokenType::ASSIGN, "="}); continue;
            }
        }

        // base case, handle as identifier
        while (i < len && (std::isalnum(src[i]) || src[i] == '$' || src[i] == '_'))
            buffer.push_back(src[i++]);
        if (i != len) i--;
        tokens.push_back({TokenType::IDENTIFIER, buffer});
    }
}