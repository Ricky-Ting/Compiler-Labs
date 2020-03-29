#include "symbol.h"
#include <stdio.h>

Symbol hashTable[HASH_TABLE_SZ];

struct {
    Symbol arr[MAX_RECUR];
    Symbol area_tail[MAX_RECUR];
    int top;
} stack;

unsigned int hash_pjw(char* name) {
    unsigned int val = 0, i;
    for (; *name; ++name) {
        val = (val<<2) + *name;
        if (i = val & ~HASH_TABLE_SZ)
            val = (val ^ (i >> 12) & HASH_TABLE_SZ);
    }
    return val;
}


void initSymbolTable() {
    for(int i=0; i<HASH_TABLE_SZ; ++i) {
        hashTable[i] = NULL;
    }
    stack.top = 0;
    //stack.area_tail[0] = NULL;
}

void insertSymbol(Symbol sym) {
    int val = hash_pjw(sym->name);
    
    // Insert into hashTable
    if(hashTable[val] == NULL) {
        hashTable[val] = sym;
        sym->next = NULL;
        sym->prev = NULL;
    } else {
        sym->next = hashTable[val];
        sym->prev = NULL;
        hashTable[val]->prev = sym;
        hashTable[val] = sym;
    }

    // Update stack
    int top = stack.top - 1;
    if(stack.arr[top] == NULL) {
        stack.arr[top] = sym;
        sym->area_prev = sym->area_next = NULL;
        stack.area_tail[top] = sym;
    } else {
        stack.area_tail[top]->area_next = sym;
        sym->area_prev = stack.area_tail;
        stack.area_tail[top] = sym;
    }
}


void stack_push() {
    stack.arr[stack.top] = NULL;
    stack.area_tail[stack.top] = NULL;
    stack.top++;
}

void stack_pop() {
    Symbol cur = stack.arr[stack.top-1];
    Symbol next = NULL;
    while(cur != NULL) {
        next = cur->area_next;
        if(cur->prev != NULL) {
            cur->prev->next = cur->next;
        }
        if(cur->next != NULL) {
            cur->next->prev = cur->prev;
        }
        freeSymbol(cur);
        cur = next;
    }
    stack.arr[stack.top-1] = NULL;
    stack.area_tail[stack.top-1] = NULL;
    stack.top--;
    return;
}

void freeSymbol(Symbol sym) {
    // TODO
    return ;
}

void *myAlloc(size_t sz) {
    void *ptr = (void *)malloc(sz);
    if(ptr == NULL)
        fprintf(stderr, "Need more memory!!!\n");
        assert(0);
    return ptr;
}