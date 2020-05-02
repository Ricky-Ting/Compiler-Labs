#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include "Tree.h"
#include "symbol.h"
#include "ir.h"

Type_t type_INT, type_FLOAT;

/* 为匿名结构体生成名字 */
char random_name[55];  
int pos;

int temp_counter = 1; // 给临时变量编号
int label_counter = 1; // 给label编号

InterCodes head, tail; // 整个代码块的head和tail

Operand_t OP_ZERO, OP_ONE; // 常量0和1的operand

void helper(TreeNode_t* root) {
    //fprintf(stderr, "In %s\n", root->Tree_token);
}

int min(int a, int b) {
    return (a<b)?a:b;
}

char* get_name() {
    if(random_name[pos] == 'Z') {
        pos++;
        random_name[pos] = 'A';
        random_name[pos+1] = '\0';
    } else {
        random_name[pos]++;
    }
    return random_name;
}

Operand get_temp() {
    Operand op = myAlloc(sizeof(Operand_t));
    op->kind = TEMP;
    op->mode = VALUE;
    op->print_mode = NORMAL;
    op->u.var_no = temp_counter++;
    return op;
}

Operand get_label() {
    Operand op = myAlloc(sizeof(Operand_t));
    op->kind = LABEL;
    op->u.label = label_counter++;
    return op;
}

Operand get_op(int id) {
    Operand op = myAlloc(sizeof(Operand_t));
    op->kind = VARIABLE;
    op->mode = VALUE;
    op->print_mode = NORMAL;
    op->u.var_no = id;
    return op;
}

Operand get_constant(int value) {
    Operand op = myAlloc(sizeof(Operand_t));
    op->kind = CONSTANT;
    op->mode = VALUE;
    op->print_mode = NORMAL;
    op->u.value = value;
    return op;
}

void append_codes(InterCodes codes) {
    if(codes == NULL) return;
    if(head == NULL) {
        head = codes;
    } else {
        tail->next = codes;
    }
    InterCodes cur = codes;
    while(cur->next != NULL) 
        cur = cur->next;
    tail = cur;
    return;
}

void append_code(InterCode code) {
    InterCodes codes = myAlloc(sizeof(InterCodes_t));
    codes->prev = codes->next = NULL;
    codes->code = code;
    append_codes(codes);
}

int get_size(Type type) {
    assert(type != NULL);
    if(type->kind == BASIC)
        return 4;
    else if(type->kind == ARRAY)
        return type->u.array.totalsize;
    else if(type->kind == STRUCTURE)  {
        if(type->u.structure == NULL)
            return 0;
        else 
            return type->u.structure->totalsize;
    } else {
        assert(0);
    }
}


void add_read_write() {
    Symbol readfunc = myAlloc(sizeof(Symbol_t));
    readfunc->prev = readfunc->next = readfunc->area_next = readfunc->area_prev;
    snprintf(readfunc->name, 55, "read");
    Type readtype = myAlloc(sizeof(Type_t));
    readtype->kind = FUNC;
    readtype->u.func.params = NULL;
    readtype->u.func.ret = &type_INT;
    readfunc->type = readtype;

    Symbol writefunc = myAlloc(sizeof(Symbol_t));
    writefunc->prev = writefunc->next = writefunc->area_next = writefunc->area_prev;
    snprintf(writefunc->name, 55, "write");
    Type writetype = myAlloc(sizeof(Type_t));
    writetype->kind = FUNC;
    writetype->u.func.ret = NULL;
    writefunc->type = writetype;

    FieldList field = myAlloc(sizeof(FieldList_t));
    snprintf(field->name, 5, "x");
    field->type = &type_INT;
    field->tail = NULL;

    writetype->u.func.params = field;

    insertSymbol(writefunc);
    insertSymbol(readfunc);

    return;
}

void ir_init() {

    // For name
    random_name[0] = '#';
    random_name[1] = 'A';
    random_name[2] = '\0';
    pos = 1;

    initTables();
    
    head = tail = NULL;

    OP_ZERO.kind = CONSTANT;
    OP_ZERO.mode = VALUE;
    OP_ZERO.print_mode = NORMAL;
    OP_ZERO.u.value = 0;

    OP_ONE.kind = CONSTANT;
    OP_ONE.mode = VALUE;
    OP_ONE.print_mode = NORMAL;
    OP_ONE.u.value = 1;

    // Add read and write
    add_read_write();
    return;
}

