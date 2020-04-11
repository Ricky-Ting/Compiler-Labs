#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "Tree.h"
#include "symbol.h"
#include "sdt.h"

Type_t type_INT, type_FLOAT;
expType_t exp_INT, exp_FLOAT;
char random_name[55];
int pos;

char* errMessage[19] = {
    "Undefined variable",
    "Undefined function",
    "Redefined variable",
    "Redefined function",
    "Type mismatched for assignment",
    "The left-hand side of an assignment must be a variable",
    "Type mismatched for operands",
    "Type mismatched for return",
    "Function args don't match",
    "Not an array",
    "Not a function",
    "Not an integer",
    "Illegal use of \".\"",
    "Non-existent field",
    "Redefined field",
    "Duplicated name",
    "Undefined structure",
    "Undefined function",
    "Inconsistent declaration of function"
};


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
        return _same_type(a->u.array.elem, b->u.array.elem) && (a->u.array.size == b->u.array.size);
    
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

int same_type(expType_t a, expType_t b) {
    if(a.type == NULL && b.type == NULL)
        return 1;
    if(a.type == NULL || b.type == NULL) 
        return 0;

    //printf("%d %d \n", a.size, b.size);

    if(a.type->kind != b.type->kind)
        return 0;

    if(a.type->kind == ARRAY) {
        if(!_same_type(a.type->u.array.elem, b.type->u.array.elem) || a.type->u.array.size - a.size != b.type->u.array.size - b.size) {
            return 0;
        }
        return 1;
    }
    return _same_type(a.type, b.type);
}



void sdt_error(int err, int lineno, char* s) {
    fprintf(stderr, "Error type %d at Line %d: %s.\n", err, lineno, errMessage[err-1]);
}

void sdt_init() {
    type_INT.kind = BASIC;
    type_INT.u.basic = TYPE_INT;
    type_FLOAT.kind = BASIC;
    type_FLOAT.u.basic = TYPE_FLOAT;

    exp_INT.type = &type_INT;
    exp_INT.size = 0;
    exp_INT.var = 0;
    exp_FLOAT.type = &type_FLOAT;
    exp_FLOAT.size = 0;
    exp_FLOAT.var = 0;

    random_name[0] = '#';
    random_name[1] = 'A';
    random_name[2] = '\0';
    pos = 1;

    initTables();
    return;
}


/* High-level Definitions */
void sdt_Program(TreeNode_t* root) {
    sdt_init();
    helper(root);
    /*
    * Program -> ExtDefList
    */
    if(root == NULL)
        return;
    if(root->num_child == 0)
        return;
    if(root->Tree_child[0] != NULL)
        sdt_ExtDefList(root->Tree_child[0]);
    showFunc(); // 打印所有未定义函数
    return;
}

void sdt_ExtDefList(TreeNode_t *root) {
    helper(root);
    /*
    * ExtDefList -> ExtDef ExtDefList
    * ExtDefList -> e (not reach)
    */

    assert(root->num_child == 2);
    
    assert(root->Tree_child[0] != NULL);
    sdt_ExtDef(root->Tree_child[0]);

    if(root->Tree_child[1] != NULL) {
        sdt_ExtDefList(root->Tree_child[1]);
    }
}

