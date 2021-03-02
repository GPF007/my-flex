
#include <sstream>
#include <vector>
#include <iostream>
#include <set>
#include <map>
#include <fstream>
#include "word.h"

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
	table_={
		{-1, 1, 2, 3, 4, -1, 5, -1, 6, 7, -1, -1		},
		{-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1		},
		{-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1		},
		{-1, -1, -1, 3, -1, -1, -1, -1, -1, -1, -1, -1		},
		{13, -1, -1, -1, 14, 15, -1, -1, -1, -1, -1, -1		},
		{-1, -1, -1, 12, -1, -1, 12, 12, -1, -1, -1, -1		},
		{-1, -1, -1, -1, -1, -1, -1, -1, 6, -1, -1, -1		},
		{-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 8, -1		},
		{9, -1, -1, -1, -1, -1, -1, -1, -1, -1, 10, -1		},
		{9, -1, -1, -1, -1, -1, -1, -1, -1, -1, 10, -1		},
		{-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 11		},
		{-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1		},
		{-1, -1, -1, 12, -1, -1, 12, 12, -1, -1, -1, -1		},
		{13, -1, -1, -1, 14, 15, -1, -1, -1, -1, -1, -1		},
		{-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1		},
		{-1, -1, -1, -1, 16, -1, -1, -1, -1, -1, -1, -1		},
		{13, -1, -1, -1, 14, 15, -1, -1, -1, -1, -1, -1		},
	};
	alphabet_={
		-1,-1,-1,-1,-1,-1,-1,-1,-1,8,8,-1,-1,-1,-1,
		-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
		-1,-1,8,-1,4,-1,-1,-1,-1,-1,9,11,-1,10,-1,
		-1,-1,-1,3,3,3,3,3,3,3,3,3,3,-1,-1,
		-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
		-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
		-1,-1,5,-1,-1,7,-1,6,6,6,6,6,6,6,6,
		6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
		6,6,6,1,-1,2,-1,-1,-1,-1,-1,-1,-1,-1,-1,
		-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
		-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
		-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
		-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
		-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
		-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
		-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
		-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
		-1,	};
	acceptStates_={1,2,3,5,6,11,12,14,	};
}

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
		case 1:
		{
			r = std::move(std::make_unique<Token>(LBRACE,line, column, p,q)); break;
		}
		case 2:
		{
			r = std::move(std::make_unique<Token>(BBRACE,line, column, p,q)); break;
		}
		case 3:
		{
			r = std::move(std::make_unique<Token>(INT,line, column, p,q)); break;
		}
		case 14:
		{
			r = std::move(std::make_unique<Token>(STR,line, column, p,q)); break;
		}
		case 5:
		case 12:
		{
			r = std::move(std::make_unique<Token>(IDENT,line, column, p,q)); break;
		}
		case 6:
		{
			 r = nullptr; break;
		}
		case 11:
		{
			 r = nullptr; break;
		}
	}
	return r;
}

std::string Token::toString() {
    std::stringstream str;
    str<<"{line: "<< line<<", column: "<<column<<", type: "<<type<<
    ", val: "<<sval<<"}";
    return str.str();
}


AbstractLexer* createLexer(const char* fname){
    return new YYLexer(fname);
}


int main(int argc, char * argv[])
{
     auto lexer = createLexer(argv[1]);
        for(int i=0;i<100;i++){

            std::cout<<"i="<<i<<std::endl;
            auto tok =  lexer->Scan2();
            std::cout<<tok->toString()<<std::endl;
            if(tok->sval == "EOF")
                break;
            //std::cout<<tok->toString()<<std::endl;
        }
     delete lexer;
}


