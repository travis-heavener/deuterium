#ifndef __EXP_PARSER_HPP
#define __EXP_PARSER_HPP

#include <vector>

#include "lexer.hpp" // for Tokens & types
#include "ast/ast.hpp"
#include "ast/ast_nodes.hpp"

// this single method deserves its own file because it's so damn complicated
ASTNode* parseExpresion(const std::vector<Token>&, size_t, size_t);

#endif