void sdt_ExtDef(TreeNode_t *root) {
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
    Type type = sdt_Specifier(root->Tree_child[0]);

    /* ExtDecList SEMI FunDec 都不会产生空 */
    assert(root->Tree_child[1] != NULL);
    if(IS_EQUAL(root->Tree_child[1]->Tree_token, "SEMI")) {
        // ExtDef -> Specifier SEMI
        return;
    } else if(IS_EQUAL(root->Tree_child[1]->Tree_token, "ExtDecList")) {
        // ExtDef -> Specifier ExtDecList SEMI
        sdt_ExtDecList(root->Tree_child[1], type);
        return;
    } else if(IS_EQUAL(root->Tree_child[1]->Tree_token, "FunDec")) {
        // ExtDef -> Specifier FunDec Compst
        // ExtDef -> Specifier FunDec SEMI
    
        // 遇到函数 压栈
        stack_push(); 
        // 创建符号
        Symbol sym = sdt_FunDec(root->Tree_child[1], type);
        assert(root->Tree_child[2] != NULL);

        stack_pop();

        if(findSymbol(sym->name) == NULL) {
            insertSymbol(sym);
            insertFunc(sym->name, root->Tree_lineno);
        } else {
            // 判断是否是同一类型函数
            if(!_same_type(sym->type, findSymbol(sym->name)->type)) {
                // 定义不同
                // 报错 类型19: 函数的多次声明互相冲突，或者声明与定义之间互相冲突
                sdt_error(19, root->Tree_lineno, "FUNC");   
                // 是否要终止？？？ 待考虑 TODO
            }
        }

        if(IS_EQUAL(root->Tree_child[2]->Tree_token, "CompSt")) {
            if(!deleteFunc(sym->name)) {
                // 报错 类型4: 函数出现重复定义
                sdt_error(4, root->Tree_lineno, "Redefined");
            }
            stack_push();
            sdt_FunDec(root->Tree_child[1], type);
            sdt_CompSt(root->Tree_child[2], type);
            stack_pop();
        }
        // 退栈
        
        return;
    }
}

void sdt_ExtDecList(TreeNode_t *root, Type baseType) {
    helper(root);
    /*
    * ExtDecList -> VarDec
    * ExtDecList -> VarDec COMMA ExtDecList
    */
   
   assert(root->num_child == 1 || root->num_child == 3);
   
   assert(root->Tree_child[0] != NULL);
   sdt_VarDec(root->Tree_child[0], baseType, 0, 0); // 0表示数组的维度，最开始是0维的

   if(root->num_child == 1) 
       return;
       
    assert(root->Tree_child[2] != NULL);
    sdt_ExtDecList(root->Tree_child[2], baseType);
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
        int basic_type = sdt_TYPE(root->Tree_child[0]);
        if(basic_type == TYPE_INT) 
            return &type_INT;
        else 
            return &type_FLOAT;
    } else if(IS_EQUAL(root->Tree_child[0]->Tree_token, "StructSpecifier")) {
        return sdt_StructSpecifier(root->Tree_child[0]);
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
        char *name = sdt_Tag(root->Tree_child[1]);
        Symbol sym = findType(name);
        if(sym == NULL) {
            // 找不到这个结构体类型
            // 报错 类型17: 直接使用未定义过得结构体来定义变量
            sdt_error(17, root->Tree_lineno, "STRUCT");
            return &type_INT;
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

        //Symbol exist_sym = findSymbol(sym->name);
        //结构体类型的作用域是全局的
        // TODO 待check
        if(findSymbol(sym->name) != NULL || findType(sym->name) != NULL) {
            // 同名结构体或变量
            // 报错 类型16: 结构体的名字与前面定义过得结构体或变量的名字重复
            sdt_error(16, root->Tree_lineno, "STRUCT");
            stack_pop();
            return &type_INT;
        } else {
            insertType(sym);
            stack_pop();
            return type;
        }
        
        
    }
}

char* sdt_OptTag(TreeNode_t* root) {
    helper(root);
    /*
        OptTag -> ID
        OptTag -> e
    */
    assert(root->num_child == 1);
    return root->Tree_child[0]->Tree_val;
}

char* sdt_Tag(TreeNode_t* root) {
    helper(root);
    // Tag -> ID
    return root->Tree_child[0]->Tree_val;
}


/* Declarators */
Symbol sdt_VarDec(TreeNode_t* root, Type baseType, int size, int inStruct) {
    helper(root);
    /* 
    * VarDec -> ID
    * VarDec -> VarDec LB INT RB
    */

    assert(root->num_child == 1 || root->num_child == 4);
    
    if(root->num_child == 1) {
        assert(root->Tree_child[0] != NULL);
        Type type = NULL;
        if(size == 0) {
            type = baseType;
        } else {
            type = myAlloc(sizeof(Type_t));
            type->kind = ARRAY;
            type->u.array.elem = baseType;
            type->u.array.size = size;
        }
        Symbol sym = myAlloc(sizeof(Symbol_t));
        strncpy(sym->name, sdt_ID(root->Tree_child[0]), 55);
        sym->type = type;
        sym->prev = sym->next = sym->area_prev = sym->area_next = NULL;
        
        //Symbol exist_sym = findSymbol(sym->name);
        // TODO 结构体名字重复待考虑
        if(existSymbol(sym->name) || findType(sym->name) != NULL) {
            // 变量重复定义

            if(inStruct) {
                // 报错 类型15: 结构体中域名重复定义
                sdt_error(15, root->Tree_lineno, "Variable");
            } else {
                // 报错 类型3: 变量出现重复定义，或变量与前面定义过的结构体名字重复
                sdt_error(3, root->Tree_lineno, "Variable");
            }

            // TODO 这次是否直接返回 待考虑
        } else {
            insertSymbol(sym);
        }
        return sym;
    } else {
        assert(root->Tree_child[0] != NULL);
        return sdt_VarDec(root->Tree_child[0], baseType, size+1, inStruct);
    }
}