/* High-level Definitions */
void ir_Program(TreeNode_t* root, FILE* ir_file) {
    ir_init();
    helper(root);
    /*
    * Program -> ExtDefList
    */
    if(root == NULL)
        return;
    if(root->num_child == 0)
        return;
    if(root->Tree_child[0] != NULL)
        ir_ExtDefList(root->Tree_child[0]);

    printIR(head, tail, ir_file);
    return;
}


void ir_ExtDefList(TreeNode_t *root) {
    helper(root);
    /*
    * ExtDefList -> ExtDef ExtDefList
    * ExtDefList -> e (not reach)
    */

    assert(root->num_child == 2);
    
    assert(root->Tree_child[0] != NULL);
    ir_ExtDef(root->Tree_child[0]);

    if(root->Tree_child[1] != NULL) {
        ir_ExtDefList(root->Tree_child[1]);
    }
}

void ir_ExtDef(TreeNode_t *root) {
    helper(root);
    /*
    * ExtDef -> Specifier ExtDecList SEMI 
    * ExtDef -> Specifier SEMI
    * ExtDef -> Specifier FunDec CompSt
    * ExtDef -> Specifier FunDec SEMI
    */

    assert(root->num_child == 2 || root->num_child == 3);
    
    /* Get Specifier */
    assert(root->Tree_child[0] != NULL);
    Type type = ir_Specifier(root->Tree_child[0]);

    /* ExtDecList SEMI FunDec 都不会产生空 */
    assert(root->Tree_child[1] != NULL);
    if(IS_EQUAL(root->Tree_child[1]->Tree_token, "SEMI")) {
        // ExtDef -> Specifier SEMI
        return;
    } else if(IS_EQUAL(root->Tree_child[1]->Tree_token, "ExtDecList")) {
        // ExtDef -> Specifier ExtDecList SEMI
        ir_ExtDecList(root->Tree_child[1], type);
        return;
    } else if(IS_EQUAL(root->Tree_child[1]->Tree_token, "FunDec")) {
        // ExtDef -> Specifier FunDec Compst
    
        // 遇到函数 压栈
        Type functype = myAlloc(sizeof(Type_t));
        functype->kind = FUNC;
        functype->u.func.ret = type;
        functype->u.func.params = NULL; // 暂时为NULL
        Symbol sym = myAlloc(sizeof(Symbol_t));
        // 获取函数名
        snprintf(sym->name, 55, "%s", root->Tree_child[1]->Tree_child[0]->Tree_val);
        sym->type = functype;

        // 创建符号
        assert(root->Tree_child[2] != NULL);

        if(findSymbol(sym->name) == NULL) {
            insertSymbol(sym);
            InterCode code = myAlloc(sizeof(InterCode_t));
            code->kind = FUNCTION;
            Operand op = myAlloc(sizeof(Operand_t));
            op->kind = FUNCT;
            if(IS_EQUAL(sym->name, "main")) {
                op->u.var_no = sym->var_no = 0;
            } else {
                op->u.var_no = sym->var_no;
            }
            code->u.unary.op = op;
            append_code(code);
        } else {
            // 只会出现一次函数定义
            // shouldn't reach here!!!
            assert(0);
        }

        if(IS_EQUAL(root->Tree_child[2]->Tree_token, "CompSt")) {
            stack_push();
            ir_FunDec(root->Tree_child[1], type, sym);
            ir_CompSt(root->Tree_child[2], type);
            stack_pop();
        } else {
            // Shouldn't reach here!!!
            assert(0);
        }
        return;
    }
}

void ir_ExtDecList(TreeNode_t *root, Type baseType) {
    helper(root);
    /*
    * ExtDecList -> VarDec
    * ExtDecList -> VarDec COMMA ExtDecList
    */
   
   assert(root->num_child == 1 || root->num_child == 3);
   
   assert(root->Tree_child[0] != NULL);
   ir_VarDec(root->Tree_child[0], baseType, 0, 0); 

   if(root->num_child == 1) 
       return;
       
    assert(root->Tree_child[2] != NULL);
    ir_ExtDecList(root->Tree_child[2], baseType);
}

