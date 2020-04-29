#ifndef IR_H
#define IR_H

#include "Tree.h"
#include "symbol.h"

#define TYPE_INT 0
#define TYPE_FLOAT 1

typedef struct Operand_ Operand_t;
typedef Operand_t* Operand;

typedef struct InterCode_ InterCode_t;
typedef InterCode_t* InterCode;

typedef struct InterCodes_ InterCodes_t;
typedef InterCodes_t* InterCodes;


struct Operand_ {
    enum { VARIABLE, TEMP, LABEL, CONSTANT, ADDRESS, FUNC } kind;
    union {
        int var_no;
        int label;
        int value;
    } u;
};

struct InterCode_ {
    enum { LABEL, FUNCTION, ASSIGN, ADD, SUB, MUL, DIV, ADDR, DEREF, REF_ASSIGN, 
            GOTO, CONJMP, RETURN, DEC, ARG, CALL, PARAM, READ, WRITE  } kind;
    union {
        struct { Operand right, left; } assign;
        struct { Operand result, op1, op2; } binop;
        struct { Opreand op; } label;
        struct { Operand op; } unary;
        struct { Operand op1, op2; int target; } conjmp;
    } u;
};

struct InterCodes_ { InterCode code; struct InterCodes_ *prev, *next; };



void sdt_init();

/* High-level Definitions */



/* Specifiers */


/* Declarators */


/* Statements */


/* Local Definitions */



/* Expressions */



/* Terminator */


#endif