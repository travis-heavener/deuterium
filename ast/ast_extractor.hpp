#ifndef __AST_EXTRACTOR_HPP
#define __AST_EXTRACTOR_HPP

#include <fstream>
#include <stack>
#include <unordered_map>

#include "ast.hpp"
#include "ast_nodes.hpp"

typedef long long asmID;
typedef std::pair<std::string, ASTNode*> func_pair;
typedef std::unordered_map<std::string, unsigned long> var_offset_map;

// used to generate ASM code from an AST
void generateASM(std::ofstream&, const AST&);

// used to explicitly convert code within an AST function to assembly code
Register compileFunction(std::ofstream&, ASTFunction&);

// used to compile an expression into assembly code
Register resolveExpression(std::ofstream&, ASTExpr&, var_offset_map&, std::stack<Register>&);

#endif