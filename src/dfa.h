//
// Created by gpf on 2020/10/8.
//

#ifndef MYLEXER_DFA_H
#define MYLEXER_DFA_H

#include <map>
#include "node.h"
#include "state.h"
#include "charClassSet.h"


class RegularLexer;

class DFA {
    friend class RegularLexer;
public:
    void BuildDFA();
    DFA(Node* start, RegularLexer* rl);
    void WriteToFile(FILE* f);
    std::vector<StatePtr>& States() {return states_;}
    int RuleIdxAt(int idx);
    int NextStateAt(int stateIdx, int charIdx)  {return states_[stateIdx]->NextAt(charIdx);}
private:
    Node* nfaStart_;
    RegularLexer* rl_;
    std::vector<Node*>& nodes_;
    typedef std::map<int, int> IntMap;

    //自己的变量
    std::vector<StatePtr> states_;
    IntMap dfaAcceptStates_; //state_idx => rule_idx
    int dfaCounts_=0;

    //methods to get dfa
    State* nullClosure(Node* node);
    void nullClosure(State* node);
    StatePtr move(State* node, CharClass* cclass);

    int getCharClasses(State* State);
    State* findState(State* s);



};


#endif //MYLEXER_DFA_H