Symbol sdt_FunDec(TreeNode_t* root, Type retType) {
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
        type->u.func.params = sdt_VarList(root->Tree_child[2]);
    }

    return sym;
}

FieldList sdt_VarList(TreeNode_t* root) {
    helper(root);
    /*
    * VarList -> ParamDec COMMA VarList
    * VarList -> ParamDec
    */

    assert(root->num_child == 1 || root->num_child == 3);

    assert(root->Tree_child[0] != NULL);

    FieldList field = sdt_ParamDec(root->Tree_child[0]);

    if(root->num_child == 3) {
        assert(root->Tree_child[2] != NULL);
        FieldList next_field = sdt_VarList(root->Tree_child[2]);
        FieldList cur = field;
        while(cur->tail != NULL)
            cur = cur->tail;
        cur->tail = next_field;
    }    
    return field;
}


FieldList sdt_ParamDec(TreeNode_t* root) {
    helper(root);
    /*
    * ParamDec -> Specifier VarDec 
    */

    assert(root->num_child == 2);
    assert(root->Tree_child[0] != NULL && root->Tree_child[1] != NULL);

    Type type = sdt_Specifier(root->Tree_child[0]);
    Symbol sym = sdt_VarDec(root->Tree_child[1], type, 0, 0);

    FieldList field = myAlloc(sizeof(FieldList_t));
    strncpy(field->name, sym->name, 55);
    field->type = sym->type;
    field->tail = NULL;
    
    
    return field;
}


/* Statements */

void sdt_CompSt(TreeNode_t* root, Type retType) {
    helper(root);
    /*
    * CompSt -> LC DefList StmtList RC
    */

    assert(root->num_child == 4);
    if(root->Tree_child[1] != NULL) {
        sdt_DefList(root->Tree_child[1], 0);
    }

    if(root->Tree_child[2] != NULL) {
        sdt_StmtList(root->Tree_child[2], retType);
    }
    return;
}

void sdt_StmtList(TreeNode_t* root, Type retType) {
    helper(root);
    /*
    * StmtList -> Stmt StmtList
    */

    assert(root->num_child == 2);
    assert(root->Tree_child[0] != NULL);
    sdt_Stmt(root->Tree_child[0], retType);
    if(root->Tree_child[1] != NULL)
        sdt_StmtList(root->Tree_child[1], retType);
    return;
}

void sdt_Stmt(TreeNode_t* root, Type retType) {
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
        sdt_CompSt(root->Tree_child[0], retType);
        stack_pop();
        return;
    }

    if(root->num_child == 2) {
        // Stmt -> Exp SEMI;
        assert(root->Tree_child[0] != NULL);
        sdt_Exp(root->Tree_child[0]);
        return;
    }

    if(root->num_child == 3) {
        // Stmt -> RETURN Exp SEMI
        assert(root->Tree_child[1] != NULL);
        expType_t type = sdt_Exp(root->Tree_child[1]);
        expType_t exp_retType = {retType, 0, 0};
        if(!same_type(type, exp_retType)) {
            // 返回值类型不统一
            // 报错 类型8: return语句的返回类型与函数定义的返回类型不匹配
            sdt_error(8, root->Tree_lineno, "return");
        }
        return;
    }

    if(root->num_child == 5) {
        // Stmt -> IF LP Exp RP Stmt
        // Stmt -> WHILE LP Exp RP Stmt
        assert(root->Tree_child[2] != NULL);
        assert(root->Tree_child[4] != NULL);
        expType_t type = sdt_Exp(root->Tree_child[2]);
        if(!same_type(type, exp_INT)) {
            // 报错 类型7: balabal
            sdt_error(7, root->Tree_lineno, "IF WHILE");
        }
        sdt_Stmt(root->Tree_child[4], retType);
        return;
    }

    if(root->num_child == 7) {
        // Stmt -> IF LP Exp RP Stmt ELSE Stmt
        assert(root->Tree_child[2] != NULL);
        assert(root->Tree_child[4] != NULL);
        assert(root->Tree_child[6] != NULL);
        expType_t type = sdt_Exp(root->Tree_child[2]);
        if(!same_type(type, exp_INT)) {
            // 报错 类型7: balabal
            sdt_error(7, root->Tree_lineno, "IF WHILE");
        }
        sdt_Stmt(root->Tree_child[4], retType);
        sdt_Stmt(root->Tree_child[6], retType);
        return;
    }

    // Should't reach Here!!!
    assert(0);

}


