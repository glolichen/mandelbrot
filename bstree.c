#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <gmp.h>

#include "bstree.h"

int has_left(const GenericBSTreeNode *node) {
	if (node->t == REAL)
		return node->data.realNode->left != NULL;
	if (node->t == IMAG)
		return node->data.imagNode->left != NULL;
}
int has_right(const GenericBSTreeNode *node) {
	if (node->t == REAL)
		return node->data.realNode->right != NULL;
	if (node->t == IMAG)
		return node->data.imagNode->right != NULL;
}

GenericBSTreeValue get_value(const GenericBSTreeNode *node) {
	GenericBSTreeValue value;
	if (node->t == REAL) {
		value.t = IMAGNODE;
		value.data.imagNode = node->data.realNode->value;
		return value;
	}
	if (node->t == IMAG) {
		value.t = INT;
		value.data.integer = node->data.imagNode->value;
		return value;
	}
}
void set_value(GenericBSTreeNode *node, GenericBSTreeValue *value) {
	if (node->t == REAL)
		node->data.realNode->value = value->data.imagNode;
	if (node->t == IMAG)
		node->data.imagNode->value = value->data.integer;
}

void make_bstree(GenericBSTree *tree, mpf_t match_range) {
	tree->root = NULL;
	mpf_init_set(tree->matchRange, match_range);
}
GenericBSTreeNode *bstree_add_node(GenericBSTreeNode *node, mpf_t key, const GenericBSTreeValue *value, GenericBSTreeNode **added) {
	if (node == NULL) {
		if (value->t == IMAGNODE) {
			GenericBSTreeNode *newNode = (GenericBSTreeNode *) malloc(sizeof(GenericBSTreeNode));
			newNode->t = REAL;
			newNode->data.realNode = (RealBSTreeNode *) malloc(sizeof(RealBSTreeNode));
			mpf_init_set(newNode->data.realNode->key, key);
			newNode->data.realNode->value = value->data.imagNode;
			newNode->data.realNode->left = NULL;
			newNode->data.realNode->right = NULL;
			return newNode;
		}
		if (value->t == INT) {
			GenericBSTreeNode *newNode = (GenericBSTreeNode *) malloc(sizeof(GenericBSTreeNode));
			newNode->t = IMAG;
			newNode->data.imagNode = (ImagBSTreeNode *) malloc(sizeof(ImagBSTreeNode));
			mpf_init_set(newNode->data.imagNode->key, key);
			newNode->data.imagNode->value = value->data.integer;
			newNode->data.imagNode->left = NULL;
			newNode->data.imagNode->right = NULL;
			return newNode;
		}
	}
	if (node->data.realNode == NULL && node->data.imagNode == NULL) {
		if (node->t == REAL) {
			node->data.realNode = (RealBSTreeNode *) malloc(sizeof(RealBSTreeNode));
			mpf_init_set(node->data.realNode->key, key);
			node->data.realNode->value = value->data.imagNode;
			node->data.realNode->left = NULL;
			node->data.realNode->right = NULL;
			return node;
		} 
		if (node->t == IMAG) {
			node->data.imagNode = (ImagBSTreeNode *) malloc(sizeof(ImagBSTreeNode));
			mpf_init_set(node->data.imagNode->key, key);
			node->data.imagNode->value = value->data.integer;
			node->data.imagNode->left = NULL;
			node->data.imagNode->right = NULL;
			return node;
		}
	}

	if (node->t == REAL) {
		int cmp = mpf_cmp(key, node->data.realNode->key);
		if (cmp == 0)
			(*added) = node;
		else if (cmp < 0) {
			GenericBSTreeNode next = { .t = REAL, .data.realNode = node->data.realNode->left };
			node->data.realNode->left = bstree_add_node(&next, key, value, added)->data.realNode;
			if ((*added) == NULL) {
				(*added) = (GenericBSTreeNode *) malloc(sizeof(GenericBSTreeNode));
				(*added)->t = REAL;
				(*added)->data.realNode = node->data.realNode->left;
			}
		}
		else {
			GenericBSTreeNode next = { .t = REAL, .data.realNode = node->data.realNode->right };
			node->data.realNode->right = bstree_add_node(&next, key, value, added)->data.realNode;
			if ((*added) == NULL) {
				(*added) = (GenericBSTreeNode *) malloc(sizeof(GenericBSTreeNode));
				(*added)->t = REAL;
				(*added)->data.realNode = node->data.realNode->right;
			}
		}
	}
	if (node->t == IMAG) {
		int cmp = mpf_cmp(key, node->data.imagNode->key);
		if (cmp == 0)
			(*added) = node;
		else if (cmp < 0) {
			GenericBSTreeNode next = { .t = IMAG, .data.imagNode = node->data.imagNode->left };
			node->data.imagNode->left = bstree_add_node(&next, key, value, added)->data.imagNode;
			if ((*added) == NULL) {
				(*added) = (GenericBSTreeNode *) malloc(sizeof(GenericBSTreeNode));
				(*added)->t = IMAG;
				(*added)->data.imagNode = node->data.imagNode->left;
			}
		}
		else {
			GenericBSTreeNode next = { .t = IMAG, .data.imagNode = node->data.imagNode->right };
			node->data.imagNode->right = bstree_add_node(&next, key, value, added)->data.imagNode;
			if ((*added) == NULL) {
				(*added) = (GenericBSTreeNode *) malloc(sizeof(GenericBSTreeNode));
				(*added)->t = IMAG;
				(*added)->data.imagNode = node->data.imagNode->right;
			}
		}
	}

	return node;
}
GenericBSTreeNode *bstree_node_add(GenericBSTreeNode *node, mpf_t key, GenericBSTreeValue *value) {
	// node->data.imagNode = (ImagBSTreeNode *) malloc(sizeof(ImagBSTreeNode));
	GenericBSTreeNode *added = NULL;
	node = bstree_add_node(node, key, value, &added);
	if (added == NULL)
		added = node;
	return added;
}
GenericBSTreeNode *bstree_add(GenericBSTree *tree, mpf_t key, GenericBSTreeValue *value) {
	GenericBSTreeNode *added = NULL;
	tree->root = bstree_add_node(tree->root, key, value, &added);
	if (added == NULL)
		added = tree->root;
	return added;
}

