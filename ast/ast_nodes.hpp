#ifndef __AST_NODES_HPP
#define __AST_NODES_HPP

#include <string>
#include <vector>

#include "../lexer.hpp"

enum class ASTNodeType {
    NODE, // base class
    FUNCTION, VARIABLE, IDENTIFIER, RETURN,
    EXPR, UNARY_EXPR, BIN_EXPR,
    LIT_BOOL, LIT_CHAR, LIT_DOUBLE, LIT_INT, LIT_STR, LIT_NULL
};

enum class Register {
    RAX, RBX, RCX, RDX, RDI, XMM0, XMM1
};

const std::string getRegisterStr(Register reg) {
    switch (reg) {
        case Register::RAX: return "rax";
        case Register::RBX: return "rbx";
        case Register::RCX: return "rcx";
        case Register::RDX: return "rdx";
        case Register::RDI: return "rdi";
        case Register::XMM0: return "xmm0";
        case Register::XMM1: return "xmm1";
    }
    return "";
}

// true for registers > 64 bit; false otherwise
constexpr bool isRegisterWide(Register reg) {
    return reg == Register::XMM0 || reg == Register::XMM1;
}

// base class for all AST node types
class ASTNode {
    public:
        ASTNode(const Token& token) : err(token.err), raw(token.raw) {};
        virtual ~ASTNode();
        void push(ASTNode* pNode) { children.push_back(pNode); };
        ASTNode* removeChild(size_t i);
        ASTNode* pop() { ASTNode* pNode = lastChild(); children.pop_back(); return pNode; };
        
        virtual ASTNodeType nodeType() const { return ASTNodeType::NODE; };

        ASTNode* at(unsigned int i) const { return children[i]; };
        size_t size() const { return children.size(); };
        ASTNode* lastChild() const { return this->size() > 0 ? children[this->size()-1] : nullptr; };

        // assembler helpers
        bool isAssembled = false; // whether or not the node has been resolved

        // for error reporting
        ErrInfo err;
        std::string raw;
    protected:
        std::vector<ASTNode*> children;
};

class ASTReturn : public ASTNode {
    public:
        ASTReturn(const Token& token) : ASTNode(token) {};
        ASTNodeType nodeType() const { return ASTNodeType::RETURN; };
};

/************* EXPRESSIONS *************/

class ASTExpr : public ASTNode {
    public:
        ASTExpr(const Token& token) : ASTNode(token) {};
        ASTNodeType nodeType() const { return ASTNodeType::EXPR; };
};

class ASTUnaryExpr : public ASTExpr {
    public:
        ASTUnaryExpr(const Token& token) : ASTExpr(token), _opType(token.type) {};
        ASTNodeType nodeType() const { return ASTNodeType::UNARY_EXPR; };
        
        ASTNode* right() { return children[0]; };
        TokenType opType() { return _opType; };

        // if this unary is a post operator (comes after the literal/identifier; ex. post-inc/decrement)
        bool isPostOperator() const { return isPostOp; };
        void setIsPostOperator(bool isPostOperator) { this->isPostOp = isPostOperator; };

        Register resultRegister; // the register that the resulting value is stored in
    private:
        TokenType _opType;
        bool isPostOp = false;
};

class ASTBinExpr : public ASTExpr {
    public:
        ASTBinExpr(const Token& token) : ASTExpr(token), _opType(token.type) {};
        ASTNodeType nodeType() const { return ASTNodeType::BIN_EXPR; };

        ASTNode* left() { return children[0]; };
        ASTNode* right() { return children[1]; };
        TokenType opType() { return _opType; };

        Register resultRegister; // the register that the resulting value is stored in
    private:
        TokenType _opType;
};

/************* LITERALS & IDENTIFIERS *************/

typedef std::pair<std::string, TokenType> param_t;
class ASTFunction : public ASTNode {
    public:
        ASTFunction(const std::string& name, const Token& token) : ASTNode(token), name(name), type(token.type) {};
        ASTNodeType nodeType() const { return ASTNodeType::FUNCTION; };
        void appendParam(const param_t p) {  params.push_back(p);  };
        size_t assemblerID; // # id

        const std::string& getName() const { return name; };
        TokenType getReturnType() const { return type; };
        std::vector<param_t> getParams() const { return params; };
        size_t getNumParams() const { return params.size(); };
    private:
        std::string name; // name of function
        TokenType type; // return type
        std::vector<param_t> params; // parameters {name, type}
};

class ASTVariable : public ASTNode {
    public:
        ASTVariable(const std::string& name, const Token& token) : ASTNode(token), name(name), type(token.type) {};
        ASTNodeType nodeType() const { return ASTNodeType::VARIABLE; };
    private:
        std::string name; // name of variable
        TokenType type; // type of variable
};

class ASTIdentifier : public ASTNode {
    public:
        ASTIdentifier(const Token& token) : ASTNode(token), name(token.raw) {};
        ASTNodeType nodeType() const { return ASTNodeType::IDENTIFIER; };
    private:
        std::string name; // name of identifier to be resolved
};

class ASTBoolLiteral : public ASTNode {
    public:
        ASTBoolLiteral(bool val, const Token& token) : ASTNode(token), val(val) {};
        ASTNodeType nodeType() const { return ASTNodeType::LIT_BOOL; };
        bool val;
};

class ASTCharLiteral : public ASTNode {
    public:
        ASTCharLiteral(char val, const Token& token) : ASTNode(token), val(val) {};
        ASTNodeType nodeType() const { return ASTNodeType::LIT_CHAR; };
        char val;
};

class ASTDoubleLiteral : public ASTNode {
    public:
        ASTDoubleLiteral(double val, const Token& token) : ASTNode(token), val(val) {};
        ASTNodeType nodeType() const { return ASTNodeType::LIT_DOUBLE; };
        double val;
};

class ASTIntLiteral : public ASTNode {
    public:
        ASTIntLiteral(int val, const Token& token) : ASTNode(token), val(val) {};
        ASTNodeType nodeType() const { return ASTNodeType::LIT_INT; };
        int val;
};

class ASTStringLiteral : public ASTNode {
    public:
        ASTStringLiteral(const std::string& val, const Token& token) : ASTNode(token), val(val) {};
        ASTNodeType nodeType() const { return ASTNodeType::LIT_STR; };
        std::string val;
        size_t assemblerID; // # id in .data section for this string
};

class ASTNullLiteral : public ASTNode {
    public:
        ASTNullLiteral(const Token& token) : ASTNode(token) {};
        ASTNodeType nodeType() const { return ASTNodeType::LIT_NULL; };
};

#endif