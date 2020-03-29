
#ifndef TREE_H
#define TREE_H
struct TreeNode {
    char Tree_token[55];
    char Tree_val[55];
    int Tree_lineno;
    unsigned int val_UINT;
    float val_FLOAT;
    int num_child;
    struct TreeNode* Tree_child[10];
};

typedef struct TreeNode TreeNode_t;

TreeNode_t* newTreeNode(char* Tree_token, char* Tree_val, int Tree_lineno);
void insertTreeNode(TreeNode_t* root, TreeNode_t* child);
void insertTreeNode_UINT(TreeNode_t* root, unsigned int val);

void insertTreeNode_FLOAT(TreeNode_t* root, double val);
void printTree(TreeNode_t* root, int space);
#endif