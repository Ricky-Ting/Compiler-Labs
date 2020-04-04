#include <stdio.h>
#include "Tree.h"
#include "symbol.h"
#include <assert.h>

#define IS_EQUAL(a,b) (strcmp(a, b) == 0)

#define TYPE_INT 0
#define TYPE_FLOAT 1

/* High-level Definitions */
void sdt_Progam(TreeNode_t *root) {
    /*
    * Program -> ExtDefList
    */
    if(root->num_child == 0)
        return;
    if(root->Tree_child[0] != NULL)
        sdt_ExtDefList(root->Tree_child[0]);
    return;
}

void sdt_ExtDefList(TreeNode_t *root) {
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
        return;
    } else if(IS_EQUAL(root->Tree_child[1]->Tree_token, "ExtDecList")) {
        // TODO
        sdt_ExtDecList(root->Tree_child[1], type);
        return;
    } else if(IS_EQUAL(root->Tree_child[1]->Tree_token, "FunDec")) {
        // TODO
        stack_push();
        Symbol sym = sdt_FunDec(root->Tree_child[1], type);
        assert(root->Tree_child[2] != NULL);
        // TODO 重找重名 以及符合

        if(!findSymbol(sym)) {
            insertSymbol(sym);
        } else {
            // TODO 比较之类的
        }

        if(IS_EQUAL(root->Tree_child[2]->Tree_token, "CompSt")) {
            sdt_CompSt(root->Tree_child[2], type);
        }

        stack_pop();
        return;
    }
}

void sdt_ExtDecList(TreeNode_t *root, Type baseType) {
    /*
    * ExtDecList -> VarDec
    * ExtDecList -> VarDec COMMA ExtDecList
    */
   
   assert(root->num_child == 1 || root->num_child == 3);
   
   assert(root->Tree_child[0] != NULL);
   sdt_VarDec(root->Tree_child[0], baseType, 0); // 0表示数组的维度，最开始是0维的

   if(root->num_child == 1) 
       return;
       
    assert(root->Tree_child[2] != NULL);
    sdt_ExtDecList(root->Tree_child[2], baseType);
}

/* Specifiers */

Type sdt_Specifier(TreeNode_t *root) {
    assert(root->num_child == 1);
    assert(root->Tree_child[0] != NULL);
    if(IS_EQUAL(root->Tree_child[0]->Tree_token, "TYPE")) {
        int basic_type = sdt_TYPE(root->Tree_child[0]);
        Type type = (Type)myAlloc(sizeof(Type_t));
        type->kind = BASIC;
        type->u.basic = basic_type;
        return type;
    } else if(IS_EQUAL(root->Tree_child[0]->Tree_token, "StructSpecifier")) {
        return sdt_StructSpecifier(root->Tree_child[0]);
    } else {
        // Shouldn't reach here!!!
        assert(0);
    }
}

Type sdt_StructSpecifier(TreeNode_t* root) {
    /* 
    * StructSpecifier -> STRUCT OptTag LC DefList RC
    * StructSpecifier -> STRUCT Tag
    */
    assert(root->num_child==2 || root->num_child==5);
    if(root->num_child == 2) {
        assert(root->Tree_child[1] != NULL)        
        char *name = sdt_Tag(root->Tree_child[1]);
        Type type = findType(name);
        if(type == NULL) {
            // TODO 报错
        }
        return type;        
    } else {
        Type type = myAlloc(sizeof(Type_t));
        type->kind = STRUCTURE;
        type->u.structure = NULL;

        Symbol sym = myAlloc(sizeof(Symbol_t));
        sym->type = type;
        sym->prev = sym->next = sym->area_prev = sym->area_next = NULL;

        if(root->Tree_child[1] == NULL) {
            // TODO 指定一个名字
        } else {
            strncpy(sym->name, sdt_OptTag(root->Tree_child[1]), 55);
        }
        if(root->Tree_child[3] != NULL)
            type->u.structure = sdt_DefList(root->Tree_child[3], 1);

        insertType(sym);
        return type;
    }
}

