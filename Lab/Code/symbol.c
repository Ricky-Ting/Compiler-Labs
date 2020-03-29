#include "symbol.h"

Symbol hashTable[HASH_TABLE_SZ];

unsigned int hash_pjw(char* name) {
    unsigned int val = 0, i;
    for (; *name; ++name) {
        val = (val<<2) + *name;
        if (i = val & ~HASH_TABLE_SZ)
            val = (val ^ (i >> 12) & HASH_TABLE_SZ);
    }
    return val;
}

