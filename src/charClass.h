//
// Created by gpf on 2020/10/7.
//

#ifndef MYLEXER_CHARCLASS_H
#define MYLEXER_CHARCLASS_H


#include <vector>


class CharClass{
    friend class NFA;
public:

    CharClass(){}

    void SetIndex(int i) {index = i;}
    virtual bool equal(CharClass* other)=0;
    virtual bool isSingle() = 0;
    virtual bool isRange()  = 0;
    virtual bool isNegate() = 0;
    virtual bool isWild()   =0;


    virtual ~CharClass(){}


    int index;//global index
};


class SingleCharClass: public CharClass{
public:
    SingleCharClass(int c):ch(c){}

    bool equal(CharClass *other) override{
        return other->isSingle() && ch == reinterpret_cast<SingleCharClass*>(other)->ch;
    }
    bool isNegate() override  {return false;}
    bool isRange() override   {return false;}
    bool isSingle() override  {return true;}
    bool isWild() override   {return false;}

    int Ch()        {return ch;}
private:
    int ch;
};

class RangeCharClass: public CharClass{
public:
    RangeCharClass(){}

    bool equal(CharClass *other) override{
        return other->isRange() && range == static_cast<RangeCharClass*>(other)->range;
    }
    bool isNegate() override  {return negate;}
    bool isRange() override   {return true;}
    bool isSingle() override  {return false;}
    bool isWild() override   {return false;}

    //non-virtual
    void AddChar(int ch){range.push_back(std::make_pair(ch, ch));}
    void AddRange(int start, int end) {range.push_back(std::make_pair(start, end));}
    void SetNegate()        {negate = true;}
    std::vector<std::pair<int,int>>& Range()           {return range;}
    bool isEmpty()  {return range.empty();}
private:
    std::vector<std::pair<int,int>> range;
    bool negate = false;
};

class WildCharClass: public CharClass{
public:
    WildCharClass(){}
    bool equal(CharClass *other) override{
        return other->isWild();
    }
    bool isNegate() override  {return false;}
    bool isRange() override   {return false;}
    bool isSingle() override  {return false;}
    bool isWild() override   {return true;}
};


#endif //MYLEXER_CHARCLASS_H
