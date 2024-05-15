#include <string>

#include "toolbox.hpp"

// list of escape chars (in JS)
// https://en.wikipedia.org/wiki/Escape_character#:~:text=%5Bedit%5D-,JavaScript,-%5Bedit%5D
char escapeChar(const std::string& str) {
    switch (str[1]) {
        case '\'': return '\'';
        case '"': return '"';
        case '\\': return '\\';
        case 'n': return '\n';
        case 'r': return '\r';
        case 't': return '\t';
        case 'b': return '\b';
        case 'f': return '\f';
        case 'v': return '\v';
        case '0':
        default: return '\0';
    }
}