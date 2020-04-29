#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "Tree.h"
#include "symbol.h"
#include "ir.h"

//Type_t type_INT, type_FLOAT;

char random_name[55];
int pos;

int temp_counter = 0;
int label_counter = 0;

InterCodes head, tail; // 整个代码块的head和tail

Operand_t OP_ZERO, OP_ONE;

void helper(TreeNode_t* root) {
    //fprintf(stderr, "In %s\n", root->Tree_token);
}

int _same_type(Type a, Type b) {
    if(a==b)
        return 1;

    if(a == NULL && b == NULL)
        return 1;
    if(a == NULL || b == NULL) 
        return 0;
    
    if(a->kind != b->kind ) 
        return 0;
    
    if(a->kind == BASIC) 
        return (a->u.basic == b->u.basic);
    
    if(a->kind == ARRAY) 
        return _same_type(a->u.array.elem, b->u.array.elem);
    
    if(a->kind == STRUCTURE) {
        FieldList cur1 = a->u.structure;
        FieldList cur2 = b->u.structure;
        while(cur1 != NULL && cur2 != NULL) {
            if(!_same_type(cur1->type, cur2->type)) 
                return 0;
            cur1 = cur1->tail;
            cur2 = cur2->tail;
        }
        if(cur1 != NULL || cur2 != NULL)
            return 0;
        return 1;
    }

    if(a->kind == FUNC) {
        if(!_same_type(a->u.func.ret, b->u.func.ret))
            return 0;
        FieldList cur1 = a->u.func.params;
        FieldList cur2 = b->u.func.params;
        while(cur1 != NULL && cur2 != NULL) {
            if(!_same_type(cur1->type, cur2->type)) 
                return 0;
            cur1 = cur1->tail;
            cur2 = cur2->tail;
        }
        if(cur1 != NULL || cur2 != NULL)
            return 0;
        return 1;
    }

    // Shound't reach here!!!
    assert(0);    
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
    op->u.var_no = temp_counter++;
    return op;
}

