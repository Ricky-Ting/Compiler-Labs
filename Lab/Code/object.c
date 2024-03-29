#include "object.h"
#include "ir.h"
#include "assert.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define IS_EQUAL(a,b) (strcmp(a, b) == 0)

static FILE* out = NULL;

int offset;
int args;


void ob_helper(char* s) {
    #ifdef IR_DEBUG
        fprintf(out, "%s\n", s);
    #endif
}


void init() {
    fprintf(out, ".data\n");
    fprintf(out, "_prompt: .asciiz \"Enter an integer:\"\n");
    fprintf(out, "_ret: .asciiz \"\\n\"\n");
    fprintf(out, ".globl main\n");
    fprintf(out, ".text\n");

    fprintf(out, "read:\n");
    fprintf(out, "\t li $v0, 4\n");
    fprintf(out, "\t la $a0, _prompt\n");
    fprintf(out, "\t syscall\n");
    fprintf(out, "\t li $v0, 5\n");
    fprintf(out, "\t syscall\n");
    fprintf(out, "\t jr $ra\n\n");

    fprintf(out, "write:\n");
    fprintf(out, "\t li $v0, 1\n");
    fprintf(out, "\t syscall\n");
    fprintf(out, "\t li $v0, 4\n");
    fprintf(out, "\t la $a0, _ret\n");
    fprintf(out, "\t syscall\n");
    fprintf(out, "\t move $v0, $0\n");
    fprintf(out, "\t jr $ra\n");


    for(int i=0; i<mxn; ++i) {
        v_off[i] = -1;
        t_off[i] = -1;
    }

}


int get_off(Operand op) {
    int v = op->u.var_no;
    if(op->kind == VARIABLE) {
        if(v_off[v] == -1) {
            v_off[v] = -offset;
            offset += 4;
            //fprintf(out, "\t addi $sp, $sp, -4\n");
            fprintf(out, "\t addi $sp, $fp, %d\n", -offset);
        }
        return v_off[v];
    } else if(op->kind == TEMP) {
        if(t_off[v] == -1) {
            t_off[v] = -offset;
            offset += 4;
            //fprintf(out, "\t addi $sp, $sp, -4\n");
            fprintf(out, "\t addi $sp, $fp, %d\n", -offset);
        }
        return t_off[v];
    } else {
        assert(0);
    }
}




void ob_LABELSET(InterCode code) {
    ob_helper("LABELSET");
    fprintf(out, "l%d :\n", code->u.label.op->u.label);
}

void ob_FUNCTION(InterCode code) {
    ob_helper("FUNCTION");
    assert(code->u.unary.op->kind == FUNCT);
    if(code->u.unary.op->u.var_no == 0) 
        fprintf(out, "main: \n");
    else 
        fprintf(out, "f%d: \n", code->u.unary.op->u.var_no);

    fprintf(out, "\t move $fp, $sp\n");
    fprintf(out, "\t sw $ra, 0($sp)\n");
    offset = 4;
    //fprintf(out, "\t addi $sp, $sp, -4\n");
    fprintf(out, "\t addi $sp, $fp, %d\n", -offset);
    args = 0;
}

void ob_ASSIGN(InterCode code) {
    ob_helper("ASSIGN");
    get_off(code->u.assign.left);

    Operand right = code->u.assign.right;
    if(right->kind == CONSTANT) {
        fprintf(out, "\t li $t0, %d\n", right->u.value);
    } else {
        fprintf(out, "\t lw $t0, %d($fp)\n", get_off(code->u.assign.right));
    }

    fprintf(out, "\t sw $t0, %d($fp)\n", get_off(code->u.assign.left));


}

void ob_ARI(InterCode code) {
    ob_helper("ARI");
    get_off(code->u.binop.result);
    Operand op1 = code->u.binop.op1;
    Operand op2 = code->u.binop.op2;
    if(op1->kind == CONSTANT) {
        fprintf(out, "\t li $t1, %d\n", op1->u.value);
    } else {
        fprintf(out, "\t lw $t1, %d($fp)\n", get_off(op1));
    }

    if(op2->kind == CONSTANT) {
        fprintf(out, "\t li $t2, %d\n", op2->u.value);
    } else {
        fprintf(out, "\t lw $t2, %d($fp)\n", get_off(op2));
    }

    switch(code->kind) {
    case ADD:
        fprintf(out, "\t add $t0, $t1, $t2\n");
        break;
    case SUB:
        fprintf(out, "\t sub $t0, $t1, $t2\n");
        break;
    case MUL:
        fprintf(out, "\t mul $t0, $t1, $t2\n");
        break;
    case DIV:
        fprintf(out, "\t div $t0, $t1, $t2\n");
        break;
    default:
        assert(0);
    }

    fprintf(out, "\t sw $t0, %d($fp)\n", get_off(code->u.binop.result));
    return;
}

