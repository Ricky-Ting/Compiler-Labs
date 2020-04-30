#include "ir.h"
#include <assert.h>
#include <stdio.h>


void print_op(Operand op) {
    if(op->kind == VARIABLE) {
        printf("v%d", op->u.var_no);
    } else if(op->kind == TEMP) {
        printf("t%d", op->u.var_no);
    } else if(op->kind == CONSTANT) {
        printf("#%d", op->u.value);
    } else {
        assert(0);
    }
}


void print_LABELSET(InterCode code) {
    printf("LABEL l%d :\n", code->u.label.op->u.label);
}

void print_FUNCTION(InterCode code) {
    assert(code->u.unary.op->kind == FUNCT);
    printf("FUNCTION f%d :\n", code->u.unary.op->u.var_no);
}

void print_ASSIGN(InterCode code) {
    print_op(code->u.assign.left);
    printf(" := ");
    print_op(code->u.assign.right);
    printf("\n");
}

void print_ARI(InterCode code) {
    print_op(code->u.binop.result);
    printf(" := ");
    print_op(code->u.binop.op1);
    switch(code->kind) {
    case ADD:
        printf(" + ");
        break;
    case SUB:
        printf(" - ");
        break;
    case MUL:
        printf(" * ");
        break;
    case DIV:
        printf(" / ");
        break;
    default:
        assert(0);
    }
    print_op(code->u.binop.op2);
    printf("\n");
}

void print_ADDR(InterCode code) {
    print_op(code->u.assign.left);
    printf(" := &");
    print_op(code->u.assign.right);
    printf("\n");
}

void print_DEREF(InterCode code) {
    print_op(code->u.assign.left);
    printf(" := *");
    print_op(code->u.assign.right);
    printf("\n");
}

void print_REF_ASSIGN(InterCode code) {
    printf("*");
    print_op(code->u.assign.left);
    printf(" := ");
    print_op(code->u.assign.right);
    printf("\n");
}

void print_GOTO(InterCode code) {
    printf("GOTO l%d\n", code->u.label.op->u.label);
}

void print_CONDJMP(InterCode code) {
    printf("IF ");
    print_op(code->u.condjmp.op1);
    printf(" %s ", code->u.condjmp.relop);
    print_op(code->u.condjmp.op2);
    printf(" GOTO l%d\n", code->u.condjmp.target->u.label);
}

void print_RETURN(InterCode code) {
    printf("RETURN ");
    print_op(code->u.unary.op);
    printf("\n");
}

void print_DEC(InterCode code) {
    printf("DEC ");
    print_op(code->u.dec.op);
    printf(" %d\n", code->u.dec.size);
}

void print_ARG(InterCode code) {
    printf("ARG ");
    print_op(code->u.unary.op);
    printf("\n");
}

void print_CALL(InterCode code) {
    print_op(code->u.assign.left);
    assert(code->u.assign.right->kind == FUNCT);
    printf(" := CALL f%d\n", code->u.assign.right->u.var_no);
}

void print_PARAM(InterCode code) {
    printf("PARAM ");
    print_op(code->u.unary.op);
    printf("\n");
}

void print_READ(InterCode code) {
    printf("READ ");
    print_op(code->u.unary.op);
    printf("\n");
}

void print_WRITE(InterCode code) {
    printf("WRITE ");
    print_op(code->u.unary.op);
    printf("\n");
}


void printIR(InterCodes head, InterCodes tail) {
    InterCodes cur = head;
    while(cur!= NULL) {
        switch(cur->code->kind){
        case LABELSET:
            print_LABELSET(cur->code);
            break;
        case FUNCTION:
            print_FUNCTION(cur->code);
            break;
        case ASSIGN:
            print_ASSIGN(cur->code);
            break;
        case ADD:
        case SUB:
        case MUL:
        case DIV:
            print_ARI(cur->code);
            break;
        case ADDR:
            print_ADDR(cur->code);
            break;
        case DEREF:
            print_DEREF(cur->code);
            break;
        case REF_ASSIGN:
            print_REF_ASSIGN(cur->code);
            break;
        case GOTO:
            print_GOTO(cur->code);
            break;
        case CONDJMP:
            print_CONDJMP(cur->code);
            break;
        case RETURN:
            print_RETURN(cur->code);
            break;
        case DEC:
            print_DEC(cur->code);
            break;
        case ARG:
            print_ARG(cur->code);
            break;
        case CALL:
            print_CALL(cur->code);
            break;
        case PARAM:
            print_PARAM(cur->code);
            break;
        case READ:
            print_READ(cur->code);
            break;
        case WRITE:
            print_WRITE(cur->code);
            break;
        default:
            assert(0);
        }
        cur = cur->next;
    }
}