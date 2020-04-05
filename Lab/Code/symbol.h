#ifndef SYMBOL_H
#define SYMBOL_H

#include <string.h>

#define HASH_TABLE_SZ 0x3fff
#define MAX_RECUR 20  // 作用域最大嵌套深度

typedef struct Type_* Type;
typedef struct Type_ Type_t;

typedef struct FieldList_* FieldList;
typedef struct FieldList_ FieldList_t;

typedef struct Symbol_* Symbol;
typedef struct Symbol_ Symbol_t;

//typedef FieldList TypeList;

struct Type_ {
    enum { BASIC, ARRAY, STRUCTURE, FUNC} kind;
    union {
        // 基本类型
        int basic;
        // 数组类型信息包括元素类型与数组大小构成
        struct { Type elem; int size; } array;
        // 结构体类型信息是一个链表
        FieldList structure;
        // 函数类型信息由返回值类型和参数类型构成
        struct { Type ret; FieldList params;} func;
    } u;
};

struct FieldList_ {
    char name[55]; // 域的名字
    Type type; // 域的类型
    FieldList tail; // 下一个域
};


struct Symbol_ {
    char name[55];
    Type type;
    int lineno;
    Symbol prev;
    Symbol next;
    Symbol area_prev;
    Symbol area_next;
};

#define IS_EQUAL(a,b) (strcmp(a, b) == 0)

void initTables();
void insertSymbol(Symbol sym);
void insertType(Symbol sym);
Symbol findSymbol(char* name);
Symbol findType(char* name);
int existSymbol(char* name);
void stack_push();
void stack_pop();
void *myAlloc(int sz);
//void freeSymbol(Symbol sym);
#endif