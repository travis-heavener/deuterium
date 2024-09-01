#include <iostream>
#include <vector>

#include "exp_parser.hpp"
#include "lexer.hpp" // for Tokens & types
#include "ast/ast.hpp"
#include "ast/ast_nodes.hpp"
#include "errors.hpp"

// for parsing an expression
// (bottom)     -->     -->     -->     -->     -->     -->     -->     (top)
// PARENTHESIS, UNARIES, MULT/DIV/MOD, ADD/SUB & INC/DEC, COMPARISON, ASSIGNMENT
/**
 * 1. PARSE ALL TOKENS (W/ PARENTHESIS RECURSIVELY) SO NODE IS FLAT EXCEPT FOR PARENTHETICALS
 * 2. COMBINE UNARIES
 * 3. COMBINE MULT/DIV/MOD ARGS ON EITHER SIDE
 * 4. ADD/SUB & INC/DEC
 * 5. COMPARISON
 * 6. ASSIGNMENT
 * 7. ASTEXPR SHOULD NOW BE FORMATTED HIERARCHICALLY
*/
ASTNode* parseExpresion(const std::vector<Token>& tokens, size_t start, size_t end) {
    ASTExpr* pNode = new ASTExpr(tokens[start]);

    // initially parse tokens as flat vector
    try {
        for (size_t i = start; i <= end; i++) {
            // if opening a parenthesis group, find closing parenthesis & recurse-parse it
            if (tokens[i].type == LPAREN) {
                size_t start = i, parensOpen = 1;
                while (++i <= end && parensOpen > 0) {
                    if (tokens[i].type == RPAREN) parensOpen--;
                    else if (tokens[i].type == LPAREN) parensOpen++;
                }
                if (i-- > end) throw DTUnclosedGroupException(tokens[start].err);
                pNode->push( parseExpresion(tokens, start+1, i-1) );
            } else if (isTokenLiteral(tokens[i].type)) { // push literal
                switch (tokens[i].type) {
                    case TokenType::LIT_BOOL:
                        pNode->push( new ASTBoolLiteral(tokens[i].raw == "true", tokens[i]) );
                        break;
                    case TokenType::LIT_CHAR:
                        pNode->push( new ASTCharLiteral(tokens[i].raw[0], tokens[i]) );
                        break;
                    case TokenType::LIT_DOUBLE:
                        pNode->push( new ASTDoubleLiteral(std::stod(tokens[i].raw), tokens[i]) );
                        break;
                    case TokenType::LIT_INT:
                        pNode->push( new ASTIntLiteral(std::stoi(tokens[i].raw), tokens[i]) );
                        break;
                    case TokenType::LIT_STR:
                        pNode->push( new ASTStringLiteral(tokens[i].raw, tokens[i]) );
                        break;
                    case TokenType::LIT_NULL:
                        pNode->push( new ASTNullLiteral(tokens[i]) );
                        break;
                    default: break; // suppress g++ warnings
                }
            } else if (isTokenUnaryOp(tokens[i].type)) { // push unary
                pNode->push( new ASTUnaryExpr(tokens[i]) );
            } else if (isTokenBinaryOp(tokens[i].type)) { // push binary
                pNode->push( new ASTBinExpr(tokens[i]) );
            } else if (tokens[i].type == TokenType::IDENTIFIER) {
                pNode->push( new ASTIdentifier(tokens[i]) );
            } else {
                throw DTSyntaxException(tokens[i].err, tokens[i].raw);
            }
        }

        // hierarchically rearrange expression

        // COMBINE UNARIES
        for (size_t i = 0; i < pNode->size(); i++) {
            ASTNode* pCurrent = pNode->at(i);
            if (pCurrent->nodeType() == ASTNodeType::UNARY_EXPR) {
                // verify if this is a math operation (+ or - sign), it's valid
                ASTUnaryExpr* pCurrentExpr = static_cast<ASTUnaryExpr*>(pCurrent);
                TokenType opType = pCurrentExpr->opType();
                if ((opType == TokenType::OP_ADD || opType == TokenType::OP_SUB) &&
                    !(i == 0 || pNode->at(i-1)->nodeType() == ASTNodeType::UNARY_EXPR || pNode->at(i-1)->nodeType() == ASTNodeType::BIN_EXPR)) {
                    continue;
                }
                
                // ignore inc/decrements
                if (opType == TokenType::OP_INC || opType == TokenType::OP_DEC) continue;

                // unary child is after the unary
                // check for following expression
                if (i+1 == pNode->size()) throw DTSyntaxException(pCurrent->err, pCurrent->raw);

                // append next node as child of unary
                pCurrent->push(pNode->at(i+1));
                pNode->removeChild(i+1);
            }
        }

        // COMBINE MULT/DIV/MOD ARGS ON EITHER SIDE
        for (size_t i = 0; i < pNode->size(); i++) {
            ASTNode* pCurrent = pNode->at(i);
            if (pCurrent->nodeType() == ASTNodeType::BIN_EXPR) {
                // verify this is mult/div/mod math operation
                ASTBinExpr* pCurrentExpr = static_cast<ASTBinExpr*>(pCurrent);
                TokenType opType = pCurrentExpr->opType();
                if (opType != TokenType::OP_MUL && opType != TokenType::OP_DIV && opType != TokenType::OP_MOD)
                    continue;

                // check for following expression
                if (i == 0 || i+1 == pNode->size()) throw DTSyntaxException(pCurrent->err, pCurrent->raw);
                
                // append previous and next nodes as children of bin expr
                pCurrent->push(pNode->at(i-1));
                pCurrent->push(pNode->at(i+1));
                pNode->removeChild(i+1); // remove last, first to prevent adjusting indexing
                pNode->removeChild(i-1);
                i--; // skip back once since removing previous node
            }
        }

        // ADD/SUB & INC/DEC
        for (size_t i = 0; i < pNode->size(); i++) {
            ASTNode* pCurrent = pNode->at(i);
            if (pCurrent->nodeType() == ASTNodeType::UNARY_EXPR) {
                // verify this is inc or dec
                ASTUnaryExpr* pCurrentExpr = static_cast<ASTUnaryExpr*>(pCurrent);
                TokenType opType = pCurrentExpr->opType();
                if (opType != TokenType::OP_INC && opType != TokenType::OP_DEC) continue;

                // check if identifiers are before/after operator
                // first takes precedence (ex. i++i -> i++ and i)
                bool isIdenBefore = i > 0 && pNode->at(i-1)->nodeType() == ASTNodeType::IDENTIFIER;
                bool isIdenAfter = i+1 < pNode->size() && pNode->at(i+1)->nodeType() == ASTNodeType::IDENTIFIER;

                if (isIdenBefore) { // append last node as child of unary
                    pCurrent->push(pNode->at(i-1));
                    pNode->removeChild(i-1);
                    i--; // fix increment offset
                } else if (isIdenAfter) { // append next node as child of unary
                    pCurrent->push(pNode->at(i+1));
                    pNode->removeChild(i+1);
                    pCurrentExpr->setIsPostOperator(true);
                } else {
                    throw DTSyntaxException(pCurrent->err, pCurrent->raw);
                }
            } else if (pCurrent->nodeType() == ASTNodeType::BIN_EXPR) {
                // verify this is add/sub math operation
                ASTBinExpr* pCurrentExpr = static_cast<ASTBinExpr*>(pCurrent);
                TokenType opType = pCurrentExpr->opType();
                if (opType != TokenType::OP_ADD && opType != TokenType::OP_SUB) continue;

                // check for following expression
                if (i == 0 || i+1 == pNode->size()) throw DTSyntaxException(pCurrent->err, pCurrent->raw);
                
                // append previous and next nodes as children of bin expr
                pCurrent->push(pNode->at(i-1));
                pCurrent->push(pNode->at(i+1));
                pNode->removeChild(i+1); // remove last, first to prevent adjusting indexing
                pNode->removeChild(i-1);
                i--; // skip back once since removing previous node
            }
        }

        // COMPARISON OPERATORS
        for (size_t i = 0; i < pNode->size(); i++) {
            ASTNode* pCurrent = pNode->at(i);
            if (pCurrent->nodeType() == ASTNodeType::BIN_EXPR) {
                // verify this is a comparison operation
                ASTBinExpr* pCurrentExpr = static_cast<ASTBinExpr*>(pCurrent);
                if (!isTokenCompOp(pCurrentExpr->opType())) continue;

                // check for following expression
                if (i == 0 || i+1 == pNode->size()) throw DTSyntaxException(pCurrent->err, pCurrent->raw);
                
                // append previous and next nodes as children of bin expr
                pCurrent->push(pNode->at(i-1));
                pCurrent->push(pNode->at(i+1));
                pNode->removeChild(i+1); // remove last, first to prevent adjusting indexing
                pNode->removeChild(i-1);
                i--; // skip back once since removing previous node
            }
        }

        // ASSIGNMENTS
        for (size_t i = 0; i < pNode->size(); i++) {
            ASTNode* pCurrent = pNode->at(i);
            if (pCurrent->nodeType() == ASTNodeType::BIN_EXPR) {
                // verify this is an assignment expression
                ASTBinExpr* pCurrentExpr = static_cast<ASTBinExpr*>(pCurrent);
                if (!isTokenAssignOp(pCurrentExpr->opType())) continue;

                // check for following expression
                if (i == 0 || i+1 == pNode->size()) throw DTSyntaxException(pCurrent->err, pCurrent->raw);
                
                // append previous and next nodes as children of bin expr
                pCurrent->push(pNode->at(i-1));
                pCurrent->push(pNode->at(i+1));
                pNode->removeChild(i+1); // remove last, first to prevent adjusting indexing
                pNode->removeChild(i-1);
                i--; // skip back once since removing previous node
            }
        }

        // ASTEXPR SHOULD NOW BE FORMATTED HIERARCHICALLY
    } catch (DTException& e) {
        delete pNode;
        throw;
    }

    return pNode;
}