#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "Tree.h"
#include "ir.h"

extern FILE* yyin;
extern int yylex(void);
extern void yyrestart(FILE * input_file );
extern void printAST(void);
extern void yyparse(void);
extern TreeNode_t* getRoot();

int main(int argc, char** argv) {
    if (argc <= 2) return 1;
    FILE* f = fopen(argv[1], "r");
    FILE* ir_file = fopen(argv[2], "w");
    if (!f) {
        perror(argv[1]);
        return 1;
    }
    yyrestart(f);
    yyparse();
    //printAST();
    ir_Program(getRoot(), ir_file);
    return 0;
}
