//
// Created by gpf on 2020/10/7.
//

#ifndef MYLEXER_CHARCLASSSET_H
#define MYLEXER_CHARCLASSSET_H


#include <memory>
#include "charClass.h"
//这个类用来存放所有的charclasses
typedef std::unique_ptr<CharClass> CharClassPtr;
class CharClassSet {
public:
    CharClassSet();

    CharClass* Get(int i)       {return cclasses[i].get();}
    CharClass* Find(CharClass* cclass){
        for(auto &item: cclasses){
            if(cclass->equal(item.get()))
                return item.get();
        }
        return nullptr;
    }
    int Size()          {return cclasses.size();}
    void Add(CharClassPtr cptr) {
        cptr->SetIndex(count++);
        cclasses.push_back(std::move(cptr));
    }
    std::vector<CharClassPtr>& Cclasses()       {return cclasses;}

    //for debug
    void WriteToFile(FILE* f);

private:
    std::vector<CharClassPtr> cclasses;
    int count=0;
};


#endif //MYLEXER_CHARCLASSSET_H
