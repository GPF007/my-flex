//
// Created by gpf on 2020/10/8.
//

#ifndef MYLEXER_STATE_H
#define MYLEXER_STATE_H

#include <set>
#include <vector>
#include <memory>

class State {
    friend class DFA;
public:

    void AddNodeIndex(int i){nfaNodes_.insert(i);}
    bool HasNodeIndex(int i){return !(nfaNodes_.find(i) == nfaNodes_.end());}
    const std::set<int>& Nodes()    {return nfaNodes_;}
    void SetIndex(int i)            {index_ = i;}
    bool Empty()                    {return nfaNodes_.empty();}
    std::vector<int>& GotoVector()  {return gotoArray_;}
    int Index()                     {return index_;}
    int WileNext()                  {return gotoArray_[0];}
    int NextAt(int idx)             {return gotoArray_[idx];}
    void WriteToFile(FILE* f);
    int GetCharClasses();
    bool equal(State* other){
        //比较两个set
        return nfaNodes_ == other->nfaNodes_;
    }

private:
    std::set<int> nfaNodes_;
    std::vector<int> gotoArray_;
    int index_ = -1;
};
typedef std::unique_ptr<State> StatePtr;


#endif //MYLEXER_STATE_H
