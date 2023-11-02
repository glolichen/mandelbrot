#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <gmp.h>

#include "bstree.h"

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
BSTreeNode *bstree_add_node(BSTreeNode *node, mpf_t key, int value) {
	if (node == NULL) {
		BSTreeNode *newNode = (BSTreeNode *) malloc(sizeof(BSTreeNode));
		mpf_init_set(newNode->key, key);
		newNode->value = value;
		newNode->left = NULL, newNode->right = NULL;
		return newNode;
	}

	int cmp = mpf_cmp(key,node->key);
	if (cmp == 0)
		node->value = value;
	else if (cmp < 0)
		node->left = bstree_add_node(node->left, key, value);
	else
		node->right = bstree_add_node(node->right, key, value);

	return node;
}
void bstree_add(BSTree *tree, mpf_t key, int value) {
	tree->root = bstree_add_node(tree->root, key, value);
}

// key - match_range <= node->key <= key + match_range
int bstree_get_node(const BSTreeNode *node, const mpf_t min_accept, const mpf_t max_accept) {
	int cmpMin = mpf_cmp(node->key, min_accept);
	int cmpMax = mpf_cmp(node->key, max_accept);

	if (cmpMin > 0 && cmpMax < 0)
		return node->value;
	else if (has_left(node) && cmpMin < 0)
		return bstree_get_node(node->right, min_accept, max_accept);
	else if (has_right(node) && cmpMax > 0)
		return bstree_get_node(node->left, min_accept, max_accept);
	return -1;
}
int bstree_get(const BSTree *tree, mpf_t key, mpf_t match_range) {
	mpf_t minAccept, maxAccept;
	mpf_init_set(minAccept, key);
	mpf_init_set(maxAccept, key);
	mpf_sub(minAccept, minAccept, match_range);
	mpf_add(maxAccept, maxAccept, match_range);

	gmp_printf("[%.Ff, %.Ff]\n", minAccept, maxAccept);
	int answer = bstree_get_node(tree->root, minAccept, maxAccept);

	mpf_clear(minAccept);
	mpf_clear(maxAccept);

	return answer;
}