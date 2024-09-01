#ifndef __PARSER_HPP
#define __PARSER_HPP

#include <vector>

#include "lexer.hpp" // for Tokens & types
#include "ast/ast.hpp"
#include "ast/ast_nodes.hpp"

// external methods
AST* buildAST(const std::vector<Token>&);

// internal use methods
void parse(const std::vector<Token>&, size_t, size_t, ASTNode*);

#endif