#ifndef BSTREE_H
#define BSTREE_H

#include <gmp.h>

typedef struct ImagBSTreeNode {
	mpf_t key;
	int value;
	struct ImagBSTreeNode *left, *right;
} ImagBSTreeNode;

typedef struct RealBSTreeNode {
	mpf_t key;
	ImagBSTreeNode *value;
	struct RealBSTreeNode *left, *right;
} RealBSTreeNode;

typedef enum { INT, IMAGNODE, NOTHING } ValueDataType;
typedef struct {
	ValueDataType t;
	union {
		int integer;
		ImagBSTreeNode *imagNode;
	} data;
} GenericBSTreeValue;

typedef enum { REAL, IMAG } NodeDataType;
typedef struct {
	NodeDataType t;
	union {
		RealBSTreeNode *realNode;
		ImagBSTreeNode *imagNode;
	} data;
} GenericBSTreeNode;

typedef struct {
	mpf_t matchRange;
	GenericBSTreeNode *root;
} GenericBSTree;

void make_bstree(GenericBSTree *tree, mpf_t match_range);
GenericBSTreeNode *bstree_node_add(GenericBSTreeNode *node, mpf_t key, GenericBSTreeValue *value);
GenericBSTreeNode *bstree_add(GenericBSTree *tree, mpf_t key, GenericBSTreeValue *value);
GenericBSTreeValue bstree_get(const GenericBSTree *tree, mpf_t key);
void clear_bstree(GenericBSTree *tree);

#endif