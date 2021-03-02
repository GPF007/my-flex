//
// Created by gpf on 2020/10/8.
//

#include "state.h"

void State::WriteToFile(FILE *f) {
    fprintf(f,"State:%d [",index_);
    for(auto &idx: nfaNodes_){
        fprintf(f,"%4d,",idx);
    }
    fprintf(f,"]\n");
}