#include <stddef.h>
#include <stdio.h>
#include <gmp.h>

#include "bstree.h"

void make_bstree_node(BSTreeNode *node, mpf_t key, int value) {
	mpf_init_set(node->key, key);
	node->value = value;
	node->left = NULL, node->right = NULL;
}
int has_left(const BSTreeNode *node) {
	return node->left != NULL;
}
int has_right(const BSTreeNode *node) {
	return node->left != NULL;
}

BSTreeNode *get_left(const BSTreeNode *node) {
	return node->left;
}
BSTreeNode *get_right(const BSTreeNode *node) {
	return node->right;
}

void set_left(BSTreeNode *node, BSTreeNode *add) {
	node->left = add;
}
void set_right(BSTreeNode *node, BSTreeNode *add) {
	node->right = add;
}

void get_key(mpf_t out, const BSTreeNode *node) {
	mpf_set(out, node->key);
}
void set_key(BSTreeNode *node, mpf_t key) {
	mpf_set(node->key, key);
}

int get_value(const BSTreeNode *node) {
	return node->value;
}
void set_value(BSTreeNode *node, int value) {
	node->value = value;
}

void make_bstree(BSTree *tree) {
	tree->root = NULL;
}
void bstree_add_node(BSTreeNode **node, mpf_t key, int value) {
	if (*node == NULL) {
		BSTreeNode newNode;
		make_bstree_node(&newNode, key, value);
		*node = &newNode;
		return;
	}

	int cmp = mpf_cmp((*node)->key, key);
	if (cmp == 0)
		(*node)->value = value;
	else if (cmp < 0)
		bstree_add_node(&(*node)->left, key, value);
	else
		bstree_add_node(&(*node)->right, key, value);
}
void bstree_add(BSTree *tree, mpf_t key, int value) {
	bstree_add_node(&tree->root, key, value);
}

int bstree_get_node(const BSTreeNode *node, mpf_t key) {
	// gmp_printf("%.Ff\n", node->key);
	if (node == NULL)
		return 0;
	int cmp = mpf_cmp(node->key, key);
	if (cmp == 0)
		return node->value;
	else if (has_left(node) && cmp < 0)
		return bstree_get_node(node->left, key);
	else if (has_right(node))
		return bstree_get_node(node->right, key);
	return -1;
}
int bstree_get(const BSTree *tree, mpf_t key) {
	bstree_get_node(tree->root, key);
}