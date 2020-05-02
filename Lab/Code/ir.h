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

typedef struct expType_ expType_t;
typedef expType_t* expType;


struct Operand_ {
    enum { VARIABLE, TEMP, FUNCT, LABEL, CONSTANT} kind;
    enum MODE mode;
    enum { NORMAL, DEF, REF} print_mode;
    union {
        int var_no;  // For VARIABLE, TEMP, FUNCT;
        int label;   // For LABEL
        int value;  // For CONSTANT
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

struct expType_ {
    Operand op;
    Type type;
};

/* High-level Definitions */
void ir_Program(TreeNode_t* root, FILE* ir_file);
void ir_ExtDefList(TreeNode_t *root);
void ir_ExtDef(TreeNode_t *root);
void ir_ExtDecList(TreeNode_t *root, Type baseType);

/* Specifiers */
Type ir_Specifier(TreeNode_t *root);
Type ir_StructSpecifier(TreeNode_t* root);
char* ir_OptTag(TreeNode_t* root);
char* ir_Tag(TreeNode_t* root);

/* Declarators */
Symbol ir_VarDec(TreeNode_t* root, Type baseType, int size, int inStruct);
Symbol ir_FunDec(TreeNode_t* root, Type retType, Symbol sym);
FieldList ir_VarList(TreeNode_t* root);
FieldList ir_ParamDec(TreeNode_t* root);

/* Statements */
void ir_CompSt(TreeNode_t* root, Type retType);
void ir_StmtList(TreeNode_t* root, Type retType);


#endif

