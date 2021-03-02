#include <iostream>
#include <unistd.h>

#include "RegularLexer.h"



using namespace std;

void printHelp();

int main(int argc, char * argv[]) {
    //std::cout << "Hello, World!" << std::endl;

    int ch;
    //后面２个冒号说明是可选的
    bool showNFA =false;
    bool showDFA = false;
    bool showRule = false;
    while((ch = getopt(argc, argv,"ndlh"))!=-1){
        switch (ch) {
            case 'n':
                showDFA = true;
                break;
            case 'd':
                showNFA = true;
                break;
            case 'l':
                showRule = true;
            default:
                printHelp();
                return 0;
        }
    }

    if(!argv[optind]){
        fprintf(stderr,"Exected a .l file\n");
        exit(EXIT_FAILURE);
    }
    std::string fname = std::string(argv[optind]);
    if(fname.back() !='l'){
        fprintf(stderr,"Expected a file with .l suffix\n");
        exit(EXIT_FAILURE);
    }
    //base name 是去除.l后缀的name
    auto basename = fname;
    basename.pop_back();
    basename.pop_back();
    //now build nfa and dfa
    auto regularLexer = std::make_unique<RegularLexer>(fname.data());
    regularLexer->Parse();
    regularLexer->BuildNFA();
    regularLexer->BuildDFA();
    if(showRule){
        auto fname = basename+".rule";
        FILE *ruleFile = fopen(fname.data(),"w");
        regularLexer->showRules(ruleFile);;
        fclose(ruleFile);
    }

    if(showNFA){
        auto fname = basename+".nfa";
        FILE *ruleFile = fopen(fname.data(),"w");
        regularLexer->showNFA(ruleFile);;
        fclose(ruleFile);
    }

    if(showDFA){
        auto fname = basename+".dfa";
        FILE *ruleFile = fopen(fname.data(),"w");
        regularLexer->showDFA(ruleFile);;
        fclose(ruleFile);
    }

    regularLexer->GenHeader();
    regularLexer->GenSource();

    return 0;
}

void printHelp(){
    printf("./mylexer [-n][-d][-l] fname\n");
    printf("\t-n\t\t= show NDA dot at file\n");
    printf("\t-d\t\t= show NFA dot at file\n");
    printf("\t-l\t\t= show rules collection at file\n");
    printf("\tfname is a lexer gammer file with '.l' suffix\n");

}