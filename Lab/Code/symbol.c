#include "symbol.h"
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>

Symbol symbolTable[HASH_TABLE_SZ]; // For symbol with variables
Symbol typeTable[HASH_TABLE_SZ]; // For struct type

/* 为了实现作用域 */
struct {
    Symbol arr[MAX_RECUR];
    Symbol area_tail[MAX_RECUR];
    int top;
} stack;

void initSymbolTable();
void initTypeTable();

/* 哈希函数 */
unsigned int hash_pjw(char* name) {
    unsigned int val = 0, i;
    for (; *name; ++name) {
        val = (val<<2) + *name;
        if (i = val & ~HASH_TABLE_SZ)
            val = (val ^ (i >> 12) & HASH_TABLE_SZ);
    }
    return val;
}

/* 初始化两张表 */
void initTables() {
    initSymbolTable();
    initTypeTable();
}


/* 初始化符号表 */
void initSymbolTable() {
    for(int i=0; i<HASH_TABLE_SZ; ++i) {
        symbolTable[i] = NULL;
    }
    stack.top = 0;
    //stack.area_tail[0] = NULL;
}

/* 初始化类型表 */
void initTypeTable() {
    for(int i=0; i<HASH_TABLE_SZ; ++i) {
        typeTable[i] = NULL;
    }
}

/* 插入符号表 */
void insertSymbol(Symbol sym) {
    // 确定在哪个slot
    int slot = hash_pjw(sym->name);
    
    if(symbolTable[slot] == NULL) {
        // 当前slot为空，直接放在第一格就行
        symbolTable[slot] = sym;
        sym->next = NULL;
        sym->prev = NULL;
    } else {
        // 当前slot不为空，插在链表的最前面
        sym->next = symbolTable[slot];
        sym->prev = NULL;
        symbolTable[slot]->prev = sym;
        symbolTable[slot] = sym;
    }

    // 更新栈
    int top = stack.top - 1;
    if(stack.arr[top] == NULL) {
        // 当前作用域里的第一个符号
        stack.arr[top] = sym;
        sym->area_prev = sym->area_next = NULL;
        stack.area_tail[top] = sym;
    } else {
        // 当前作用域里已经有其他的符号表, 更新tail
        stack.area_tail[top]->area_next = sym;
        sym->area_prev = stack.area_tail[top];
        stack.area_tail[top] = sym;
    }
}

/* 插入类型表 */
void insertType(Symbol sym) {
    // 确定在哪个slot
    int slot = hash_pjw(sym->name);
    
    if(typeTable[slot] == NULL) {
        // 当前slot为空，直接放在第一格就行
        typeTable[slot] = sym;
        sym->next = NULL;
        sym->prev = NULL;
    } else {
        // 当前slot不为空，插在链表的最前面
        sym->next = typeTable[slot];
        sym->prev = NULL;
        typeTable[slot]->prev = sym;
        typeTable[slot] = sym;
    }
}

Symbol findSymbol(char* name) {
    int slot = hash_pjw(name);
    Symbol cur = symbolTable[slot];
    while(cur != NULL && !IS_EQUAL(name, cur->name)) {
        cur = cur->next;
    }
    return cur;
}

Symbol findType(char* name) {
    int slot = hash_pjw(name);
    Symbol cur = typeTable[slot];
    while(cur != NULL && !IS_EQUAL(name, cur->name)) {
        cur = cur->next;
    }
    return cur;
}

/* 带作用域 */
int existSymbol(char* name) {
    int top = stack.top - 1;
    int slot = hash_pjw(name);
    Symbol cur = typeTable[slot];
    while(cur != NULL && !IS_EQUAL(name, cur->name)) {
        cur = cur->next;
    }

    if(cur == NULL)
        return 0;
    
    while(cur != NULL) {
        cur = cur->area_prev;
    }
    if(cur == stack.arr[top]) {
        return 1;
    } else {
        return 0;
    }

}


/* 压入新的作用域 */
void stack_push() {
    stack.arr[stack.top] = NULL;
    stack.area_tail[stack.top] = NULL;
    stack.top++;
}

/* 弹出作用域 */
void stack_pop() {
    int top = stack.top - 1;
    Symbol cur = stack.arr[top];
    Symbol next = NULL;
    while(cur != NULL) {
        next = cur->area_next;
        int slot = hash_pjw(cur->name);
        if(cur->prev != NULL) {
            cur->prev->next = cur->next;
        } else {
            symbolTable[slot] = cur->next; 
        }
        if(cur->next != NULL) {
            cur->next->prev = cur->prev;
        }
        //freeSymbol(cur);
        cur = next;
    }
    stack.arr[top] = NULL;
    stack.area_tail[top] = NULL;
    stack.top--;
    return;
}

/* 释放符号 */
void freeSymbol(Symbol sym) {
    // TODO
    return ;
}

/* 封装了malloc, 在内存不足的时候报错 */
void *myAlloc(int sz) {
    void *ptr = (void *)malloc(sz);
    if(ptr == NULL)
        fprintf(stderr, "Need more memory!!!\n");
        assert(0);
    return ptr;
}