#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "Tree.h"



TreeNode_t* newTreeNode(char* Tree_token, char* Tree_val, int Tree_lineno) {
    TreeNode_t* ret = malloc(sizeof(TreeNode_t));
    if(ret == NULL) {
        fprintf(stderr, "Need more memory!!\n");
        assert(0);
    }
    //ret->Tree_token = Tree_token;
    //ret->Tree_val = Tree_val;
    strncpy(ret->Tree_token, Tree_token, 80);
    if(Tree_val != NULL)
        strcpy(ret->Tree_val, Tree_val);
    ret->Tree_lineno = Tree_lineno;
    ret->num_child = 0;
    return ret;
}


void insertTreeNode(TreeNode_t* root, TreeNode_t* child) {
    // Modified for Lab2
    //if(child != NULL) 
        root->Tree_child[root->num_child++] = child;
    return;
}

void insertTreeNode_UINT(TreeNode_t* root, unsigned int val) {
    TreeNode_t* node = newTreeNode("INT", NULL,  0);
    node->val_UINT = val;
    root->Tree_child[root->num_child++] = node;
    return;
}

void insertTreeNode_FLOAT(TreeNode_t* root, double val) {
    TreeNode_t* node = newTreeNode("FLOAT", NULL,  0);
    node->val_FLOAT = val;
    root->Tree_child[root->num_child++] = node;
    return;
}

void printTree(TreeNode_t* root, int space) {
    if(root == NULL)
        return;
    for(int i=0; i<space; ++i) {
        printf(" ");
    }
    
    if(root->Tree_lineno != 0) {
        printf("%s (%d)\n", root->Tree_token, root->Tree_lineno);
    } else {
        if(strcmp(root->Tree_token, "INT") == 0) {
            printf("%s: %u\n", root->Tree_token, root->val_UINT);
        } else if(strcmp(root->Tree_token, "FLOAT") == 0) {
            printf("%s: %f\n", root->Tree_token, root->val_FLOAT);
        } else if(strcmp(root->Tree_token, "TYPE") == 0 || strcmp(root->Tree_token, "ID") == 0) {
            printf("%s: %s\n", root->Tree_token, root->Tree_val);
        } else {
            printf("%s\n", root->Tree_token);
        }
    }

    for(int i=0; i<root->num_child; ++i) {
        printTree(root->Tree_child[i], space+2);
    }
    return;
}


