#ifndef __ERRORS_HPP
#define __ERRORS_HPP

#include <stdexcept>
#include <string>
#include <vector>

#include "lexer.hpp"
#include "ast/ast_nodes.hpp"

// base class, ideally shouldn't be called but can
class DTException : public std::runtime_error {
    public:
        // keep a global list of all files so that their corresponding index
        // can be stored in tokens to save memory
        static std::vector<std::string> filesIndex;
        static int registerFile(const std::string& fileName) {
            filesIndex.push_back(fileName);
            return (int)filesIndex.size()-1;
        }

        // as individual args
        DTException(c_trace lineNum, c_trace colNum, const int fileIndex, const std::string& type) : std::runtime_error(type) {
            this->msg = genMsg(lineNum, colNum, fileIndex, type);
        };
        DTException(c_trace lineNum, c_trace colNum, const int fileIndex, const std::string& type, const std::string& msg)
            : std::runtime_error(type) {
            this->msg = genMsg(lineNum, colNum, fileIndex, type, msg);
        };

        // as Tokens
        DTException(const ErrInfo& err, const std::string& type) : std::runtime_error(type) {
            this->msg = genMsg(err.line, err.col, err.fileIndex, type);
        };
        DTException(const ErrInfo& err, const std::string& type, const std::string& msg) : std::runtime_error(type) {
            this->msg = genMsg(err.line, err.col, err.fileIndex, type, msg);
        };

        const char* what() { return msg.c_str(); }
    private:
        static std::string genMsg(c_trace line, c_trace col, const int fileIndex, const std::string& type, const std::string& msg="") {
            return type + "Exception at " + filesIndex[fileIndex] + ':'
                   + std::to_string(line) + ':' + std::to_string(col)
                   + (msg.size() > 0 ? ('\n' + msg) : "");
        }
        std::string msg; // err msg
};

// specific exceptions
class DTSyntaxException : public DTException {
    public:
        DTSyntaxException(const ErrInfo& err, const std::string& raw)
            : DTException(err, "Syntax", "Near: " + raw) {};
};

class DTUnclosedGroupException : public DTException {
    public:
        DTUnclosedGroupException(const ErrInfo& err) : DTException(err, "Syntax") {};
};

#endif