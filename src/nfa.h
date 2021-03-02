//
// Created by gpf on 2020/10/7.
//

#ifndef MYLEXER_NFA_H
#define MYLEXER_NFA_H
#include "node.h"
#include "charClassSet.h"
#include <unordered_map>


class RegularLexer;

class NFA {

public:
    NFA(std::string& r, RegularLexer* rl): regex_(r), rl_(rl){}
    //NFA(std::string& r,Arena* a, CharClassSet* cs,std::array<int,256>* arr, std::vector<Node*>* ns);
    NFA(Node* start, Arena* a, CharClassSet* cs,std::array<int,256>* arr, std::vector<Node*>* ns);
    NodePair BuildNFA();
    //std::array<int,256>& Alphabet() {return alphabet;}
    void WriteDotGraph(FILE* f);
    void SetStart(Node* s) {nfaStart_ = s;}
    Node* Start()   {return nfaStart_;}
    Node* End()     {return nfaEnd_;}
    int StartIndex()   {return nfaStart_->index_;}
    int EndIndex()     {return nfaEnd_->index_;}
    //static members
    static std::string puncts;
    static std::string escapes;
    static std::string escaped;

private:
    std::string& regex_;
    RegularLexer* rl_;
    int nodeIdx_=0;
    //Arena* arena_;
    //CharClassSet* charClassSet_;
    int regexIdx_=0;
    //std::array<int,256>* alphabet_;
    bool isNested_ = false;
    //std::vector<Node*> *nodes_;
    Node* nfaStart_;
    Node* nfaEnd_;


    //some help method to build node and edge
    //Node* newNode(){    return arena_->newNode(nodeIdx_++);}
    Node* newNode();
    Edge* newEdge(Node* u, Node* v,EDGEKIND k= kNull);
    Edge* newStartEdge(Node *u, Node *v)    {return newEdge(u,v,kStart);}
    Edge* newEndEdge(Node *u, Node *v)      {return newEdge(u,v, kEnd);}
    Edge* newWildEdge(Node *u, Node *v);
    Edge* newCharEdge(Node *u, Node *v,int ch);
    Edge* newEdgeWithIndex(Node* u, Node* v, int idx);
    Edge* newNullEdge(Node *u, Node *v)     {return newEdge(u,v,kNull);}
    Edge* newRangeEdge(Node *u, Node *v)    {return newEdge(u,v, kCharClass);}
    NodePair parseBasicTerm();
    NodePair parseClosureTerm();
    NodePair parseCatTerm();
    NodePair parseTerm();
    NodePair parseRangeTerm();

    //help functions
    int getChar();
    void showNode(FILE* f, Node* node, std::unordered_map<Node*,bool>& done);
    void visitNode(Node* visit, std::vector<int>& mark, std::vector<int>& newIndex);
    void reduceNodes(Node* start);


    //static function and members
    static bool isPunct(int ch);
    static int escape(int ch);
};




#endif //MYLEXER_NFA_H
