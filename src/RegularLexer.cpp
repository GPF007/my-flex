//
// Created by gpf on 2020/10/7.
//

#include "RegularLexer.h"
#include "strs.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <utility>
using namespace std;

#define panic(str) {fprintf(stderr,"%s\n",str);exit(1);}
#define panic_if(expr,str) if(expr) panic(str)

RegularLexer::RegularLexer(const char *fname) {
    std::ifstream in;
    fname_ = fname;
    auto tmpname = fname_;

    tmpname.pop_back();
    tmpname.pop_back();
    fheader_ = tmpname+".h";
    fsource_ = tmpname+".cpp";
    in.open(fname);
    if(!in){
        cout<<"open file: "<<fname<<" failed"<<endl;
    }
    std::stringstream tmp;
    tmp << in.rdbuf();
    buffer_ = std::move(tmp.str());
    len_ = buffer_.size();

    //std::cout<<buffer_<<std::endl;
    //初始化一些通用的变量
    arena_ = std::make_unique<Arena>();
    charClassSet = std::make_unique<CharClassSet>();
    alphabet_= std::make_unique<CharArray>();
    alphabet_->fill(-1);

    //初始化第一个node
    nfaStart_ = arena_->newNode(0);
    nodes_.push_back(nfaStart_);
}


void RegularLexer::Parse() {
    parseInclude();
    //parse rules
    parseRules();
    parseCode();

}


RegularLexer::RulePtr RegularLexer::parseRule() {
    skipSpace();
    //printf("current ch is %c\n",buffer_[idx_]);
    expect('#');
    auto ruleKind = readIdent();
    if(ruleKind == "endrules") return nullptr;
    skipSpace();
    //read re
    int delim = readc();
    panic_if(delim != '{',"expected '{' before regular expression");
    int ch;
    string regex;
    for(;;){
        ch = readc();
        //遇到delim且前面不是转义字符
        if(ch=='}' && buffer_[idx_-2]!='\\'){
            break;
        }
        regex.push_back(ch);
    }
    if(regex.empty()){ //此时是最后的规则
        return nullptr;
    }
    //此时ch=delim，读取返回的ident
    skipSpace();
    expect('=');
    expect('>');
    skipSpace();
    auto typeIdent = readIdent();
    //printf("typeident: %s\n",typeIdent.data());
    auto isSkp = ruleKind=="skip"?true: false;
    return std::make_unique<Rule>(regex, typeIdent,isSkp);

}


void RegularLexer::parseRules() {
    for(;;){
        auto rule = parseRule();
        if(!rule) break;
        //将rule加入vector
        rule->idx = ruleCount_++;
        rules_.push_back(std::move(rule));
    }
}
//std::string& r, int idx,Arena* a, CharClassSet* cs,std::array<int,256>* arr, std::vector<Node*>* ns
void RegularLexer::BuildNFA() {
    for(auto &rule: rules_){
        //首先创建nfa
        rule->nfa = std::move(std::make_unique<NFA>(
                rule->regex,
                this
                ));
        rule->nfa->BuildNFA();
        nfaAcceptStates_[rule->nfa->EndIndex()] = rule->idx;
    }
    //然后用| 合并所有的node
    //在总的start 和各个规则nfa的startnode 建立一条null 边
    for(auto &rule: rules_){
        nfaStart_->AddEdge(arena_->newEdge(rule->nfa->Start()));
    }
    //new nfa
    std::string tmp_str = "";
    nfa_ = std::move(std::make_unique<NFA>(
            tmp_str,
            this
    ));
    nfa_->SetStart(nfaStart_);

}

void RegularLexer::showNFA(FILE *f) {
    charClassSet->WriteToFile(f);
    fprintf(f,"--------------------------\n");
    fprintf(f,"--------------------------\n");
    fprintf(f,"NFA counts: %ld\n",rules_.size());
    for(auto &rule: rules_){
        auto nfa = rule->nfa.get();
        fprintf(f,"rule %d:\n",rule->idx);
        fprintf(f,"StartIdx:%d,\t EndIdx:%d\n",nfa->StartIndex(), nfa->EndIndex());
        nfa->WriteDotGraph(f);
        fprintf(f,"--------------------------\n");
    }

    fprintf(f,"-----------------------\nMerged NFA:\n");
    nfa_->WriteDotGraph(f);

}

void RegularLexer::showRules(FILE *f) {
    for(auto& rule: rules_){
        fprintf(f,"Rule: %d\n",rule->idx);
        fprintf(f,"Regex: %s\n",rule->regex.data());
        fprintf(f,"Token type: %s: \n",rule->ident.data());
    }
}

void RegularLexer::BuildDFA() {
    int n = nodes_.size();
    dfa_ = std::move(std::make_unique<DFA>(
            nfaStart_,
            this
            ));
    dfa_->BuildDFA();
    for(auto &kv: dfa_->dfaAcceptStates_){
        dfaAcceptStates_.insert(kv.first);
    }
}

void RegularLexer::showDFA(FILE *f) {
    dfa_->WriteToFile(f);
    fprintf(f,"rule -> accept states\n");
    for(auto &rule : rules_){
        fprintf(f,"rule:%d -> {",rule->idx);
        for(auto &item : rule->acceptStates_){
            fprintf(f, "%d, ",item);
        }
        fprintf(f,"}\n");
    }
}


void RegularLexer::GenHeader(){
    auto f = fopen(fheader_.data(),"w");
    fprintf(f,"%s",HEADER_INCLUDE.data());
    fprintf(f,"enum TokenType{\n");
    for(auto &rule: rules_){
        fprintf(f,"\t%s,\n",rule->ident.data());
    }
    fprintf(f,"};\n");
    fprintf(f,"%s",HEADER_CONTENT.data());
    fclose(f);

}