/* Local Definitions */
FieldList sdt_DefList(TreeNode_t* root, int inStruct) {
    helper(root);
    /* 
    * DefList -> Def DefList 
    * DefList -> e
    */

    assert(root->num_child == 2);

    if(inStruct == 1) {
        FieldList field = sdt_Def(root->Tree_child[0], inStruct); 
        if(root->Tree_child[1] != NULL) {
            FieldList next_field = sdt_DefList(root->Tree_child[1], inStruct);
            FieldList cur = field;
            while(cur->tail != NULL)
                cur = cur->tail;
            cur->tail = next_field;
        }
        return field;
    } else {
        sdt_Def(root->Tree_child[0], inStruct);
        if(root->Tree_child[1] != NULL) {
            sdt_DefList(root->Tree_child[1], inStruct);
        }
        return NULL;
    }
}

FieldList sdt_Def(TreeNode_t* root, int inStruct) {
    helper(root);
    /*
    * Def -> Specifier DecList SEMI
    */

    assert(root->num_child == 3);
   
    assert(root->Tree_child[0] != NULL);
    Type type = sdt_Specifier(root->Tree_child[0]);

    assert(root->Tree_child[1] != NULL);
    return sdt_DecList(root->Tree_child[1], type, inStruct);
}

FieldList sdt_DecList(TreeNode_t* root, Type baseType, int inStruct) {
    helper(root);
    /* 
    * DecList -> Dec
    * DecList -> Dec COMMA DecList
    */

    assert(root->num_child == 1 || root->num_child == 3);

    assert(root->Tree_child[0] != NULL);

    if(inStruct == 1) {
        FieldList field = sdt_Dec(root->Tree_child[0], baseType, inStruct);
        field->tail = NULL;
        if(root->num_child != 1) {
            assert(root->Tree_child[2] != NULL);
            FieldList next_field = sdt_DecList(root->Tree_child[2], baseType, inStruct);
            FieldList cur = field;
            while(cur->tail != NULL)
                cur = cur->tail;
            cur->tail = next_field;
        }
        return field;
    } else {
        sdt_Dec(root->Tree_child[0], baseType, inStruct);
        if(root->num_child != 1) {
            assert(root->Tree_child[2] != NULL);
            sdt_DecList(root->Tree_child[2], baseType, inStruct);
        }
        return NULL;

    }


}

FieldList sdt_Dec(TreeNode_t* root, Type baseType, int inStruct) {
    helper(root);
    /*
    * Dec -> VarDec
    * Dec -> VarDec ASSIGNOP Exp
    */

    assert(root->num_child == 1 || root->num_child == 3);
    
    assert(root->Tree_child[0] != NULL);
    Symbol sym = sdt_VarDec(root->Tree_child[0], baseType, 0, inStruct);

    if(inStruct == 1) {
        FieldList field = myAlloc(sizeof(FieldList_t));
        field->tail = NULL;
        strncpy(field->name, sym->name, 55);
        field->type = sym->type;

        if(root->num_child == 3) {
            // 结构体里面不能赋值
            // 报错 类型15: 结构体中域名重复定义，或在定义时对域进行初始化
            sdt_error(15, root->Tree_lineno, "Strutc");
        }
        return field;
    } else {
        
        if(root->num_child == 3) {
            // 判断左右类型是否相同
            expType_t ltype = {sym->type, 0, 0};
            expType_t rtype = sdt_Exp(root->Tree_child[2]);
            if(!same_type(ltype, rtype)) {
                // 报错 类型5: 赋值号两边的表达式类型不匹配
                sdt_error(5, root->Tree_lineno, "=");
            }
        }
        return NULL;
    }

}


