#include <iostream>

#include "parser.hpp"
#include "lexer.hpp" // for Tokens & types
#include "ast/ast.hpp"
#include "ast/ast_nodes.hpp"
#include "errors.hpp"
#include "exp_parser.hpp"

const Token* peekBack(const std::vector<Token>& tokens, size_t i) {
    return (i == 0) ? nullptr : &(tokens[i-1]);
}

const Token* peek(const std::vector<Token>& tokens, size_t i) {
    return (i+1 == tokens.size()) ? nullptr : &(tokens[i+1]);
}

// for function definitions
ASTNode* parseFunction(const std::vector<Token>& tokens, size_t start, size_t endParen, size_t endBrace) {
    ASTFunction* pNode = new ASTFunction(tokens[start+1].raw, tokens[start]); // name & return type
    
    // extract params between parenthesis
    std::vector<std::pair<std::string, TokenType>> params; // name & param type
    for (size_t i = start+3; i < endParen; i += 3) {
        // verify typename, name, and comma are present
        if (!isTokenPrimitiveType(tokens[i].type))
            throw DTSyntaxException(tokens[i].err, tokens[i].raw);
        if (tokens[i+1].type != IDENTIFIER)
            throw DTSyntaxException(tokens[i+1].err, tokens[i+1].raw);
        if (i+2 != endParen && tokens[i+2].type != COMMA)
            throw DTSyntaxException(tokens[i+2].err, tokens[i+2].raw);

        pNode->appendParam({tokens[i+1].raw, tokens[i].type}); // append param
    }

    parse(tokens, endParen+2, endBrace-1, pNode); // parse body (ignore braces)
    return pNode;
}

// for variable declarations
ASTNode* parseDeclaration(const std::vector<Token>& tokens, size_t start, size_t end) {
    ASTNode* pNode = new ASTVariable(tokens[start+1].raw, tokens[start]);
    pNode->push( parseExpresion(tokens, start+3, end) ); // parse subsequent expression w/o end semi
    return pNode;
}

// for parsing a return & its expression
ASTNode* parseReturn(const std::vector<Token>& tokens, size_t start, size_t end) {
    ASTReturn* pNode = new ASTReturn(tokens[start]);
    try {
        pNode->push(parseExpresion(tokens, start+1, end-1)); // parse subsequent expression w/o end semi
    } catch (DTException& e) {
        delete pNode;
        throw;
    }
    return pNode;
}

// master parse method, calls other specific methods based on tokens present & their semantic validity
void parse(const std::vector<Token>& tokens, size_t start, size_t end, ASTNode* pHead) {
    try {
        for (size_t i = start; i <= end; i++) {
            const Token* pNext = peek(tokens, i);
            const Token& token = tokens[i];
            switch (tokens[i].type) {
                case TokenType::TYPE_BOOL: case TokenType::TYPE_CHAR: case TokenType::TYPE_DOUBLE:
                case TokenType::TYPE_INT: case TokenType::TYPE_STR: {
                    // check for identifier after
                    const Token* pNextNext = peek(tokens, i+1);
                    if (pNext == nullptr || pNext->type != IDENTIFIER) throw DTSyntaxException(token.err, token.raw);
                    
                    // check for either function or variable declaration
                    bool isFunction = false;
                    if (pNextNext == nullptr) throw DTSyntaxException(pNext->err, pNext->raw);
                    else if (pNextNext->type == LPAREN) isFunction = true;
                    else if (pNextNext->type != ASSIGN) throw DTSyntaxException(pNext->err, pNext->raw);
                    
                    if (isFunction) {
                        // find closing parenthesis )
                        size_t parensOpen = 0, start = i;
                        i += 2; // skip type & identifier
                        do {
                            if (tokens[i].type == RPAREN) parensOpen--;
                            else if (tokens[i].type == LPAREN) parensOpen++;
                            i++;
                        } while (i <= end && parensOpen > 0);
                        size_t endParen = i-1;

                        if (parensOpen > 0)
                            throw DTUnclosedGroupException(pNext->err);
                        else if (i <= end && tokens[i].type != LBRACE)
                            throw DTSyntaxException(tokens[i].err, tokens[i].raw);

                        // find closing brace }
                        size_t bracesOpen = 0;
                        do {
                            if (tokens[i].type == RBRACE) bracesOpen--;
                            else if (tokens[i].type == LBRACE) bracesOpen++;
                            i++;
                        } while (i <= end && bracesOpen > 0);
                        size_t endBrace = i-1;

                        if (bracesOpen > 0) throw DTUnclosedGroupException(tokens[endParen+1].err);

                        // parse as function
                        pHead->push(parseFunction(tokens, start, endParen, endBrace));
                    } else {
                        // look for terminating semicolon
                        size_t start = i;
                        i += 2; // skip type & identifier
                        while (i <= end && tokens[i].type != SEMICOLON) i++;

                        if (i > end)
                            throw DTSyntaxException(tokens[i-1].err, tokens[i-1].raw);

                        // parse as variable declaration
                        pHead->push(parseDeclaration(tokens, start, i-1));
                    }
                    break;
                }
                case TokenType::RETURN: {
                    // find closing semicolon
                    size_t start = i;
                    while (i <= end && tokens[i].type != TokenType::SEMICOLON) i++;
                    if (i > end) throw DTSyntaxException(tokens[start].err, tokens[start].raw); // handle missing semicolon
                    pHead->push(parseReturn(tokens, start, i));
                    break;
                }
                default: break; // TODO: suppress compiler errors
            }
        }
    } catch (DTException& e) {
        delete pHead;
        throw;
    }
}

AST* buildAST(const std::vector<Token>& tokens) {
    ASTNode* pHead = new ASTNode({TokenType::IDENTIFIER, "", {0, 1, 0}});
    AST* ast = new AST( pHead );
    try {
        parse(tokens, 0, tokens.size()-1, pHead);
    } catch (DTException& e) {
        ast->pRoot = nullptr; // parse function already deletes pHead, so just delete the AST
        delete ast;
        throw;
    }
    return ast;
}