

#rule
{\{} => LBRACE

#rule
{\}} => BBRACE

#rule
{[0-9]+} => INT

#rule
{"((\\")|.)*"}         => STR

#rule
{[a-z][a-z_0-9]*} => IDENT


#skip
{[ \t\n]+}        => WS


#skip
{\(\+.*\+\)} => COMMENT
#endrules

%code{
int main(int argc, char * argv[])
{
     auto lexer = createLexer(argv[1]);
        for(int i=0;i<100;i++){

            std::cout<<"i="<<i<<std::endl;
            auto tok =  lexer->Scan2();
            std::cout<<tok->toString()<<std::endl;
            if(tok->sval == "EOF")
                break;
            //std::cout<<tok->toString()<<std::endl;
        }
     delete lexer;
}


}
