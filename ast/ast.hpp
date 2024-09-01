#ifndef __AST_HPP
#define __AST_HPP

#include "ast_nodes.hpp"

class AST {
    public:
        AST(ASTNode* pRoot) : pRoot(pRoot) {};
        ~AST() {  delete pRoot;  }
        ASTNode* pRoot = nullptr;
};

#endif