void RegularLexer::GenTable(FILE *f) {
    fprintf(f,"\ttable_={\n");
    auto& states = dfa_->States();
    int col = charClassSet->Size();
    for(auto& state: states){
        fprintf(f,"\t\t{");
        auto& gotoVector = state->GotoVector();
        fprintf(f,"%d",gotoVector[0]);
        for(int i=1;i<col;i++){
            fprintf(f,", %d",gotoVector[i]);
        }
        fprintf(f,"\t\t},\n");
    }
    fprintf(f,"\t};\n");
}

void RegularLexer::GenClass(FILE *f) {
    fprintf(f,"%s",SOURCE_CLASS.data());
}

void RegularLexer::GenAlphabet(FILE *f) {

    fprintf(f,"\talphabet_={");
    for(int i=0;i<alphabet_->size();i++){
        if(i%15==0)
            fprintf(f,"\n\t\t");
        fprintf(f,"%d,",alphabet_->at(i));
    }
    fprintf(f,"\t};\n");
}

void RegularLexer::GenAccepts(FILE *f) {
    fprintf(f,"\tacceptStates_={");
    for(auto &i: dfaAcceptStates_){
        fprintf(f,"%d,",i);
    }
    fprintf(f,"\t};\n");
}

void RegularLexer::GenScanFunction(FILE *f) {
    //lex function
    fprintf(f,"%s",YYSCAN.data());
    //switch function
    for(auto &rule: rules_){
        for(auto &i: rule->acceptStates_){
            fprintf(f,"\t\tcase %d:\n",i);
        }
        fprintf(f,"\t\t{\n");
        //start action
        //对于skipped 的规则直接返回nullptr
        if(rule->isSkipped){
            fprintf(f,"\t\t\t r = nullptr;");
        }else{
            fprintf(f,"\t\t\tr = std::move(std::make_unique<Token>(%s,line, column, p,q));",rule->ident.data());
        }
        fprintf(f," break;\n");
        //end action
        fprintf(f,"\t\t}\n");
    }
    //end of switch
    fprintf(f,"\t}\n");
    fprintf(f,"\treturn r;\n}");
}

void RegularLexer::GenConstructor(FILE *f) {
    fprintf(f,"%s",SOURCE_CONSTRUCTOR_PREV.data());
    GenTable(f);
    GenAlphabet(f);
    GenAccepts(f);
    //end of constructor
    fprintf(f,"}\n");

}

void RegularLexer::GenSource() {
    auto f = fopen(fsource_.data(),"w");
    fprintf(f,"%s",SOURCE_INCLUDE.data());
    fprintf(f,"#include \"%s\"\n",fheader_.data());
    GenClass(f);
    GenConstructor(f);
    GenScanFunction(f);
    fprintf(f,"\n%s",TOKEN_STR_FUNC.data());
    fprintf(f,"\n%s",CREATE_LEXER_FUNC.data());
    if(!code_.empty()){
        fprintf(f,"\n%s",code_.data());
    }
    fclose(f);

}

void RegularLexer::expect(int ch) {
    int c = readc();
    if(c!=ch){
        fprintf(stderr,"Expected '%c' but got '%c'\n",ch, c);
        exit(EXIT_FAILURE);
    }
}

void RegularLexer::skipLine() {
    for(;;){
        int c = readc();
        if(c==EOF)
            return;
        if(c=='\n'){
            unreadc();
            return;
        }
    }
}

bool RegularLexer::doSkipSpace() {
    int c= readc();
    if(c == EOF)
        return false;
    if(isWhiteSpace(c))
        return true;
    if(c=='/'){
        if(next('*')){
            skipBlockComment();
            return true;
        }
        if(next('/')){
            skipLine();
            return true;
        }
    }
    unreadc();
    return false;
}

bool RegularLexer::skipSpace() {
    if(!doSkipSpace())
        return false;
    while(doSkipSpace());//do nothing
    return true;
}

void RegularLexer::skipBlockComment() {
    bool maybeEnd = false;
    for(;;){
        int c = readc();

        if(c=='/' && maybeEnd){
            //printf("next ch is %c\n",buffer_[idx_+1]);
            return;
        }
        maybeEnd = (c=='*');
    }
}

void RegularLexer::parseCode() {
    skipSpace();
    static string CODE_STR = "%code{";
    int i=0;
    while(i<CODE_STR.length()){
        if(CODE_STR[i] != readc())
            break;
        i++;
    }
    if(i == CODE_STR.size()){
        //parse include
        parseRawCode(code_);
    }else{
        while(i--){
            unreadc();
        }
    }
}

std::string RegularLexer::readIdent() {
    string r;
    int ch;
    while((ch = readc())!=EOF){
        if(isalnum(ch)){
            r.push_back(ch);
        }else
            break;
    }
    unreadc();
    return r;
}

void RegularLexer::parseInclude() {
    skipSpace();
    static string INCLUDE_STR = "%include{";
    int i=0;
    while(i<INCLUDE_STR.length()){
        if(INCLUDE_STR[i++] != readc())
            break;
    }
    if(i == INCLUDE_STR.size()){
        //parse include
        parseRawCode(include_);
    }else{
        while(i--){
            unreadc();
        }
    }
}

void RegularLexer::parseRawCode(std::string &s) {
    //now idx is at {
    int nested = 1;
    int ch = 0; //ch is char next  {
    while(nested>0){//until ch = 0
        ch = readc();
        panic_if(ch<0, "Unexpected EOF at code section");
        s.push_back(ch);
        if(ch == '{'){
            nested++;
        }else if(ch=='}'){
            nested--;
        }
    }
    //now idx is at }
    code_.pop_back();
    readc();
}