/* Specifiers */
Type ir_Specifier(TreeNode_t *root) {
    helper(root);
    /*
        Specifier -> TYPE
        Specifier -> StructSpecifier
    */
    assert(root->num_child == 1);
    assert(root->Tree_child[0] != NULL);
    if(IS_EQUAL(root->Tree_child[0]->Tree_token, "TYPE")) {
        int basic_type = ir_TYPE(root->Tree_child[0]);
        if(basic_type == TYPE_INT) 
            return &type_INT;
        else 
            return &type_FLOAT;
    } else if(IS_EQUAL(root->Tree_child[0]->Tree_token, "StructSpecifier")) {
        return ir_StructSpecifier(root->Tree_child[0]);
    } else {
        // Shouldn't reach here!!!
        assert(0);
    }
}

Type ir_StructSpecifier(TreeNode_t* root) {
    helper(root);
    /* 
    * StructSpecifier -> STRUCT OptTag LC DefList RC
    * StructSpecifier -> STRUCT Tag
    */
    assert(root->num_child==2 || root->num_child==5);
    if(root->num_child == 2) {
        assert(root->Tree_child[1] != NULL);     
        char *name = ir_Tag(root->Tree_child[1]);
        Symbol sym = findType(name);
        if(sym == NULL) {
            // Shouldn't reach here !!!
            assert(0);
        }
        return sym->type;        
    } else {
        stack_push();
        Type type = myAlloc(sizeof(Type_t));
        type->kind = STRUCTURE;
        type->u.structure = NULL;

        Symbol sym = myAlloc(sizeof(Symbol_t));
        sym->type = type;

        if(root->Tree_child[1] == NULL) {
            // 匿名结构体指定一个名字
            strncpy(sym->name, get_name(), 55);
        } else {
            strncpy(sym->name, ir_OptTag(root->Tree_child[1]), 55);
        }

        if(root->Tree_child[3] != NULL) {
            type->u.structure = ir_DefList(root->Tree_child[3], 1, 0);
            
            FieldList cur = type->u.structure;
            assert(cur != NULL); //TODO
            while(cur->tail != NULL)
                cur = cur->tail;
            type->u.structure->totalsize = cur->offset + cur->size;
        }

        if(findSymbol(sym->name) != NULL || findType(sym->name) != NULL) {
            // shouldn't reach here!!!
            assert(0);
        } else {
            insertType(sym);
            stack_pop();
            return type;
        }
    }
}

char* ir_OptTag(TreeNode_t* root) {
    helper(root);
    /*
        OptTag -> ID
        OptTag -> e
    */
    assert(root->num_child == 1);
    return root->Tree_child[0]->Tree_val;
}

char* ir_Tag(TreeNode_t* root) {
    helper(root);
    // Tag -> ID
    return root->Tree_child[0]->Tree_val;
}

/* Declarators */
Symbol ir_VarDec(TreeNode_t* root, Type baseType, int size, int inStruct) {
    helper(root);
    /* 
    * VarDec -> ID
    * VarDec -> VarDec LB INT RB
    */

    assert(root->num_child == 1 || root->num_child == 4);
    
    if(root->num_child == 1) {
        assert(root->Tree_child[0] != NULL);
        Type type = baseType;
        
        Symbol sym = myAlloc(sizeof(Symbol_t));
        strncpy(sym->name, ir_ID(root->Tree_child[0]), 55);
        sym->type = type;
        sym->prev = sym->next = sym->area_prev = sym->area_next = NULL;
        sym->mode = VALUE;
        insertSymbol(sym);

        if(inStruct == 0 && baseType->kind != BASIC) {
            int sz = get_size(baseType);

            InterCode code = myAlloc(sizeof(InterCode_t));
            code->kind = DEC;
            code->u.dec.op = get_op(sym->var_no);
            code->u.dec.size = sz;
            append_code(code);
        } else if(inStruct == 2) {
            InterCode code = myAlloc(sizeof(InterCode_t));
            //printf("HERE\n");
            code->kind = PARAM;
            code->u.unary.op = get_op(sym->var_no);
            if(sym->type->kind != BASIC)
                sym->mode = ADDRESS;
            append_code(code);
        }

        return sym;
    } else {
        assert(root->Tree_child[0] != NULL);
        Type newType = myAlloc(sizeof(Type_t));
        newType->kind = ARRAY;
        newType->u.array.size = root->Tree_child[2]->val_UINT;
        newType->u.array.elem = baseType;
        newType->u.array.totalsize = newType->u.array.size * get_size(baseType);

        return ir_VarDec(root->Tree_child[0], newType, size+1, inStruct);
    }
}

