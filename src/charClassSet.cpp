//
// Created by gpf on 2020/10/7.
//

#include "charClassSet.h"
CharClassSet::CharClassSet() {
    auto wild = std::make_unique<WildCharClass>();
    wild->SetIndex(count++);
    cclasses.push_back(std::move(wild));
}

void CharClassSet::WriteToFile(FILE *f) {
    fprintf(f,"CharClass Count: %d \n",count);
    fprintf(f,"[Wild]: \n");
    for(size_t i=1;i<count;i++){
        auto cclass = cclasses[i].get();
        if(cclass->isSingle()){
            auto tmp = static_cast<SingleCharClass*>(cclass);
            fprintf(f,"[Single]: %c    index:%d\n", static_cast<char>(tmp->Ch()),tmp->index);
        }else if(cclass->isRange()){
            auto tmp = static_cast<RangeCharClass*>(cclass);
            fprintf(f,"[Range]: [");
            if(tmp->isNegate()){
                fprintf(f,"^");
            }
            for(auto &lr: tmp->Range()){
                fprintf(f,"%c", static_cast<char >(lr.first));
                if(lr.second != lr.first){
                    fprintf(f,"-%c", static_cast<char>(lr.second));
                }
            }
            fprintf(f,"]    index:%d\n",tmp->index);
        }
    }
    fprintf(f,"\n");
}