void ob_ADDR(InterCode code) {
    ob_helper("ADDR");
    get_off(code->u.assign.left);
    fprintf(out, "\t li $t0, %d\n", get_off(code->u.assign.right));
    fprintf(out, "\t sw $t0, %d($fp)\n", get_off(code->u.assign.left));
}

void ob_DEREF(InterCode code) {
    //ob_op(code->u.assign.left);
    //fprintf(out, " := *");
    //ob_op(code->u.assign.right);
    //fprintf(out, "\n");
    ob_helper("DEREF");
    get_off(code->u.assign.left);
    fprintf(out, "\t lw $t1, %d($fp)\n", get_off(code->u.assign.right));
    fprintf(out, "\t add $t1, $t1, $fp\n");
    fprintf(out, "\t lw $t0, 0($t1)\n");
    fprintf(out, "\t sw $t0, %d($fp)\n", get_off(code->u.assign.left));

}

void ob_REF_ASSIGN(InterCode code) {
    //fprintf(out, "*");
    //ob_op(code->u.assign.left);
    //fprintf(out, " := ");
    //ob_op(code->u.assign.right);
    //fprintf(out, "\n");
    ob_helper("REF_ASSIGN");

    fprintf(out, "\t lw $t1, %d($fp)\n", get_off(code->u.assign.left));
    fprintf(out, "\t add $t1, $t1, $fp\n");
    fprintf(out, "\t lw $t0, %d($fp)\n", get_off(code->u.assign.right));
    fprintf(out, "\t sw $t0, 0($t1)\n");
}

void ob_GOTO(InterCode code) {
    ob_helper("GOTO");
    //fprintf(out, "GOTO l%d\n", code->u.label.op->u.label);
    fprintf(out, "\t j l%d\n", code->u.label.op->u.label);
}

void ob_CONDJMP(InterCode code) {
    //fprintf(out, "IF ");
    //ob_op(code->u.condjmp.op1);
    //fprintf(out, " %s ", code->u.condjmp.relop);
    //ob_op(code->u.condjmp.op2);
    //fprintf(out, " GOTO l%d\n", code->u.condjmp.target->u.label);
    ob_helper("CONDJMP");
    Operand op1 = code->u.condjmp.op1;
    Operand op2 = code->u.condjmp.op2;

    if(op1->kind == CONSTANT) {
        fprintf(out, "\t li $t1, %d\n", op1->u.value);
    } else {
        fprintf(out, "\t lw $t1, %d($fp)\n",get_off(op1));
    }

    if(op2->kind == CONSTANT) {
        fprintf(out, "\t li $t2, %d\n", op2->u.value);
    } else {
        fprintf(out, "\t lw $t2, %d($fp)\n",get_off(op2));
    }

    if(IS_EQUAL(code->u.condjmp.relop, "==")) {
        fprintf(out, "\t beq $t1, $t2, l%d\n", code->u.condjmp.target->u.label);
    } else if(IS_EQUAL(code->u.condjmp.relop, "!=")) {
        fprintf(out, "\t bne $t1, $t2, l%d\n", code->u.condjmp.target->u.label);
    } else if(IS_EQUAL(code->u.condjmp.relop, ">")) {
        fprintf(out, "\t bgt $t1, $t2, l%d\n", code->u.condjmp.target->u.label);
    } else if(IS_EQUAL(code->u.condjmp.relop, ">=")) {
        fprintf(out, "\t bge $t1, $t2, l%d\n", code->u.condjmp.target->u.label);
    } else if(IS_EQUAL(code->u.condjmp.relop, "<")) {
        fprintf(out, "\t blt $t1, $t2, l%d\n", code->u.condjmp.target->u.label);
    } else if(IS_EQUAL(code->u.condjmp.relop, "<=")) {
        fprintf(out, "\t ble $t1, $t2, l%d\n", code->u.condjmp.target->u.label);
    }

    return;
}

void ob_RETURN(InterCode code) {
    //fprintf(out, "RETURN ");
    //ob_op(code->u.unary.op);
    //fprintf(out, "\n");
    ob_helper("RETURN");

    fprintf(out, "\t lw $v0, %d($fp)\n", get_off(code->u.unary.op));
    fprintf(out, "\t lw $ra, 0($fp)\n");
    fprintf(out, "\t move $sp, $fp\n");
    fprintf(out, "\t lw $fp, 4($fp)\n");
    fprintf(out, "\t addi $sp, $sp, %d\n", args*4 + 4);
    fprintf(out, "\t jr $ra\n");
}