// key - match_range <= node->key <= key + match_range
GenericBSTreeValue bstree_get_node(const GenericBSTreeNode *node, const mpf_t min_accept, const mpf_t max_accept) {
	if (node != NULL && node->t == REAL) {
		int cmpMin = mpf_cmp(node->data.realNode->key, min_accept);
		int cmpMax = mpf_cmp(node->data.realNode->key, max_accept);

		if (cmpMin > 0 && cmpMax < 0) {
			GenericBSTreeValue value = { .t = IMAGNODE, .data.imagNode = node->data.realNode->value };
			return value;
		}
		else if (has_left(node) && cmpMin < 0) {
			GenericBSTreeNode next = { .t = REAL, .data.realNode = node->data.realNode->left };
			return bstree_get_node(&next, min_accept, max_accept);
		}
		else if (has_right(node) && cmpMax > 0) {
			GenericBSTreeNode next = { .t = REAL, .data.realNode = node->data.realNode->right };
			return bstree_get_node(&next, min_accept, max_accept);
		}
	}
	if (node != NULL && node->t == IMAG) {
		int cmpMin = mpf_cmp(node->data.imagNode->key, min_accept);
		int cmpMax = mpf_cmp(node->data.imagNode->key, max_accept);

		if (cmpMin > 0 && cmpMax < 0) {
			GenericBSTreeValue value = { .t = IMAGNODE, .data.integer = node->data.imagNode->value };
			return value;
		}
		else if (has_right(node) && cmpMax > 0) {
			GenericBSTreeNode next = { .t = IMAG, .data.imagNode = node->data.imagNode->left };
			return bstree_get_node(&next, min_accept, max_accept);
		}
		else if (has_left(node) && cmpMin < 0) {
			GenericBSTreeNode next = { .t = IMAG, .data.imagNode = node->data.imagNode->right };
			return bstree_get_node(&next, min_accept, max_accept);
		}
	}

	GenericBSTreeValue none = { .t = NOTHING, .data.integer = -1, .data.imagNode = NULL };
	return none;
}
GenericBSTreeValue bstree_get(const GenericBSTree *tree, mpf_t key) {
	mpf_t minAccept, maxAccept;
	mpf_init_set(minAccept, key);
	mpf_init_set(maxAccept, key);
	mpf_sub(minAccept, minAccept, tree->matchRange);
	mpf_add(maxAccept, maxAccept, tree->matchRange);

	GenericBSTreeValue answer = bstree_get_node(tree->root, minAccept, maxAccept);

	mpf_clear(minAccept);
	mpf_clear(maxAccept);

	return answer;
}
void clear_bstree_node(GenericBSTreeNode *node) {
	if (node == NULL)
		return;
	if (node->t == REAL) {
		if (has_left(node)) {
			GenericBSTreeNode next = { .t = REAL, .data.realNode = node->data.realNode->left };
			clear_bstree_node(&next);
		}
		if (has_right(node)) {
			GenericBSTreeNode next = { .t = REAL, .data.realNode = node->data.realNode->right };
			clear_bstree_node(&next);
		}
		free(node->data.realNode);
	}
	if (node->t == IMAG) {
		if (has_left(node)) {
			GenericBSTreeNode next = { .t = IMAG, .data.imagNode = node->data.imagNode->left };
			clear_bstree_node(&next);
		}
		if (has_right(node)) {
			GenericBSTreeNode next = { .t = IMAG, .data.imagNode = node->data.imagNode->right };
			clear_bstree_node(&next);
		}
		free(node->data.imagNode);
	}
}
void clear_bstree(GenericBSTree *tree) {
	clear_bstree_node(tree->root);
}