#ifndef IR_H
#define IR_H

#include <stdio.h>
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
    enum { VARIABLE, TEMP, FUNCT, LABEL, CONSTANT} kind;
    enum MODE mode;
    enum { NORMAL, DEF, REF} print_mode;
    union {
        int var_no;
        int label;
        int value;
    } u;
};

struct InterCode_ {
    enum { LABELSET, FUNCTION, ASSIGN, ADD, SUB, MUL, DIV, ADDR, DEREF, REF_ASSIGN, 
            GOTO, CONDJMP, RETURN, DEC, ARG, CALL, PARAM, READ, WRITE  } kind;
    union {
        struct { Operand right, left; } assign;
        struct { Operand result, op1, op2; } binop;
        struct { Operand op; } label;
        struct { Operand op; } unary;
        struct { Operand op1, op2; Operand target; char relop[5]; } condjmp;
        struct { Operand op; int size; } dec;
    } u;
};

struct InterCodes_ { InterCode code; struct InterCodes_ *prev, *next; };




#endif