void ob_DEC(InterCode code) {
    //fprintf(out, "DEC ");
    //ob_op(code->u.dec.op);
    //fprintf(out, " %d\n", code->u.dec.size);
    ob_helper("DEC");
    int v = code->u.dec.op->u.var_no;

    v_off[v] = -offset - code->u.dec.size + 4;
    fprintf(out, "\t addi $sp, $sp, %d\n", -code->u.dec.size);
    offset += code->u.dec.size;

}

void ob_ARG(InterCode code) {
    ob_helper("ARG");
    //fprintf(out, "ARG ");
    //ob_op(code->u.unary.op);
    //fprintf(out, "\n");

    Operand op = code->u.unary.op;

    if(op->kind == CONSTANT) {
        fprintf(out, "\t li $t1, %d\n", op->u.value);
    } else {
        fprintf(out, "\t lw $t1, %d($fp)\n", get_off(code->u.unary.op));
    }

    fprintf(out, "\t sw $t1, 0($sp)\n");
    fprintf(out, "\t addi $sp, $sp, -4\n");

    
}

void ob_CALL(InterCode code) {
    ob_helper("CALL");
    //ob_op(code->u.assign.left);
    //assert(code->u.assign.right->kind == FUNCT);
    //if(code->u.assign.right->u.var_no == 0) {
    //    fprintf(out, " := CALL main\n");
    //} else {
    //    fprintf(out, " := CALL f%d\n", code->u.assign.right->u.var_no);
    //}

    fprintf(out, "\t sw $fp, 0($sp)\n");
    fprintf(out, "\t addi $sp, $sp, -4\n");

    if(code->u.assign.right->u.var_no == 0) {
        fprintf(out, "\t jal main\n");
    } else {
        fprintf(out, "\t jal f%d\n", code->u.assign.right->u.var_no);
    }
    
    get_off(code->u.assign.left);

    fprintf(out, "\t sw $v0, %d($fp)\n", get_off(code->u.assign.left));
    

}

void ob_PARAM(InterCode code) {
    ob_helper("PARAM");
    //fprintf(out, "PARAM ");
    //ob_op(code->u.unary.op);
    //fprintf(out, "\n");

    args++;
    int v = code->u.unary.op->u.var_no;
    v_off[v] = args*4 + 4;
}

void ob_READ(InterCode code) {
    ob_helper("READ");
    fprintf(out, "\t jal read\n");

    get_off(code->u.unary.op);

    get_off(code->u.unary.op);
    fprintf(out, "\t sw $v0, %d($fp)\n", get_off(code->u.unary.op));
}

void ob_WRITE(InterCode code) {
    ob_helper("WRITE");
    get_off(code->u.unary.op);
    fprintf(out, "\t lw $a0, %d($fp)\n", get_off(code->u.unary.op));

    fprintf(out, "\t jal write\n");
}

void ob_IR(InterCodes head, InterCodes tail, FILE* file_ir) {
    InterCodes cur = head;
    out = file_ir;
    //out = stderr;
    init();
    while(cur!= NULL) {
        switch(cur->code->kind){
        case LABELSET:
            ob_LABELSET(cur->code);
            break;
        case FUNCTION:
            ob_FUNCTION(cur->code);
            break;
        case ASSIGN:
            ob_ASSIGN(cur->code);
            break;
        case ADD:
        case SUB:
        case MUL:
        case DIV:
            ob_ARI(cur->code);
            break;
        case ADDR:
            ob_ADDR(cur->code);
            break;
        case DEREF:
            ob_DEREF(cur->code);
            break;
        case REF_ASSIGN:
            ob_REF_ASSIGN(cur->code);
            break;
        case GOTO:
            ob_GOTO(cur->code);
            break;
        case CONDJMP:
            ob_CONDJMP(cur->code);
            break;
        case RETURN:
            ob_RETURN(cur->code);
            break;
        case DEC:
            ob_DEC(cur->code);
            break;
        case ARG:
            ob_ARG(cur->code);
            break;
        case CALL:
            ob_CALL(cur->code);
            break;
        case PARAM:
            ob_PARAM(cur->code);
            break;
        case READ:
            ob_READ(cur->code);
            break;
        case WRITE:
            ob_WRITE(cur->code);
            break;
        default:
            assert(0);
        }
        cur = cur->next;
    }
}