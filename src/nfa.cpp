//
// Created by gpf on 2020/10/7.
//

#include "nfa.h"
#include <assert.h>
#include <sstream>
#include <set>
#include <unordered_map>
#include "RegularLexer.h"
#define panic(str) {fprintf(stderr,"%s\n",str);exit(1);}
#define panic_if(expr,str) if(expr) panic(str)

std::string NFA::puncts="!\"#$%&'()*+,-./:;<=>?@[\\]^_`{|}~";
std::string NFA::escaped="\a\b\f\n\r\t\v";
std::string NFA::escapes="abfnrtv";

/*
NFA::NFA(std::string &r, Arena *a, CharClassSet *cs, std::array<int,256>* arr, std::vector<Node*>* ns):
    regex_(r), arena_(a),rl_->charClassSet(cs),alphabet_(arr),nodes_(ns){
}
 */

Node* NFA::newNode(){    return rl_->arena_->newNode(nodeIdx_++);}


Edge* NFA::newWildEdge(Node *u, Node *v)     {
    auto r= newEdge(u,v,kCharClass);
    r->cclass = rl_->charClassSet->Get(0); //0号位置是wild,hh
    return r;
}

Edge * NFA::newEdge(Node *u, Node *v,EDGEKIND k) {
    auto r = rl_->arena_->newEdge(v,k);
    u->AddEdge(r);
    //todo sort(e.edges)
    return r;
}


NodePair NFA::BuildNFA(){
    auto nodes = parseTerm();
    nodes.second->isAccept_ = true;
    auto nfaStart = nodes.first;
    auto nfaEnds = nodes.second;
    reduceNodes(nfaStart);

    nfaStart_ = nodes.first;
    nfaEnd_ = nodes.second;

    return nodes;
}

bool NFA::isPunct(int ch) {
    for(auto &c: puncts){
        if(c==ch)
            return true;
    }
    return false;
}

int NFA::escape(int ch) {
    for(size_t i=0;i<escapes.size();i++){
        if(escapes[i]==ch)
            return escaped[i];
    }
    return -1;
}



int NFA::getChar() {
    auto ch = regex_[regexIdx_];
    if(ch=='\\'){
        ch = regex_[++regexIdx_];
        if(isPunct(ch)){
            return ch;
        }else if(escape(ch)>=0){
            return escape(ch);
        }
        panic("unknown escaped char");
    }

    return ch;
}

NodePair NFA::parseBasicTerm() {
    //如果到达末尾或者遇到了　| ，则返回一个新的node
    if(regexIdx_ == regex_.size() || regex_[regexIdx_]=='|'){
        auto end = newNode();
        return NodePair(end, end);//start = end
    }
    switch (regex_[regexIdx_]) {
        case '*':case '+': case '?':
            panic("closure applies to nothing");
        case ')':{
            panic_if(!isNested_,"must be nested!");
            auto end = newNode();
            return NodePair(end, end);
        }
        case '(':{
            regexIdx_++;
            auto oldIsNested = isNested_;
            isNested_ = true;
            auto r = parseTerm();
            isNested_ = oldIsNested;
            panic_if(regexIdx_ == regex_.size() || regex_[regexIdx_]!=')',
                     "Expected right parent");
            regexIdx_++;
            return r;
        }
        case '.':{
            auto start = newNode();
            auto end = newNode();
            newWildEdge(start, end);
            regexIdx_++;
            return NodePair(start, end);
        }
        case '[':{
            regexIdx_++;
            auto nodes = parseRangeTerm();
            if(regexIdx_==regex_.size() || regex_[regexIdx_]!=']'){
                panic("Err Unmatched ]");
            }
            regexIdx_++;
            return nodes;

        }
        default:{
            auto start = newNode();
            auto end = newNode();
            newCharEdge(start, end, getChar());
            regexIdx_++;
            return NodePair(start, end);
        }
    }

}

