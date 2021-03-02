//
// Created by gpf on 2020/10/8.
//

#include "dfa.h"
#include "RegularLexer.h"
#include <stack>

DFA::DFA(Node *start, RegularLexer* rl):
    nfaStart_(start), rl_(rl),nodes_(rl_->nodes_){

}

//这个函数不会创建新的node
void DFA::nullClosure(State* state){
    auto& nodeIdxs = state->Nodes();
    std::stack<int> nodeStack;
    //讲所有索引的节点加入栈
    for(auto idx: nodeIdxs){
        nodeStack.push(idx);
    }

    while(!nodeStack.empty()){
        auto tmpNode =  nodes_.at(nodeStack.top());
        nodeStack.pop();
        for(auto edge: tmpNode->outedges_){
            if(edge->kind!= kNull) continue;
            //如果没有通过ｎｕｌｌ的边到达的节点
            if(!state->HasNodeIndex(edge->dst->index_)){
                state->AddNodeIndex(edge->dst->index_);
                nodeStack.push(edge->dst->index_);
            }
        }

    }
}

State* DFA::nullClosure(Node* node){
    auto newState = new State();
    newState->AddNodeIndex(node->index_);
    std::stack<int> nodeStack;
    //讲所有索引的节点加入栈
    nodeStack.push(node->index_);
    while(!nodeStack.empty()){
        auto tmpNode =  nodes_.at(nodeStack.top());
        nodeStack.pop();
        for(auto edge: tmpNode->outedges_){
            if(edge->kind!= kNull) continue;
            //如果没有通过ｎｕｌｌ的边到达的节点
            if(!newState->HasNodeIndex(edge->dst->index_)){
                newState->AddNodeIndex(edge->dst->index_);
                nodeStack.push(edge->dst->index_);
            }
        }

    }

    return newState;
}

//node with cclass 这个函数会创建一个新的unique_ptr
StatePtr DFA::move(State* node, CharClass* cclass){
    auto newState = std::make_unique<State>();
    auto& nodeIndexs = node->Nodes();
    for(auto idx: nodeIndexs){
        auto nfaNode = nodes_[idx];
        for(auto e:nfaNode->outedges_){
            if(e->cclass && e->cclass->equal(cclass)){
                newState->AddNodeIndex(e->dst->index_);
            }
        }
    }
    return newState;
}

State* DFA::findState(State* s){
    for(auto& node: states_){
        if(node->equal(s)){
            return node.get();
        }
    }
    return nullptr;
}

void DFA::BuildDFA(){
    //初始的状态是 null-closure(s0)
    auto charClassSet = rl_->charClassSet.get();
    auto dfaStart = nullClosure(nfaStart_);
    dfaStart->SetIndex(dfaCounts_++);
    states_.emplace_back(dfaStart);
    //用来存储未标记的DFA State
    std::stack<State*> DFAState;
    DFAState.push(dfaStart);
    //一共有几种输出类型
    while(!DFAState.empty()){
        auto state = DFAState.top();
        DFAState.pop();
        //每一个DFA状态对应一个向量,即goto table
        auto tmpVec = std::vector<int>(charClassSet->Size(), -1);
        for(auto &cclass: charClassSet->Cclasses()){
            auto tmp = move(state, cclass.get());
            nullClosure(tmp.get());
            //这里tmp可能是空的node
            if(tmp->Empty()){
                continue;
            }
            State* newNode = findState(tmp.get());
            if(!newNode){
                //没有找到对应的node,则ｔｍｐ是新的node
                newNode = tmp.get();
                newNode->SetIndex(dfaCounts_++);
                DFAState.push(newNode);
                states_.push_back(std::move(tmp));
            }
            //此时new node 是node 在　cclass的下一个状态
            tmpVec.at(cclass->index) = newNode->index_;
        }
        state->gotoArray_ = std::move(tmpVec);
    }

    auto& nfaAcceptStates = rl_->nfaAcceptStates_;
    //完成之后根据nfa的结束state来确定accpet的state
    //确定accept对应的rule
    auto getRuleIndex = [&nfaAcceptStates](State* state){
        //这里一定是从小到大find
        for(auto nfaIdx: state->nfaNodes_){
            auto it = nfaAcceptStates.find(nfaIdx);
            if(it!= nfaAcceptStates.end())
                return it->second;
        }
        return -1;
    };

    for(auto& state: states_){
        auto endRuleIdx = getRuleIndex(state.get());
        if(endRuleIdx>=0){
            dfaAcceptStates_.insert({state->index_, endRuleIdx});
            rl_->rules_[endRuleIdx]->acceptStates_.push_back(state->index_);
        }
    }
}

void DFA::WriteToFile(FILE *f) {
    auto charClassSet = rl_->charClassSet.get();
    fprintf(f,"CharClasses:\n");
    charClassSet->WriteToFile(f);
    fprintf(f,"-----------------\n");
    fprintf(f,"DFAState count: %d\n",dfaCounts_);
    for(auto &node: states_){
        node->WriteToFile(f);
    }

    fprintf(f,"-----------------\n");
    fprintf(f,"DFAState table:\n");
    int i=0;
    for(auto &node:states_){
        fprintf(f,"State %d:  ",i++);
        for(auto n: node->GotoVector()){
            fprintf(f,"%4d",n);
        }
        fprintf(f,"\n");
    }

    fprintf(f,"-----------------\n");
    fprintf(f,"Accepting states:\n");
    fprintf(f,"State  -->   RuleIndex\n");
    for(auto& kv: dfaAcceptStates_){
        fprintf(f,"%5d  ->   %5d\n",kv.first, kv.second);
    }

}

int DFA::RuleIdxAt(int idx) {
    auto it = dfaAcceptStates_.find(idx);
    if(it!=dfaAcceptStates_.end()){
        return it->second;
    }
    return -1;
}