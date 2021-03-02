//
// Created by gpf on 2020/10/9.
//

#include "token.h"

#include <sstream>

std::string Token::toString() {
    std::stringstream str;
    str<<"{line: "<< line<<", column: "<<column<<", type: "<<type<<
    ", val: "<<sval<<"}";
    return str.str();
}