Edge * NFA::newCharEdge(Node *u, Node *v, int ch) {
    auto r= newEdge(u,v,kCharClass);
    CharClassPtr tmp = std::make_unique<SingleCharClass>(ch);
    auto res = rl_->charClassSet->Find(tmp.get());
    if(res){
        //delete tmp;
        r->cclass = res;
    }else{//not found, new char class
        r->cclass = tmp.get();
        //tmp->SetIndex(charClassIdx++);
        //设置alphabet_
        //alphabet_->at(ch) = tmp->index;
        rl_->charClassSet->Add(std::move(tmp));
        rl_->alphabet_->at(ch) = r->cclass->index;
    }
    return r;
}


//parse * + ? symbol
NodePair NFA::parseClosureTerm() {
    auto nodes = parseBasicTerm();
    auto start = nodes.first;
    auto end = nodes.second;

    if(start == end || regexIdx_ == regex_.size()){
        return nodes;
    }

    switch (regex_[regexIdx_++]) {
        case '*':{
            newNullEdge(end, start);
            auto newEnd = newNode();
            newNullEdge(end, newEnd);
            return NodePair(end, newEnd);
        }
        case '+':{
            newNullEdge(end, start);
            auto newEnd = newNode();
            newNullEdge(end, newEnd);
            return NodePair(start, newEnd);
        }
        case '?':{
            auto newStart =  newNode();
            newNullEdge(newStart, start);
            newNullEdge(newStart, end);
            return NodePair(newStart, end);
        }
        default:{//默认情况不用递增regexIdx_
            --regexIdx_;
            return nodes;
        }
    }
}

NodePair NFA::parseCatTerm() {
    Node* start = nullptr;
    Node* end = nullptr;
    for(;;){
        auto nodes = parseClosureTerm();
        auto nstart = nodes.first;
        auto nend = nodes.second;
        if(start== nullptr){//第一个节点
            start = nstart;
            end = nend;
        }else if(nstart!=nend){//不是结束节点
            end->CopyEdges(nstart);
            end = nend;
        }
        if(nstart == nend) {
            break;
        }
    }
    return NodePair(start, end);
}

NodePair NFA::parseTerm() {
    auto nodes = parseCatTerm();
    auto start = nodes.first;
    auto end = nodes.second;

    while(regexIdx_<regex_.size() && regex_[regexIdx_]!=')'){
        panic_if(regex_[regexIdx_]!='|',"must meet |");
        regexIdx_++;
        auto newNodes = parseCatTerm();
        auto nstart = newNodes.first;
        auto nend = newNodes.second;

        //建立　| 的图
        auto tmp = newNode();
        newNullEdge(tmp,start);
        newNullEdge(tmp, nstart);
        start = tmp;
        tmp = newNode();
        newNullEdge(end, tmp);
        newNullEdge(nend, tmp);
        end = tmp;
    }
    return NodePair(start, end);

}

//important!!
NodePair NFA::parseRangeTerm() {
    auto start = newNode();
    auto end = newNode();
    auto edge = newRangeEdge(start, end);
    CharClassPtr cclass = std::make_unique<RangeCharClass>();
    RangeCharClass* rangeCharClass = static_cast<RangeCharClass*>(cclass.get());

    auto addSingleCharToRange = [rangeCharClass](int ch){
        rangeCharClass->AddChar(ch);
    };

    auto addRangeCharToRange = [rangeCharClass](int start, int end){
        if(end < start) return;
        rangeCharClass->AddRange(start, end);
    };

    if(regexIdx_< regex_.size() && regex_[regexIdx_]=='^'){
        rangeCharClass->SetNegate();
        regexIdx_++;
    }
    bool sawDash = false;
    int left=-1;
    //parse chars between []

    while(regexIdx_< regex_.size() && regex_[regexIdx_]!=']'){
        auto c = getChar();
        if(c == '-'){
            sawDash = true;
        }else{
            if(sawDash){//right half
                panic_if(left<0,"there must left half before dash");
                addRangeCharToRange(left, c);
                left = -1;
                sawDash = false;
            }else{//left half
                if(left>=0){
                    addSingleCharToRange(left);
                }
                left = c;
            }
        }
        regexIdx_++;
    }

    if(left>=0){
        addSingleCharToRange(left);
    }
    //判断该charclass是否是已经存在的
    //然后判断该range内的所有字符是否已经在其他的charclass中了，如果在则加入一条边
    //判断该CHAR CLASS是否已经存在
    auto res = rl_->charClassSet->Find(rangeCharClass);
    if(res){
        edge->cclass = res;
    }else{
        edge->cclass = rangeCharClass;
        rl_->charClassSet->Add(std::move(cclass));
        //用来存储之后需要添加的边的classIndex
        std::set<int> tmpClassIndexs;
        //此时需要检查冲突
        for(auto &lr: rangeCharClass->Range()){
            for(int i=lr.first; i<= lr.second; i++){
                if(rl_->alphabet_->at(i)<0){//该charclass对应的字符没有设置
                    rl_->alphabet_->at(i) = rangeCharClass->index;
                }else{//此时新加一条边，charclassindex为重复的char
                    tmpClassIndexs.insert(rl_->alphabet_->at(i));
                }
            }
        }

        //加入剩余的边
        for(auto &idx: tmpClassIndexs){
            newEdgeWithIndex(start, end, idx);
        }
    }



    return NodePair(start, end);
}

