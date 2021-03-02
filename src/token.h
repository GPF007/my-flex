//
// Created by gpf on 2020/10/9.
//

#ifndef MYLEXER_TOKEN_H
#define MYLEXER_TOKEN_H

#include <string>

class Token {
public:
    Token(int t, int l, int c, const char* p, const char* q): type(t),line(line),column(c){
        sval = std::move(std::string(p,q-p));
    }

    std::string toString();

    int type;
    int line;
    int column;
    std::string sval;
};


#endif //MYLEXER_TOKEN_H