/* Expressions */

expType_t sdt_Exp(TreeNode_t* root) {
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
                if(!same_type(ltype, rtype)) {
                    // 报错 类型5: 赋值号两边的表达式类型不匹配
                    sdt_error(5, root->Tree_lineno, "Type mismatched for assignment");
                    return exp_INT;
                }
                if(!ltype.var) {
                    // 报错 类型6: 赋值号左边出现一个只有右值得表达式
                    sdt_error(6, root->Tree_lineno, "Dism");
                    return exp_INT;
                }
                return ltype;
            } else {
                if(!same_type(ltype, rtype)) {
                    // 报错 类型7: 操作数类型不匹配或操作数类型与操作符不匹配
                    sdt_error(7, root->Tree_lineno, "Unmatched operands");
                    return exp_INT;
                }

                if(IS_EQUAL(root->Tree_child[1]->Tree_token, "AND") || IS_EQUAL(root->Tree_child[1]->Tree_token, "OR")) {
                    // 报错 类型7: 操作数类型不匹配或操作数类型与操作符不匹配
                    if(!same_type(ltype, exp_INT)) {
                        sdt_error(7, root->Tree_lineno, "Unmatched operands");
                        return exp_INT;
                    }
                    return exp_INT;
                }

                if(!same_type(ltype, exp_INT) && !same_type(rtype, exp_FLOAT)) {
                    // 报错 类型7: 操作数类型不匹配或操作数类型与操作符不匹配
                    sdt_error(7, root->Tree_lineno, "Unmatched operands");
                    return exp_INT;
                }
                ltype.var = 0;
                return ltype;
            }
            
        }

        if(IS_EQUAL(root->Tree_child[0]->Tree_token, "ID")) {
            // Exp -> ID LP RP
            char func_name[55];
            strncpy(func_name, sdt_ID(root->Tree_child[0]), 55);
            Symbol sym = findSymbol(func_name);
            if(sym == NULL) {
                // 函数未定义
                // 报错 类型2: 函数在调用时未经定义
                sdt_error(2, root->Tree_lineno, "Undefined func");
                return exp_INT;
            }

            if(sym->type->kind != FUNC) {
                // 非函数类型
                // 报错 类型11: 对普通变量使用"(...)"或"()"操作符
                sdt_error(11, root->Tree_lineno, "Func");
                return exp_INT;
            }
            if(sym->type->u.func.params != NULL) {
                // 参数不符合
                // 报错 类型9: 函数调用时实参与形参的数目或类型不匹配
                sdt_error(9, root->Tree_lineno, "Func");
                return exp_INT;
            }
            // 函数调用成功，Exp为返回值类型
            expType_t type = {sym->type->u.func.ret, 0, 0};
            return type;

        }

        if(IS_EQUAL(root->Tree_child[0]->Tree_token, "LP")) {
            // Exp -> LP Exp RP
            expType_t type = sdt_Exp(root->Tree_child[1]);
            type.var  = 0;
            return type;
        }

        if(IS_EQUAL(root->Tree_child[1]->Tree_token, "DOT")) {
            // Exp -> Exp DOT ID 
            // TODO 
            expType_t struct_type = sdt_Exp(root->Tree_child[0]);
            char field_name[55];
            strncpy(field_name, sdt_ID(root->Tree_child[2]), 55);

            if(struct_type.type->kind != STRUCTURE) {
                // 非结构体变量
                // 报错 类型13: 对非结构体型变量使用"."操作符
                sdt_error(13, root->Tree_lineno, "STRUCT");
                return exp_INT;
            }

            FieldList field = struct_type.type->u.structure;
            while(field != NULL && !IS_EQUAL(field->name, field_name)) {
                field  = field->tail;
            }

            if(field == NULL) {
                // 结构体中没有当前域
                // 报错 类型14: 访问结构体中未定义过的域。
                sdt_error(14, root->Tree_lineno, "Undefined field");
                return exp_INT;
            }
            expType_t type = {field->type, 0, 1};

            return type;
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
            expType_t type = sdt_Exp(root->Tree_child[1]);
            if(!same_type(type, exp_INT) && !same_type(type, exp_FLOAT)) {
                // 非整数和浮点数
                // 报错 类型7: 操作数类型不匹配或操作数类型与操作符不匹配
                sdt_error(7, root->Tree_lineno, "Unmacthed");
                return exp_INT;
            }
            type.var = 0;
            return type;
        }

        if(IS_EQUAL(root->Tree_child[0]->Tree_token, "NOT")) {
            // Exp -> NOT Exp
            // TODO 确认类型 报错
            expType_t type = sdt_Exp(root->Tree_child[1]);
            if(!same_type(type, exp_INT)) {
                // 非整数
                // 报错 类型7: 操作数类型不匹配或操作数类型与操作符不匹配
                sdt_error(7, root->Tree_lineno, "Unmacthed");
                return exp_INT;
            }
            type.var = 0;
            return type;
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
            if(sym == NULL) {
                // 函数未定义
                // 报错 类型2: 函数在调用时未经定义
                sdt_error(2, root->Tree_lineno, "Undefined func");
                return exp_INT;
            }

            if(sym->type->kind != FUNC) {
                // 非函数类型
                // 报错 类型11: 对普通变量使用"(...)"或"()"操作符
                sdt_error(11, root->Tree_lineno, "Func");
                return exp_INT;
            }

            // 参数不符合的报错留个Args解决：
            if (!sdt_Args(root->Tree_child[2], sym->type->u.func.params)) {
                return exp_INT;
            }

            // 函数调用成功，Exp为返回值类型
            expType_t type = {sym->type->u.func.ret, 0, 0};
            return type;

        }
        if(IS_EQUAL(root->Tree_child[0]->Tree_token, "Exp")) {
            // Exp -> Exp LB Exp RB
            // TODO
            expType_t ltype = sdt_Exp(root->Tree_child[0]);
            expType_t rtype = sdt_Exp(root->Tree_child[2]);

            if(ltype.type->kind != ARRAY || ltype.type->u.array.size <= ltype.size) {
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

            ltype.size++;
            
            if(ltype.size == ltype.type->u.array.size) {
                expType_t ret = {ltype.type->u.array.elem, 0, 1};
                return ret;
            }
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
            expType_t type = {sym->type, 0, 1};
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


int sdt_Args(TreeNode_t* root, FieldList field) {
    helper(root);
    /*
    * Args -> Exp COMMA Args
    * Args -> Exp
    */

    assert(root->num_child == 1 || root->num_child == 3);

    assert(root->Tree_child[0] != NULL);

    if(field == NULL) {
        // 报错: 类型9: 函数调用时实参与形参的数目或类型不匹配
        sdt_error(9, root->Tree_lineno, "Args doesn't match");
        return 0;
    }
    
    expType_t type = sdt_Exp(root->Tree_child[0]);
    expType_t type2 = {field->type, 0};
    if(!same_type(type, type2)) {
        // 报错: 类型9: 函数调用时实参与形参的数目或类型不匹配
        sdt_error(9, root->Tree_lineno, "Args doesn't match");
        return 0;
    }
    
    if(root->num_child == 1) {
        if(field->tail != NULL) {
            // 报错: 类型9: 函数调用时实参与形参的数目或类型不匹配
            sdt_error(9, root->Tree_lineno, "Args doesn't match");
            return 0;
        }
        return 1;
    }

    assert(root->Tree_child[2] != NULL);
    return sdt_Args(root->Tree_child[2], field->tail);
}


/* Terminator */
char* sdt_ID(TreeNode_t* root) {
    helper(root);
    return root->Tree_val;
}

int sdt_TYPE(TreeNode_t* root) {
    helper(root);
    if(IS_EQUAL(root->Tree_val, "int"))
        return TYPE_INT;
    else if(IS_EQUAL(root->Tree_val, "float")) 
        return TYPE_FLOAT;
    else 
        assert(0);
    return -1;
}