Symbol ir_FunDec(TreeNode_t* root, Type retType, Symbol sym) {
    helper(root);
    /* 
    * FunDec -> ID LP VarList RP
    * FunDec -> ID LP RP
    */

    assert(root->num_child == 3 || root->num_child == 4);

    if(root->num_child == 4) {
        assert(root->Tree_child[2] != NULL);
        sym->type->u.func.params = ir_VarList(root->Tree_child[2]);

        FieldList field = sym->type->u.func.params;

        //TODO
        /*
        while(field != NULL) {
            if(field->type->kind != BASIC) {
                Symbol fieldsym = findSymbol(field->name);
                fieldsym->mode = ADDRESS;
            }
            field = field->tail;
        }
        */

    }
    return sym;
}


FieldList ir_VarList(TreeNode_t* root) {
    helper(root);
    /*
    * VarList -> ParamDec COMMA VarList
    * VarList -> ParamDec
    */

    assert(root->num_child == 1 || root->num_child == 3);

    assert(root->Tree_child[0] != NULL);

    FieldList field = ir_ParamDec(root->Tree_child[0]);

    //TODO 可以优化
    if(root->num_child == 3) {
        assert(root->Tree_child[2] != NULL);
        FieldList next_field = ir_VarList(root->Tree_child[2]);
        FieldList cur = field;
        while(cur->tail != NULL)
            cur = cur->tail;
        cur->tail = next_field;
    }    
    return field;
}

FieldList ir_ParamDec(TreeNode_t* root) {
    helper(root);
    /*
    * ParamDec -> Specifier VarDec 
    */

    assert(root->num_child == 2);
    assert(root->Tree_child[0] != NULL && root->Tree_child[1] != NULL);

    Type type = ir_Specifier(root->Tree_child[0]);
    Symbol sym = ir_VarDec(root->Tree_child[1], type, 0, 2);

    FieldList field = myAlloc(sizeof(FieldList_t));
    strncpy(field->name, sym->name, 55);
    field->type = sym->type;
    field->tail = NULL;
    
    return field;
}

/* Statements */

void ir_CompSt(TreeNode_t* root, Type retType) {
    helper(root);
    /*
    * CompSt -> LC DefList StmtList RC
    */

    assert(root->num_child == 4);
    if(root->Tree_child[1] != NULL) {
        ir_DefList(root->Tree_child[1], 0, 0);
    }

    if(root->Tree_child[2] != NULL) {
        ir_StmtList(root->Tree_child[2], retType);
    }
    return;
}

void ir_StmtList(TreeNode_t* root, Type retType) {
    helper(root);
    /*
    * StmtList -> Stmt StmtList
    */

    assert(root->num_child == 2);
    assert(root->Tree_child[0] != NULL);
    ir_Stmt(root->Tree_child[0], retType);
    if(root->Tree_child[1] != NULL)
        ir_StmtList(root->Tree_child[1], retType);
    return;
}


