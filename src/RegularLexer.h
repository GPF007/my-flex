//
// Created by gpf on 2020/10/7.
//

#ifndef MYLEXER_REGULARLEXER_H
#define MYLEXER_REGULARLEXER_H

#include <map>
#include "charClassSet.h"
#include "node.h"
#include "nfa.h"
#include "dfa.h"

class RegularLexer {
    friend class DFA;
    friend class NFA;

public:
    RegularLexer(const char* fname);
    void Parse();
    void BuildNFA();
    void BuildDFA();
    std::array<int,256>* Alphabet() {return alphabet_.get();}

    //for debug method
    void showRules(FILE* f);
    void showNFA(FILE* f);
    void showDFA(FILE* f);

    //generation code
    void GenHeader();
    void GenSource();
    void GenTable(FILE* f);
    void GenAlphabet(FILE* f);
    void GenAccepts(FILE* f);
    void GenScanFunction(FILE* f);
    void GenClass(FILE* f);
    void GenConstructor(FILE* f);

private:
    class Rule {
    public:
        Rule(std::string r, std::string id,bool skp= false):regex(r),ident(id),isSkipped(skp){}
        std::string regex;
        std::string ident;
        bool isSkipped;
        int idx;
        std::vector<int> acceptStates_;
        std::unique_ptr<NFA> nfa;//每一个rule都有一个nfa
    };
    std::string fname_;
    std::string fheader_;
    std::string fsource_;
    int ruleCount_=0;
    std::string buffer_;
    int idx_=0;
    int len_;

    std::string include_;
    std::string code_;

    typedef std::unique_ptr<Rule> RulePtr;
    typedef std::array<int,256> CharArray;
    std::vector<RulePtr> rules_;//rule的集合
    std::vector<Node*> nodes_; //nfa nodes
    std::unique_ptr<CharClassSet> charClassSet;
    std::unique_ptr<CharArray> alphabet_;
    std::map<int,int> nfaAcceptStates_; //node index -> rule index
    std::set<int> dfaAcceptStates_;
    //用来存储nfa的node and edge
    std::unique_ptr<Arena> arena_;

    //nfa start
    Node* nfaStart_;
    std::unique_ptr<NFA> nfa_;
    std::unique_ptr<DFA> dfa_;

    //private methods
    void parseRules();


    //getch method to parse .l file
    int readc() {return buffer_[idx_++];}
    std::string readIdent();
    void unreadc()  {--idx_;}
    int peek()  {return buffer_[idx_];}
    bool next(int expect) {return peek()==expect? true: false;}
    void expect(int ch);
    void skipLine();
    void skipBlockComment();
    bool doSkipSpace();
    bool skipSpace();
    static bool isWhiteSpace(int c) {return c == ' ' || c == '\t' || c == '\f' || c == '\v' || c == '\n';}

    //parse method
    void parseInclude();
    void parseCode();
    void parseRawCode(std::string& s);
    RulePtr parseRule();



};


#endif //MYLEXER_REGULARLEXER_H
