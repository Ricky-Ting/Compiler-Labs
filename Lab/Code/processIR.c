#include "ir.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


FILE* out = NULL;

/*
void print_op(Operand op) {
    assert(op != NULL);
    if(op->kind == VARIABLE) {
        fprintf(out, "v%d", op->u.var_no);
    } else if(op->kind == TEMP) {
        fprintf(out, "t%d", op->u.var_no);
    } else if(op->kind == CONSTANT) {
        fprintf(out, "#%d", op->u.value);
    } else {
        assert(0);
    }
}
*/

void print_op(Operand op) {
    assert(op != NULL);
    if(op->kind == VARIABLE) {
        if(op->print_mode == NORMAL) 
            fprintf(out, "v%d", op->u.var_no);
        else if(op->print_mode ==  REF) 
            fprintf(out, "&v%d", op->u.var_no);
        else 
            assert(0);
    } else if(op->kind == TEMP) {
        if(op->print_mode == NORMAL) 
            fprintf(out, "t%d", op->u.var_no);
        else if(op->print_mode == DEF) 
            fprintf(out, "*t%d", op->u.var_no);
    } else if(op->kind == CONSTANT) {
        fprintf(out, "#%d", op->u.value);
    } else {
        assert(0);
    }
}


void print_LABELSET(InterCode code) {
    fprintf(out, "LABEL l%d :\n", code->u.label.op->u.label);
}

void print_FUNCTION(InterCode code) {
    assert(code->u.unary.op->kind == FUNCT);
    if(code->u.unary.op->u.var_no == 0) 
        fprintf(out, "FUNCTION main :\n");
    else 
        fprintf(out, "FUNCTION f%d :\n", code->u.unary.op->u.var_no);
}

void print_ASSIGN(InterCode code) {
    // TODO
    if(code->u.assign.left->print_mode == DEF) {
        assert(0);
    }

    print_op(code->u.assign.left);
    fprintf(out, " := ");
    print_op(code->u.assign.right);
    fprintf(out,"\n");
}

void print_ARI(InterCode code) {
    print_op(code->u.binop.result);
    fprintf(out," := ");
    print_op(code->u.binop.op1);
    switch(code->kind) {
    case ADD:
        fprintf(out, " + ");
        break;
    case SUB:
        fprintf(out, " - ");
        break;
    case MUL:
        fprintf(out, " * ");
        break;
    case DIV:
        fprintf(out, " / ");
        break;
    default:
        assert(0);
    }
    print_op(code->u.binop.op2);
    fprintf(out, "\n");
}

void print_ADDR(InterCode code) {
    assert(0);
    print_op(code->u.assign.left);
    fprintf(out, " := &");
    print_op(code->u.assign.right);
    fprintf(out, "\n");
}

void print_DEREF(InterCode code) {
    assert(0);
    print_op(code->u.assign.left);
    fprintf(out, " := *");
    print_op(code->u.assign.right);
    fprintf(out, "\n");
}

void print_REF_ASSIGN(InterCode code) {
    assert(0);
    fprintf(out, "*");
    print_op(code->u.assign.left);
    fprintf(out, " := ");
    print_op(code->u.assign.right);
    fprintf(out, "\n");
}

void print_GOTO(InterCode code) {
    fprintf(out, "GOTO l%d\n", code->u.label.op->u.label);
}

void print_CONDJMP(InterCode code) {
    fprintf(out, "IF ");
    print_op(code->u.condjmp.op1);
    fprintf(out, " %s ", code->u.condjmp.relop);
    print_op(code->u.condjmp.op2);
    fprintf(out, " GOTO l%d\n", code->u.condjmp.target->u.label);
}

void print_RETURN(InterCode code) {
    fprintf(out, "RETURN ");
    print_op(code->u.unary.op);
    fprintf(out, "\n");
}

void print_DEC(InterCode code) {
    fprintf(out, "DEC ");
    print_op(code->u.dec.op);
    fprintf(out, " %d\n", code->u.dec.size);
}

void print_ARG(InterCode code) {
    fprintf(out, "ARG ");
    print_op(code->u.unary.op);
    fprintf(out, "\n");
}

void print_CALL(InterCode code) {
    print_op(code->u.assign.left);
    assert(code->u.assign.right->kind == FUNCT);
    if(code->u.assign.right->u.var_no == 0) {
        fprintf(out, " := CALL main\n");
    } else {
        fprintf(out, " := CALL f%d\n", code->u.assign.right->u.var_no);
    }
}

void print_PARAM(InterCode code) {
    fprintf(out, "PARAM ");
    print_op(code->u.unary.op);
    fprintf(out, "\n");
}

void print_READ(InterCode code) {
    fprintf(out, "READ ");
    print_op(code->u.unary.op);
    fprintf(out, "\n");
}

void print_WRITE(InterCode code) {
    fprintf(out, "WRITE ");
    print_op(code->u.unary.op);
    fprintf(out, "\n");
}

void printIR(InterCodes head, InterCodes tail, FILE* file_ir) {
    InterCodes cur = head;
    out = file_ir;
    //out = stderr;
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