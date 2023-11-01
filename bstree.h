#ifndef BSTREE_H
#define BSTREE_H

#include <gmp.h>

typedef struct BSTreeNode {
	mpf_t key;
	int value;
	struct BSTreeNode *left, *right;
} BSTreeNode;

// int has_left(const BSTreeNode *node);
// int has_right(const BSTreeNode *node);

// BSTreeNode *get_left(const BSTreeNode *node);
// BSTreeNode *get_right(const BSTreeNode *node);

// void set_left(BSTreeNode *node, BSTreeNode *add);
// void set_right(BSTreeNode *node, BSTreeNode *add);

// void get_key(mpf_t out, const BSTreeNode *node);
// void set_key(BSTreeNode *node, mpf_t key);

// int get_value(const BSTreeNode *node);
// void set_value(BSTreeNode *node, int value);

typedef struct {
	BSTreeNode *root;
} BSTree;

void make_bstree(BSTree *tree);
void bstree_add(BSTree *tree, mpf_t key, int value);
int bstree_get(const BSTree *tree, mpf_t key);

#endif