char* sdt_OptTag(TreeNode_t* root) {
    if(root->num_child == 0)
        return NULL;
    return root->Tree_child[0]->Tree_val;
}

char* Tag(TreeNode_t* root) {
    return root->Tree_child[0]->Tree_val;
}


/* Declarators */
Symbol sdt_VarDec(TreeNode_t* root, Type baseType, int size) {
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
        // TODO
        insertSymbol(sym);

        return sym;
    } else {
        assert(root->Tree_child[0] != NULL);
        return sdt_VarDec(root->Tree_child[0], baseType, size+1);
        
    }
}

Type FunDec(TreeNode_t* root, Type retType) {
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
    /*
    * ParamDec -> Specifier VarDec 
    */

    assert(root->num_child == 2);
    assert(root->Tree_child[0] != NULL && root->Tree_child[1] != NULL);

    Type type = sdt_Specifier(root->Tree_child[0]);
    Symbol sym = sdt_VarDec(root->Tree_child[1], type, 0);

    FieldList field = myAlloc(sizeof(FieldList_t));
    strncpy(field->name, sym->name, 55);
    field->type = sym->type;
    field->tail = NULL;
    // TODO 重名检测
    insertSymbol(sym);
}


/* Statements */

void sdt_CompSt(TreeNode_t* root, Type retType) {
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
        Type type = sdt_Exp(root->Tree_child[1]);
        if(!same_type(type, retType)) {
            // TODO 报错
        }
        return;
    }

    if(root->num_child == 5) {
        // Stmt -> IF LP Exp RP Stmt
        // Stmt -> WHILE LP Exp RP Stmt
        assert(root->Tree_child[2] != NULL);
        assert(root->Tree_child[4] != NULL);
        Type type = sdt_Exp(root->Tree_child[2]);
        if(not_int(type)) {
            // TODO 报错
        }
        sdt_Stmt(root->Tree_child[4], retType);
        return;
    }

    if(root->num_child == 7) {
        // Stmt -> IF LP Exp RP Stmt ELSE Stmt
        assert(root->Tree_child[2] != NULL);
        assert(root->Tree_child[4] != NULL);
        assert(root->Tree_child[6] != NULL);
        Type type = sdt_Exp(root->Tree_child[2]);
        if(not_int(type)) {
            // TODO 报错
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
        // TODO
        sdt_Def(root->Tree_child[0], inStruct);
        if(root->Tree_child[1] != NULL) {
            sdt_DefList(root->Tree_child[1], inStruct);
        }
        return NULL;
    }
}

FieldList sdt_Def(TreeNode_t* root, int inStruct) {
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
    /*
    * Dec -> VarDec
    * Dec -> VarDec ASSIGNOP Exp
    */

    assert(root->num_child == 1 || root->num_child == 3);
    
    assert(root->Tree_child[0] != NULL);
    Symbol sym = sdt_VarDec(root->Tree_child[0], baseType, 0);

    if(inStruct == 1) {
        FieldList field = myAlloc(sizeof(FieldList_t));
        field->tail = NULL;
        strncpy(field->name, sym->name, 55);
        field->type = sym->type;

        if(root->num_child == 3) {
            // TODO 报错：结构体里面不能赋值
        }
        return field;
    } else {
        // TODO 判断左右类型是否相同
    }

}


/* Expressions */

Type sdt_Exp(TreeNode_t* root) {
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
    * Exp -> Exp LB Exp Exp RB
    * Exp -> Exp DOT ID
    * Exp -> ID
    * Exp -> INT
    * Exp -> FLOAT
    */



}






/* Terminator */
char* sdt_ID(TreeNode_t* root) {
    return root->Tree_val;
}

int sdt_TYPE(TreeNode_t* root) {
    if(IS_EQUAL(root->Tree_token, "int"))
        return TYPE_INT;
    else if(IS_EQUAL(root->Tree_token, "float")) 
        return TYPE_FLOAT;
    else 
        assert(0);
    return -1;
}