Operand get_label() {
    Operand op = myAlloc(sizeof(Operand_t));
    op->kind = LABEL;
    op->u.label = label_counter++;
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

void ir_init() {

    random_name[0] = '#';
    random_name[1] = 'A';
    random_name[2] = '\0';
    pos = 1;

    initTables();
    
    head = tail = NULL;

    OP_ZERO.kind = CONSTANT;
    OP_ZERO.u.value = 0;

    OP_ONE.kind = CONSTANT;
    OP_ONE.u.value = 1;

    // Add read and write
    return;
}


/* High-level Definitions */
void ir_Program(TreeNode_t* root) {
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

    //showFunc(); // 打印所有未定义函数 Not required in Lab3
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
        // ExtDef -> Specifier FunDec SEMI
    
        // 遇到函数 压栈
        stack_push(); 
        // 创建符号
        Symbol sym = ir_FunDec(root->Tree_child[1], type);
        assert(root->Tree_child[2] != NULL);


        if(findSymbol(sym->name) == NULL) {
            insertSymbol(sym);
        } else {
            // 只会出现一次函数定义
            // shouldn't reach here!!!
            assert(0);
        }

        if(IS_EQUAL(root->Tree_child[2]->Tree_token, "CompSt")) {
            ir_CompSt(root->Tree_child[2], type);
        } else {
            // Shouldn't reach here!!!
            assert(0);
        }
        // 退栈
        stack_pop();
        
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

Type sdt_Specifier(TreeNode_t *root) {
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

Type sdt_StructSpecifier(TreeNode_t* root) {
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
        sym->prev = sym->next = sym->area_prev = sym->area_next = NULL;

        if(root->Tree_child[1] == NULL) {
            // 匿名结构体指定一个名字
            strncpy(sym->name, get_name(), 55);
        } else {
            strncpy(sym->name, sdt_OptTag(root->Tree_child[1]), 55);
        }

        if(root->Tree_child[3] != NULL)
            type->u.structure = sdt_DefList(root->Tree_child[3], 1);

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
        
        //Symbol exist_sym = findSymbol(sym->name);
        // TODO 结构体名字重复待考虑
        if(existSymbol(sym->name) || findType(sym->name) != NULL) {
            // Shound't reach here!!!
            assert(0);
        } else {
            insertSymbol(sym);
        }
        return sym;
    } else {
        assert(root->Tree_child[0] != NULL);
        Type newType = myAlloc(sizeof(Type_t));
        newType->kind = ARRAY;
        newType->u.array.size = root->Tree_child[2]->val_UINT;
        newType->u.array.elem = baseType;

        return ir_VarDec(root->Tree_child[0], newType, size+1, inStruct);
    }
}

Symbol ir_FunDec(TreeNode_t* root, Type retType) {
    helper(root);
    /* 
    * FunDec -> ID LP VarList RP
    * FunDec -> ID LP RP
    */

    assert(root->num_child == 3 || root->num_child == 4);

    Type type = myAlloc(sizeof(Type_t));
    Symbol sym = myAlloc(sizeof(Symbol_t));
    type->kind = FUNC;
    type->u.func.ret = retType;
    type->u.func.params = NULL;

    assert(root->Tree_child[0] != NULL);
    strncpy(sym->name, sdt_ID(root->Tree_child[0]), 55);
    sym->type = type;

    if(root->num_child == 4) {
        assert(root->Tree_child[2] != NULL);
        type->u.func.params = ir_VarList(root->Tree_child[2]);
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
    Symbol sym = ir_VarDec(root->Tree_child[1], type, 0, 0);

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
        ir_DefList(root->Tree_child[1], 0);
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
        ir_Exp(root->Tree_child[0], NULL);
        return;
    }

    if(root->num_child == 3) {
        // Stmt -> RETURN Exp SEMI
        assert(root->Tree_child[1] != NULL);

        Operand t1 = get_temp();
        ir_Exp(root->Tree_child[1], t1);
        
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

            ir_Cond(Exp, label1, label2); // TODO

            InterCode l1 = myAlloc(sizeof(InterCode_t));
            l1->kind = LABEL;
            l1->u.label.op = label1;
            append_code(l1);

            ir_Stmt(root->Tree_child[4], retType);

            InterCode l2 = myAlloc(sizeof(InterCode_t));
            l2->kind = LABEL;
            l2->u.label.op = label2;
            append_code(l2);
        } else {
            Operand label1 = get_label();
            Operand label2 = get_label();
            Operand label3 = get_label();

            InterCode l1 = myAlloc(sizeof(InterCode_t));
            l1->kind = LABEL;
            l1->u.label.op = label1;
            append_code(l1);

            ir_Cond(Exp, label2, label3); // TODO

            InterCode l2 = myAlloc(sizeof(InterCode_t));
            l2->kind = LABEL;
            l2->u.label.op = label2;
            append_code(l2);

            ir_Stmt(root->Tree_child[6], retType);

            InterCode g1 = myAlloc(sizeof(InterCode_t));
            g1->kind = LABEL;
            g1->u.label.op = label1;
            append_code(g1);

            InterCode l3 = myAlloc(sizeof(InterCode_t));
            l3->kind = LABEL;
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

        ir_Cond(Exp, label1, label2); // TODO

        InterCode l1 = myAlloc(sizeof(InterCode_t));
        l1->kind = LABEL;
        l1->u.label.op = label1;
        append_code(l1);

        ir_Stmt(root->Tree_child[4], retType);

        InterCode g1 = myAlloc(sizeof(InterCode_t));
        g1->kind = LABEL;
        g1->u.label.op = label3;
        append_code(g1);


        InterCode l2 = myAlloc(sizeof(InterCode_t));
        l2->kind = LABEL;
        l2->u.label.op = label2;
        append_code(l2);

        ir_Stmt(root->Tree_child[6], retType);

        InterCode l3 = myAlloc(sizeof(InterCode_t));
        l3->kind = LABEL;
        l3->u.label.op = label3;
        append_code(l3);

        return;
    }

    // Should't reach Here!!!
    assert(0);

}


/* Local Definitions */
FieldList ir_DefList(TreeNode_t* root, int inStruct) {
    helper(root);
    /* 
    * DefList -> Def DefList 
    * DefList -> e
    */

    assert(root->num_child == 2);

    if(inStruct == 1) {
        FieldList field = ir_Def(root->Tree_child[0], inStruct); 
        if(root->Tree_child[1] != NULL) {
            FieldList next_field = ir_DefList(root->Tree_child[1], inStruct);
            FieldList cur = field;
            while(cur->tail != NULL)
                cur = cur->tail;
            cur->tail = next_field;
        }
        return field;
    } else {
        ir_Def(root->Tree_child[0], inStruct);
        if(root->Tree_child[1] != NULL) {
            ir_DefList(root->Tree_child[1], inStruct);
        }
        return NULL;
    }
}

FieldList ir_Def(TreeNode_t* root, int inStruct) {
    helper(root);
    /*
    * Def -> Specifier DecList SEMI
    */

    assert(root->num_child == 3);
   
    assert(root->Tree_child[0] != NULL);
    Type type = ir_Specifier(root->Tree_child[0]);

    assert(root->Tree_child[1] != NULL);
    return ir_DecList(root->Tree_child[1], type, inStruct);
}

FieldList ir_DecList(TreeNode_t* root, Type baseType, int inStruct) {
    helper(root);
    /* 
    * DecList -> Dec
    * DecList -> Dec COMMA DecList
    */

    assert(root->num_child == 1 || root->num_child == 3);

    assert(root->Tree_child[0] != NULL);

    if(inStruct == 1) {
        FieldList field = ir_Dec(root->Tree_child[0], baseType, inStruct);
        field->tail = NULL;
        if(root->num_child != 1) {
            assert(root->Tree_child[2] != NULL);
            FieldList next_field = ir_DecList(root->Tree_child[2], baseType, inStruct);
            FieldList cur = field;
            while(cur->tail != NULL)
                cur = cur->tail;
            cur->tail = next_field;
        }
        return field;
    } else {
        ir_Dec(root->Tree_child[0], baseType, inStruct);
        if(root->num_child != 1) {
            assert(root->Tree_child[2] != NULL);
            ir_DecList(root->Tree_child[2], baseType, inStruct);
        }
        return NULL;

    }


}

FieldList ir_Dec(TreeNode_t* root, Type baseType, int inStruct) {
    helper(root);
    /*
    * Dec -> VarDec
    * Dec -> VarDec ASSIGNOP Exp
    */

    assert(root->num_child == 1 || root->num_child == 3);
    
    assert(root->Tree_child[0] != NULL);
    Symbol sym = ir_VarDec(root->Tree_child[0], baseType, 0, inStruct);

    if(inStruct == 1) {
        FieldList field = myAlloc(sizeof(FieldList_t));
        field->tail = NULL;
        strncpy(field->name, sym->name, 55);
        field->type = sym->type;

        if(root->num_child == 3) {
            // 结构体里面不能赋值
            assert(0);
        }
        return field;
    } else {
        
        if(root->num_child == 3) {
            // TODO

        }
        return NULL;
    }

}


/* Expressions */

void ir_Exp(TreeNode_t* root, Operand place) {
    helper(root);
    /*
    * Exp -> Exp ASSIGNOP Exp
    * Exp -> Exp AND Exp 
    * Exp -> Exp OR Exp 
    * Exp -> Exp RELOP Exp 
    * Exp -> Exp PLUS Exp
    * Exp -> Exp MINUS Exp
    * Exp -> Exp STAR Exp 
    * Exp -> Exp DIV Exp 
    * Exp -> LP Exp RP
    * 
    * Exp -> MINUS Exp 
    * Exp -> NOT Exp 
    * 
    * Exp -> ID LP Args RP
    * Exp -> ID LP RP 
    * Exp -> Exp LB Exp RB
    * Exp -> Exp DOT ID
    * Exp -> ID
    * Exp -> INT
    * Exp -> FLOAT
    */

    if(root->num_child == 3) {
        assert(root->Tree_child[0] != NULL);
        assert(root->Tree_child[1] != NULL);
        assert(root->Tree_child[2] != NULL);

        if(IS_EQUAL(root->Tree_child[0]->Tree_token, "Exp") && IS_EQUAL(root->Tree_child[2]->Tree_token, "Exp")) {
            // Exp (ASSIGNOP AND OR RELOP PLUS MINUS STAR DIV) Exp
            expType_t ltype = sdt_Exp(root->Tree_child[0]);
            expType_t rtype = sdt_Exp(root->Tree_child[2]);
            if(IS_EQUAL(root->Tree_child[1]->Tree_token, "ASSIGNOP")) {
                // TODO

            } else {
                if(IS_EQUAL(root->Tree_child[1]->Tree_token, "AND") 
                  || IS_EQUAL(root->Tree_child[1]->Tree_token, "OR") 
                  || IS_EQUAL(root->Tree_child[1]->Tree_token, "RELOP") ) {
                    Operand label1 = get_label();
                    Operand label2 = get_label();
                    
                    InterCode code0 = myAlloc(sizeof(InterCode_t));
                    code0->kind = ASSIGN;
                    code0->u.assign.left = place;
                    code0->u.assign.right = &OP_ZERO;
                    append_code(code0);

                    ir_Cond(root, label1, label2);

                    InterCode l1 = myAlloc(sizeof(InterCode_t));
                    l1->kind = LABEL;
                    l1->u.label.op = label1;
                    append_code(l1);

                    InterCode code2 = myAlloc(sizeof(InterCode_t));
                    code2->kind = ASSIGN;
                    code2->u.assign.left = place;
                    code2->u.assign.right = &OP_ONE;
                    append_code(code2);

                    InterCode l2 = myAlloc(sizeof(InterCode_t));
                    l2->kind = LABEL;
                    l2->u.label.op = label2;
                    append_code(l2);
                } else {
                    Operand op1 = get_temp();
                    Operand op2 = get_temp();
                    ir_Exp(root->Tree_child[0], op1);
                    ir_Exp(root->Tree_child[2], op2);

                    InterCode code = myAlloc(sizeof(InterCode_t));
                    code->u.binop.op1 = op1;
                    code->u.binop.op2 = op2;
                    code->u.binop.result = place;

                    if(IS_EQUAL(root->Tree_child[1]->Tree_token, "PLUS")) {
                        code->kind = PLUS;
                    } else if(IS_EQUAL(root->Tree_child[1]->Tree_token, "MINUS")) {
                        code->kind = MINUS;
                    } else if(IS_EQUAL(root->Tree_child[1]->Tree_token, "STAR")) {
                        code->kind = MUL;
                    } else if(IS_EQUAL(root->Tree_child[1]->Tree_token, "DIV")) {
                        code->kind = DIV;
                    } else {
                        assert(0);
                    }
                    append_code(code);

                }
            }
            
        }

        if(IS_EQUAL(root->Tree_child[0]->Tree_token, "ID")) {
            // Exp -> ID LP RP
            char func_name[55];
            strncpy(func_name, sdt_ID(root->Tree_child[0]), 55);
            Symbol sym = findSymbol(func_name);
            
            if(IS_EQUAL(func_name, "read")) {
                InterCode code = myAlloc(sizeof(InterCode_t));
                code->kind = READ;
                code->u.unary.op = place;
                append_code(code);
            } else {
                InterCode code = myAlloc(sizeof(InterCode_t));
                code->kind = CALL;
                code->u.assign.left = place;

                Operand func = myAlloc(sizeof(Operand_t));
                func->kind = FUNC;
                func->u.val_no = sym->var_no;
                code->u.assign.right = func;
                append_code(code);
            }

            return ;

        }

        if(IS_EQUAL(root->Tree_child[0]->Tree_token, "LP")) {
            // Exp -> LP Exp RP
            ir_Exp(root->Tree_child[1], place);
            return;
        }

        if(IS_EQUAL(root->Tree_child[1]->Tree_token, "DOT")) {
            // Exp -> Exp DOT ID 
            // TODO 
            expType_t struct_type = sdt_Exp(root->Tree_child[0]);
            char field_name[55];
            strncpy(field_name, sdt_ID(root->Tree_child[2]), 55);


            FieldList field = struct_type.type->u.structure;
            while(field != NULL && !IS_EQUAL(field->name, field_name)) {
                field  = field->tail;
            }

            return;
        }

        // Shound't reach here!!!
        assert(0);
    }

    if(root->num_child == 2) {
        assert(root->Tree_child[0] != NULL);
        assert(root->Tree_child[1] != NULL);

        if(IS_EQUAL(root->Tree_child[0]->Tree_token, "MINUS")) {
            // Exp -> MINUS Exp
            // TODO 确认类型 报错
            Operand t1 = get_temp();
            ir_Exp(root->Tree_child[1], t1);

            InterCode code2 = myAlloc(sizeof(InterCode_t));
            code2->kind = MINUS;
            code2->u.binop.op1 = &OP_ZERO;
            code2->u.binop.op2 = t1
            code2->u.binop.result = place;
            append_code(code2);

            return;
        }

        if(IS_EQUAL(root->Tree_child[0]->Tree_token, "NOT")) {
            // Exp -> NOT Exp
            Operand label1 = get_label();
            Operand label2 = get_label();   

            InterCode code0 = myAlloc(sizeof(InterCode_t));
            code0->kind = ASSIGN;
            code0->u.assign.left = place;
            code0->u.assign.right = &OP_ZERO;
            append_code(code0);

            ir_Cond(root, label1, label2);

            InterCode l1 = myAlloc(sizeof(InterCode_t));
            l1->kind = LABEL;
            l1->u.label.op = label1;
            append_code(l1);

            InterCode code2 = myAlloc(sizeof(InterCode_t));
            code2->kind = ASSIGN;
            code2->u.assign.left = place;
            code2->u.assign.right = &OP_ONE;
            append_code(code2);

            InterCode l2 = myAlloc(sizeof(InterCode_t));
            l2->kind = LABEL;
            l2->u.label.op = label2;
            append_code(l2);
            return;
        }
        // Shound't reach here!!!
        assert(0);
    }

    if(root->num_child ==  4) {
        assert(root->Tree_child[0] != NULL);
        assert(root->Tree_child[1] != NULL);
        assert(root->Tree_child[2] != NULL);
        assert(root->Tree_child[3] != NULL);
        if(IS_EQUAL(root->Tree_child[0]->Tree_token, "ID")) {
            // Exp -> ID LP Args RP
            // TODO
            char func_name[55];
            strncpy(func_name, sdt_ID(root->Tree_child[0]), 55);
            Symbol sym = findSymbol(func_name);

            if(IS_EQUAL(func_name, "WRITE")) {
                Operand t1 = get_temp();
                ir_Exp(root->Tree_child[2]->Tree_child[0], t1);

                InterCode code = myAlloc(sizeof(InterCode_t));
                code->kind = WRITE;
                code->u.unary.op = t1;
                append_code(code);

            } else {
                ir_Args(root->Tree_child[2]);

                InterCode code = myAlloc(sizeof(InterCode_t));
                code->kind = CALL;
                code->u.assign.left = place;
                Operand func = myAlloc(sizeof(Operand_t));
                func->kind = FUNC;
                func->u.val_no = sym->var_no;
                code->u.assign.right = func;
                append_code(code);
            } 

            return;

        }
        if(IS_EQUAL(root->Tree_child[0]->Tree_token, "Exp")) {
            // Exp -> Exp LB Exp RB
            // TODO
            expType_t ltype = sdt_Exp(root->Tree_child[0]);
            expType_t rtype = sdt_Exp(root->Tree_child[2]);

            if(ltype.type->kind != ARRAY) {
                // 不是数组类型 或者维度不对
                // 报错 类型10: 对非数组型变量使用"[...]"操作符
                sdt_error(10, root->Tree_lineno, "ARRAYH");
                return exp_INT;
            }

            if(!same_type(rtype, exp_INT)) {
                // 下标非整数
                // 报错 类型12: 数组访问操作符"[...]"中出现非整数
                sdt_error(12, root->Tree_lineno, "ARRAY");
                return exp_INT;
            }

            ltype.type = ltype.type->u.array.elem;

            ltype.var = 1;

            return ltype;
        }
        // Shound't reach here!!!
        assert(0);
    }

    if(root->num_child == 1) {
        assert(root->Tree_child[0] != NULL);
        if(IS_EQUAL(root->Tree_child[0]->Tree_token, "ID")) {
            // Exp -> ID
            // TODO 查找符号表 获取ID的类型
            char name[55];
            strncpy(name, sdt_ID(root->Tree_child[0]), 55);
            Symbol sym = findSymbol(name);
            if(sym == NULL) {
                // 变量未定义
                // 报错 类型1: 变量在使用时未经定义
                sdt_error(1, root->Tree_lineno, "Variable");
                return exp_INT;
            }
            expType_t type = {sym->type, 1};
            return type;
        }
        if(IS_EQUAL(root->Tree_child[0]->Tree_token, "INT")) {
            // Exp -> INT
            return exp_INT;
        }
        if(IS_EQUAL(root->Tree_child[0]->Tree_token, "FLOAT")) {
            // Exp -> FLOAT
            return exp_FLOAT;
        }
        // Shound't reach here!!!
        assert(0);
    }

    // Shound't reach here!!!
    //printf("%s\n", root->Tree_token);
    for(int i=0; i<root->num_child; ++i) {
        printf("%s ", root->Tree_child[i]->Tree_token);
    }
    assert(0);
}


int ir_Args(TreeNode_t* root) {
    helper(root);
    /*
    * Args -> Exp COMMA Args
    * Args -> Exp
    */

    assert(root->num_child == 1 || root->num_child == 3);

    assert(root->Tree_child[0] != NULL);

    if(root->num_child == 3) {
        assert(root->Tree_child[2] != NULL);
        ir_Args(root->Tree_child[2]);
    }


    Operand t1 = myAlloc(sizeof(Operand_t));
    ir_Exp(root->Tree_child[0], t1);

    InterCode code = myAlloc(sizeof(InterCode_t));
    code->kind = ARG;
    code->u.unary.op = t1;
    append_code(code);
    
    return;
}


/* Terminator */
char* ir_ID(TreeNode_t* root) {
    helper(root);
    return root->Tree_val;
}

int ir_TYPE(TreeNode_t* root) {
    helper(root);
    if(IS_EQUAL(root->Tree_val, "int"))
        return TYPE_INT;
    else if(IS_EQUAL(root->Tree_val, "float")) 
        return TYPE_FLOAT;
    else 
        assert(0);
    return -1;
}