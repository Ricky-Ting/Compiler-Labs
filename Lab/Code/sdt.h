#ifndef SDT_H
#define SDT_H

#include "Tree.h"
#include "symbol.h"

struct expType_ {
    Type type;
    int size;  // 数组
    int var; // 是否可以作为左值
};

typedef struct expType_ expType_t;
typedef expType_t* expType;



#define TYPE_INT 0
#define TYPE_FLOAT 1



void sdt_init();

/* High-level Definitions */
void sdt_Program(TreeNode_t* root);
void sdt_ExtDefList(TreeNode_t *root);
void sdt_ExtDef(TreeNode_t *root);
void sdt_ExtDecList(TreeNode_t *root, Type baseType);


/* Specifiers */
Type sdt_Specifier(TreeNode_t *root);
Type sdt_StructSpecifier(TreeNode_t* root);
char* sdt_OptTag(TreeNode_t* root);
char* sdt_Tag(TreeNode_t* root);

/* Declarators */
Symbol sdt_VarDec(TreeNode_t* root, Type baseType, int size);
Symbol sdt_FunDec(TreeNode_t* root, Type retType);
FieldList sdt_VarList(TreeNode_t* root);
FieldList sdt_ParamDec(TreeNode_t* root);

/* Statements */
void sdt_CompSt(TreeNode_t* root, Type retType);
void sdt_StmtList(TreeNode_t* root, Type retType);
void sdt_Stmt(TreeNode_t* root, Type retType);

/* Local Definitions */
FieldList sdt_DefList(TreeNode_t* root, int inStruct);
FieldList sdt_Def(TreeNode_t* root, int inStruct);
FieldList sdt_DecList(TreeNode_t* root, Type baseType, int inStruct);
FieldList sdt_Dec(TreeNode_t* root, Type baseType, int inStruct);


/* Expressions */
expType_t sdt_Exp(TreeNode_t* root);
int sdt_Args(TreeNode_t* root, FieldList field);


/* Terminator */
char* sdt_ID(TreeNode_t* root);
int sdt_TYPE(TreeNode_t* root);


#endif