void NFA::WriteDotGraph(FILE *f) {
    fprintf(f,"digraph NFA {\n");
    std::unordered_map<Node*, bool> done;
    showNode(f,nfaStart_, done);
    fprintf(f,"}");
    fprintf(f,"\n");
    //fprintf(f,"CharClass Count: %d\n",charClassSet->Size());
    //showCharClasses(f);

}

void NFA::showNode(FILE * f,Node* node, std::unordered_map<Node*,bool>& done){
    if(node->isAccept_){
        fprintf(f," %d[style=filled, color=green];\n", node->index_);
    }
    done[node]= true;
    for(auto e: node->outedges_){
        if(e->dst->index_ == -1) continue;
        std::stringstream label;
        switch (e->kind) {
            case kCharClass:{
                auto cclass = e->cclass;
                assert(cclass);
                if(cclass->isSingle()){
                    auto singleClass = static_cast<SingleCharClass*>(cclass);
                    label<<"[label='"<< static_cast<char>(singleClass->Ch()) << "']";
                }else if(cclass->isWild()){
                    label<<"[color=blud]";
                }else if(cclass->isRange()){
                    label << "label=\"[";
                    auto rangeClass = static_cast<RangeCharClass*>(cclass);
                    if(rangeClass->isNegate()){
                        label << "^";
                    }
                    for(auto &lr: rangeClass->Range()){
                        label<< static_cast<char>(lr.first);
                        if(lr.second != lr.first){
                            label<<"-"<< static_cast<char>(lr.second);
                        }
                    }
                    label << "]\"]";
                }
                break;
            }
            case kNull:{
                label<<"label=[null]";
                break;
            }
            default:{
                label<<"nothing";
                break;
            }
        }
        fprintf(f," %d -> %d %s;\n",node->index_, e->dst->index_, label.str().data());
    }
    for(auto e: node->outedges_){
        if(!done[e->dst]){
            showNode(f,e->dst,done);
        }
    }
}


//有些节点已经不需要了
void NFA::reduceNodes(Node* nfaStart) {
    int originNodeCount = nodeIdx_;
    std::vector<int> mark(originNodeCount,0);
    std::vector<int> newIndex(originNodeCount, 0);
    int startIndex = rl_->nodes_.size();

    visitNode(nfaStart,mark, newIndex);
    for(int i=startIndex;i<rl_->nodes_.size(); i++){
        auto& node = rl_->nodes_[i];
        node->index_ = newIndex[node->index_];
    }
}

void NFA::visitNode(Node* node, std::vector<int>& mark, std::vector<int>& newIndex){
    mark[node->index_]=1;
    newIndex[node->index_] = rl_->nodes_.size();
    rl_->nodes_.push_back(node);
    for(auto e: node->outedges_){
        if(!mark[e->dst->index_])
            visitNode(e->dst, mark, newIndex);
    }
}

Edge * NFA::newEdgeWithIndex(Node *u, Node *v, int idx) {
    auto r= newEdge(u,v,kCharClass);
    r->cclass = rl_->charClassSet->Get(idx);
    return r;
}