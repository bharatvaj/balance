#ifndef __PAYEREDO_ACCOUNT_H
#define __PAYEREDO_ACCOUNT_H

#include <string.h>
#include <assert.h>

#include <vstr.h>

struct map_tree;

struct map_tree {
	vstr_t *value;
	size_t children_cap;
	size_t children_len;
	struct map_tree* children;
};

typedef struct map_tree map_tree_t;

// acc: this:is:us
//root->|this|->|is|->|us|
//	->children
//			->children
//				  ->children

// TODO handle both rootp,this:is:us case and rootp->children,is:us case
// Currently only the rootp->value and acc are compared

map_tree_t *account_search(map_tree_t *rootp, char *acc, size_t acc_size);


int account_add(map_tree_t **rootp, char *acc, size_t acc_size);

#endif
