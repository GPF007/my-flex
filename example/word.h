
#include <string>
#include <memory>
enum TokenType{
	LBRACE,
	BBRACE,
	INT,
	STR,
	IDENT,
	WS,
	COMMENT,
};

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
