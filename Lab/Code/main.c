#include <stdio.h>
#include <string.h>
#include <stdlib.h>

extern FILE* yyin;
extern int yylex(void);
extern void yyrestart(FILE * input_file );
extern void printAST(void);
extern void yyparse(void);

int main(int argc, char** argv) {
    if (argc <= 1) return 1;
    FILE* f = fopen(argv[1], "r");
    if (!f) {
        perror(argv[1]);
        return 1;
    }
    yyrestart(f);
    yyparse();
    printAST();
    return 0;
}
