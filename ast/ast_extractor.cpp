#include <iostream>
#include <fstream>
#include <stack>
#include <vector>
#include <unordered_map>

#include "ast.hpp"
#include "ast_extractor.hpp"

// assign strings to a lookup table for assembling
#define ASM_STR_PREFIX "_LS" // LS for "literal string", as _LS0000 for corresponding numeric id in AST
#define ASM_STRLEN_SUFFIX "_SZ" // suffix for AST string variables' sizes (ex. string is _LS0, size is _LS0_SZ)
#define ASM_FUNC_PREFIX "_FD" // FD for "function definition"
#define TAB "    "
#define outTab outHandle << TAB

void markStrings(ASTNode* pNode, std::vector<std::string>& strsVec) {
    const size_t len = pNode->size();
    for (size_t i = 0; i < len; i++) {
        ASTNode* pChild = pNode->at(i);
        if (pChild->nodeType() == ASTNodeType::LIT_STR) {
            // generate id and extract text
            ASTStringLiteral& strLit = *static_cast<ASTStringLiteral*>(pChild);
            strsVec.push_back(strLit.val);
            strLit.assemblerID = strsVec.size();
        } else {
            // recurse all other nodes
            markStrings(pChild, strsVec);
        }
    }
}

// used to generate ASM code from an AST
void generateASM(std::ofstream& outHandle, const AST& ast) {
    outHandle << "global _start\n";

    ASTNode* pNode = nullptr;
    const size_t len = ast.pRoot->size();

    // 1. initial pass over AST (store string literals & such) in .data section
    outHandle << "section .data\n";

    // 1.A mark out each string in the AST
    std::vector<std::string> strsVec;
    markStrings(ast.pRoot, strsVec);

    // 1.B write each string to the output file
    const size_t strsLen = strsVec.size();
    std::string strBuf, nameBuf;
    for (size_t i = 0; i < strsLen; i++) {
        // update buffers
        strBuf = strsVec[i];
        nameBuf = ASM_STR_PREFIX + std::to_string(i);

        // write
        outHandle << TAB << nameBuf << ": DB '" << strBuf << "'\n"; // string
        outHandle << TAB << nameBuf + ASM_STRLEN_SUFFIX << " EQU $ - " << nameBuf << '\n'; // size
    }

    // 2. compile .text section
    outHandle << "section .text\n";

    // 2.A index all functions
    std::vector<func_pair> funcsVec;
    asmID funcIndex = 0, mainFuncIndex = -1;
    for (size_t i = 0; i < len; i++) {
        pNode = ast.pRoot->at(i);
        
        if (pNode->nodeType() == ASTNodeType::FUNCTION) {
            ASTFunction& func = *static_cast<ASTFunction*>(pNode);
            func.assemblerID = funcIndex++; // store the assembler index
            funcsVec.push_back({func.getName(), &func});
            
            // check for main function
            if (mainFuncIndex == -1 && func.getName() == "main" && func.getNumParams() == 0)
                mainFuncIndex = func.assemblerID;
        }
    }

    // 2.C parse global functions
    pNode = nullptr;

    for (size_t i = 0; i < len; i++) {
        pNode = ast.pRoot->at(i);
        const ASTNodeType type = pNode->nodeType();
        
        if (type == ASTNodeType::FUNCTION) {
            ASTFunction& func = *static_cast<ASTFunction*>(pNode);
            asmID assemblerID = func.assemblerID;

            // append label to document
            outHandle << ASM_FUNC_PREFIX << std::to_string(assemblerID) << ":\n";

            // create stack frame
            outHandle << TAB << "push rbp\n"; // save old base ptr
            outHandle << TAB << "mov rbp, rsp\n"; // set new base ptr

            // compile function code
            Register outRegister = compileFunction(outHandle, func);

            // collapse stack frame & return
            outHandle << TAB << "mov rsp, rbp\n";
            outHandle << TAB << "pop rbp\n";
            outHandle << TAB << "ret\n";
        }
    }

    // 2.B generate start entry point
    outHandle << "_start:\n" <<
          TAB << "xor rdi, rdi\n" << // default exit code (0)
          TAB << "call " << ASM_FUNC_PREFIX << std::to_string(mainFuncIndex) << '\n' << // call main
          TAB << "mov rdi, rax\n" << // move return value from main function into rdi for sys_exit
          TAB << "mov rax, 60\n" << // specify syscall # for sys_exit
          TAB << "syscall\n"; // syscall
}

// used to explicitly convert code within an AST function to assembly code
Register compileFunction(std::ofstream& outHandle, ASTFunction& func) {
    // create a map to store the offset from the stack ptr for all declared variables
    var_offset_map varOffsets;

    // iterate over all code within the function
    const size_t len = func.size();
    const TokenType returnType = func.getReturnType();

    Register outRegister; // store what register the return expression is stored in

    for (size_t i = 0; i < len; i++) {
        // switch based on type
        ASTNode* pChild = func.at(i);
        switch (pChild->nodeType()) {
            case ASTNodeType::RETURN: {
                ASTReturn& node = *static_cast<ASTReturn*>(pChild);

                // handle return values
                if (node.size() > 0) {
                    // resolve expression
                    outRegister = resolveExpression(outHandle, *static_cast<ASTExpr*>(node.at(0)), varOffsets);
                } else {
                    // no expression, return 0
                    outTab << "mov rax, 0\n"; // put 0 into output register rax
                    outRegister = Register::RAX;
                }
            }
            default: break;
        }
    }

    return outRegister;
}

// used to compile an expression into assembly code
Register resolveExpression(std::ofstream& outHandle, ASTExpr& expr,
                           var_offset_map& varOffsets, std::stack<Register>& stack) {
    // iterate depth-first
    Register outRegister = Register::RAX;

    ASTNode* pNode;
    size_t len = expr.size();
    for (size_t i = 0; i < len; i++) {
        pNode = expr.at(i);
        ASTNodeType type = pNode->nodeType();

        switch (type) {
            case ASTNodeType::BIN_EXPR: {
                ASTBinExpr& binExpr = *static_cast<ASTBinExpr*>(pNode);

                // traverse downward
                if (!binExpr.left()->isAssembled) {
                    // traverse left
                    Register outL = resolveExpression(outHandle, binExpr, varOffsets, stack);

                    // push result register to stack
                    bool isLeftWide = isRegisterWide(outL);
                    if (isLeftWide) { // wide registers (128 bits in this case)
                        outTab << "sub rsp, 16"; // move stack ptr back 16 bytes
                        outTab << "movdqu [rsp], " << getRegisterStr(outL) << '\n'; // "push" register to stack
                    } else { // all other registers <= 64 bits
                        outTab << "push " << getRegisterStr(outL) << '\n';
                    }

                    // traverse right
                    Register outR = resolveExpression(outHandle, binExpr, varOffsets, stack);

                    // if either of the two output registers is wide, we MUST use wide registers now
                    bool isRightWide = isRegisterWide(outR);

                    // pop the argument into the proper register
                    if (isLeftWide || isRightWide) { // use XMM0 & XMM1
                        // handle right output
                        if (outR != Register::XMM1) {
                            
                            outTab << "movsd xmm1, " << getRegisterStr(outR) << '\n'; // move right into XMM1
                        }
                        // if the left output is wide, move into XMM0
                    } else { // use RAX & RBX
                        if (outR != Register::RBX) // move the right node's output into RBX if not there
                            outTab << "mov " << getRegisterStr(outR) << ", rbx\n";
                        outTab << "pop rax\n"; // pop stack into RAX
                    }
                }
                break;
            }
        }
    }

    return outRegister;
}