
relop   >|<|>=|<=|==|!=
type    int|float
%%

";"         { printf("SEMI\n"); }
","         { printf("COMMA\n"); }
"="         { printf("ASSIGNOP\n"); }
{relop}     { printf("RELOP\n"); }
"+"         { printf("PLUS\n");}
"-"         { printf("MINUS\n");}
"*"         { printf("STAR\n");}
"/"         { printf("DIV\n");}
"&&"        { printf("AND\n");}
"||"        { printf("OR\n");}
"."         { printf("DOT\n");}
{type}      { printf("TYPE\n");}
"("         { printf("LP\n");}
")"         { printf("RP\n");}
"["         { printf("LB\n");}
"]"         { printf("RB\n");}
"{"         { printf("LC\n");}
"}"         { printf("RC\n");}
"struct"    { printf("STRUCT\n");}
"return"    { printf("RETURN\n");}
"if"        { printf("IF\n");}
"else"      { printf("ELSE\n");}
"while"     { printf("WHILE\n");}


%%


int main(int argc, char** argv) {
    if (argc> 1) {
        if (!(yyin=fopen(argv[1], "r"))) {
            perror(argv[1]);
            return 1;
        }
    }
    while (yylex() != 0);
    return 0;
}



