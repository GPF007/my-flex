//
// Created by gpf on 2020/10/9.
//

#ifndef MYLEXER_STRS_H
#define MYLEXER_STRS_H

#include <string>


std::string YYSCAN = R"(
TokenPtr YYLexer::Scan() {
    int ch=0;
    p = q;
    lastAcceptState_ = -1;
    curState_ = 0;
    int line = line_, column = column_;
    TokenPtr r = nullptr;
    for (ch = getch(); ch >=0 ; ch=getch()) {
        lastAcceptState_ = isAccept(curState_)?curState_:lastAcceptState_;
        auto charClassIdx = alphabet_[ch];
        charClassIdx = charClassIdx<0?0:charClassIdx;
        auto nextState = table_[curState_][charClassIdx];
        nextState = nextState<0?table_[curState_][0]:nextState;
        //printf("charidx = %d, curstate= %d, nextstate=%d\n",charClassIdx,curState_, nextState);

        if(nextState<0) break;
        curState_ = nextState;
    }
    ungetch();


    if(lastAcceptState_ < 0){
        if(ch<0)
            return std::make_unique<Token>(0,line, column, "EOF");
        else{
            fprintf(stderr,"%s %d:%d '%c' is not recognized by lexer!\n",filename_.data(),
                    line_,column_, *q);
            exit(EXIT_FAILURE);
        }
    }
    switch (lastAcceptState_) {
)";



std::string HEADER_INCLUDE = R"(
#include <string>
#include <memory>
)";

std::string HEADER_CONTENT = R"(
class Token {
public:
    Token(int t, int l, int c, const char* p, const char* q): type(t),line(l),column(c){
        sval = std::move(std::string(p,q-p));
    }
    Token(int t, int l, int c, const char* p): type(t),line(l),column(c){
        sval = std::move(std::string(p));
    }
    std::string toString();

    int type;
    int line;
    int column;
    std::string sval;
};
typedef std::unique_ptr<Token> TokenPtr;
class AbstractLexer{
public:
    virtual TokenPtr Scan()=0;
    virtual TokenPtr Scan2()=0;
    virtual ~AbstractLexer(){}
};

AbstractLexer* createLexer(const char* fname);
)";

std::string SOURCE_INCLUDE = R"(
#include <sstream>
#include <vector>
#include <iostream>
#include <set>
#include <map>
#include <fstream>
)";

std::string SOURCE_CLASS = R"(
class YYLexer: public AbstractLexer {
public:
    YYLexer(const char* fname);
    TokenPtr Scan() override;
private:
    int line_;
    int column_;
    int lastLine_;
    int lastColumn_;
    std::string filename_;
    std::string buffer_;
    std::vector<std::vector<int>> table_;
    std::map<int,int> stateToRule_;
    std::array<int,256> alphabet_;
    std::set<int> acceptStates_;

    //for loop
    int curState_;
    int lastAcceptState_;
    char* data_;
    char* end_;
    char* p;
    char* q;


    //methods
    int getch(){
        if(*q==*end_){
            q++;
            return -1;//eof
        }
        lastLine_ = line_;
        lastColumn_ = column_;
        if(*q=='\n'){
            line_++;
            column_ = 0;
        }else{
            column_++;
        }
        return *q++;
    }

    void ungetch(){
        --q;
        line_ = lastLine_;
        column_ = lastColumn_;
    }

    bool isAccept(int stateIdx){
        return acceptStates_.find(stateIdx) != acceptStates_.end();
    }

    TokenPtr Scan2() override{
        TokenPtr tok = nullptr;
        while(!(tok = Scan()));//until not empty
        return tok;
    }

};
)";

std::string SOURCE_CONSTRUCTOR_PREV= R"(
//open file and read ot buffer
YYLexer::YYLexer(const char *fname):line_(0),column_(0), lastLine_(0),lastColumn_(0),filename_(fname){
    std::ifstream in(fname);
    if(!in){
        fprintf(stderr, "open file:%s failed\n",fname);
        exit(EXIT_FAILURE);
    }
    std::stringstream tmp;
    tmp << in.rdbuf();
    buffer_ = std::move(tmp.str());
    //a little trick appen a space to buffer to avoid eof error
    //buffer_.push_back(' ');
    data_ = const_cast<char*>(buffer_.c_str());
    end_ = data_;
    while(*end_!='\0'){
        *end_++;
    }
    p=data_;
    q=data_;
    lastAcceptState_=-1;
    curState_ = 0;

    //initial table

)";

std::string TOKEN_STR_FUNC=R"(
std::string Token::toString() {
    std::stringstream str;
    str<<"{line: "<< line<<", column: "<<column<<", type: "<<type<<
    ", val: "<<sval<<"}";
    return str.str();
}
)";

std::string CREATE_LEXER_FUNC=R"(
AbstractLexer* createLexer(const char* fname){
    return new YYLexer(fname);
}
)";

#endif //MYLEXER_STRS_H