void ir_Stmt(TreeNode_t* root, Type retType) {
    helper(root);
    /* 
    * Stmt -> Exp SEMI
    * Stmt -> CompSt
    * Stmt -> RETURN Exp SEMI
    * Stmt -> IF LP Exp RP Stmt
    * Stmt -> IF LP Exp RP Stmt ELSE Stmt
    * Stmt -> WHILE LP Exp RP Stmt
    */

    if(root->num_child == 1) {
        // Stmt -> CompSt
        assert(root->Tree_child[0] != NULL);
        stack_push();
        ir_CompSt(root->Tree_child[0], retType);
        stack_pop();
        return;
    }

    if(root->num_child == 2) {
        // Stmt -> Exp SEMI;
        assert(root->Tree_child[0] != NULL);
        ir_Exp(root->Tree_child[0], 0);
        return;
    }

    if(root->num_child == 3) {
        // Stmt -> RETURN Exp SEMI
        assert(root->Tree_child[1] != NULL);

        // 返回值一定是int

        Operand t1 = call_Exp(root->Tree_child[1], 1).op;
        
        InterCode code = myAlloc(sizeof(InterCode_t));
        code->kind = RETURN;
        code->u.unary.op = t1;
        append_code(code);
        return;
    }

    if(root->num_child == 5) {
        // Stmt -> IF LP Exp RP Stmt
        // Stmt -> WHILE LP Exp RP Stmt
        assert(root->Tree_child[2] != NULL);
        assert(root->Tree_child[4] != NULL);

        if(IS_EQUAL(root->Tree_child[0]->Tree_token, "IF")) {
            Operand label1 = get_label();
            Operand label2 = get_label();

            ir_Cond(root->Tree_child[2], label1, label2); // TODO

            InterCode l1 = myAlloc(sizeof(InterCode_t));
            l1->kind = LABELSET;
            l1->u.label.op = label1;
            append_code(l1);

            ir_Stmt(root->Tree_child[4], retType);

            InterCode l2 = myAlloc(sizeof(InterCode_t));
            l2->kind = LABELSET;
            l2->u.label.op = label2;
            append_code(l2);
        } else {
            Operand label1 = get_label();
            Operand label2 = get_label();
            Operand label3 = get_label();

            InterCode l1 = myAlloc(sizeof(InterCode_t));
            l1->kind = LABELSET;
            l1->u.label.op = label1;
            append_code(l1);

            ir_Cond(root->Tree_child[2], label2, label3); // TODO

            InterCode l2 = myAlloc(sizeof(InterCode_t));
            l2->kind = LABELSET;
            l2->u.label.op = label2;
            append_code(l2);

            ir_Stmt(root->Tree_child[4], retType);

            InterCode g1 = myAlloc(sizeof(InterCode_t));
            g1->kind = GOTO;
            g1->u.label.op = label1;
            append_code(g1);

            InterCode l3 = myAlloc(sizeof(InterCode_t));
            l3->kind = LABELSET;
            l3->u.label.op = label3;
            append_code(l3);
        }

        return;
    }

    if(root->num_child == 7) {
        // Stmt -> IF LP Exp RP Stmt ELSE Stmt
        assert(root->Tree_child[2] != NULL);
        assert(root->Tree_child[4] != NULL);
        assert(root->Tree_child[6] != NULL);

        Operand label1 = get_label();
        Operand label2 = get_label();
        Operand label3 = get_label();

        ir_Cond(root->Tree_child[2], label1, label2); // TODO

        InterCode l1 = myAlloc(sizeof(InterCode_t));
        l1->kind = LABELSET;
        l1->u.label.op = label1;
        append_code(l1);

        ir_Stmt(root->Tree_child[4], retType);

        InterCode g1 = myAlloc(sizeof(InterCode_t));
        g1->kind = GOTO;
        g1->u.label.op = label3;
        append_code(g1);


        InterCode l2 = myAlloc(sizeof(InterCode_t));
        l2->kind = LABELSET;
        l2->u.label.op = label2;
        append_code(l2);

        ir_Stmt(root->Tree_child[6], retType);

        InterCode l3 = myAlloc(sizeof(InterCode_t));
        l3->kind = LABELSET;
        l3->u.label.op = label3;
        append_code(l3);

        return;
    }

    // Should't reach Here!!!
    assert(0);

}


/* Local Definitions */
FieldList ir_DefList(TreeNode_t* root, int inStruct, int offset) {
    helper(root);
    /* 
    * DefList -> Def DefList 
    * DefList -> e
    */

    assert(root->num_child == 2);

    if(inStruct == 1) {
        FieldList field = ir_Def(root->Tree_child[0], inStruct, offset); 
        if(root->Tree_child[1] != NULL) {
            FieldList cur = field;
            while(cur->tail != NULL)
                cur = cur->tail;
            offset = cur->offset + cur->size;
            FieldList next_field = ir_DefList(root->Tree_child[1], inStruct, offset);

            cur->tail = next_field;
        }
        return field;
    } else {
        ir_Def(root->Tree_child[0], inStruct, offset);
        if(root->Tree_child[1] != NULL) {
            ir_DefList(root->Tree_child[1], inStruct, offset);
        }
        return NULL;
    }
}

FieldList ir_Def(TreeNode_t* root, int inStruct, int offset) {
    helper(root);
    /*
    * Def -> Specifier DecList SEMI
    */

    assert(root->num_child == 3);
   
    assert(root->Tree_child[0] != NULL);
    Type type = ir_Specifier(root->Tree_child[0]);

    assert(root->Tree_child[1] != NULL);

    return ir_DecList(root->Tree_child[1], type, inStruct, offset);
}