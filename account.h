#ifndef __PAYEREDO_ACCOUNT_H
#define __PAYEREDO_ACCOUNT_H

#include <string.h>
#include <assert.h>

#include <vstr.h>

size_t tree_depth = 4;

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
map_tree_t *account_search(map_tree_t *rootp, char *acc, size_t acc_size)
{
	assert(rootp != NULL);
	// we hit leaf node, return rootp
	if (rootp->children == NULL) return rootp;

	//  return rootp when the 'acc' matches exactly with rootp->value
	//  acc: this, rootp->value: this
	vstr_t *rk = rootp->value;
	if (rk != NULL && acc_size == rk->len && (strncmp(acc, rk->str, acc_size) == 0)) {
		return rootp;
	}

	// search the string in it's children
	for (size_t i = 0; i < rootp->children_len; i++) {
		vstr_t *val = rootp->children[i].value;
		if (val != NULL && acc_size == val->len && (strncmp(acc, val->str, acc_size) == 0)) {
			return rootp->children + i;
		}
	}
	return NULL;
}

int account_add(map_tree_t **rootp, char *acc, size_t acc_size)
{
	size_t records_needed = tree_depth * 4;
	if (*rootp == NULL) {
		*rootp = malloc(sizeof(map_tree_t));
	}
	if ((*rootp)->children == NULL) {
		(*rootp)->children =
			(map_tree_t *) calloc(records_needed, sizeof(map_tree_t));
		(*rootp)->children_len = 0;
		(*rootp)->children_cap = records_needed;
	}
	map_tree_t* _rootp = *rootp;
	size_t i = 0;
	while (i < acc_size) {
		if (acc[i] == ':' || i + 1 == acc_size) {
			size_t j = i + 1;
			map_tree_t *current_node = account_search(_rootp, acc, j);
			if (current_node == NULL) {
				// return the previously allocated child
				current_node = _rootp->children + _rootp->children_len++;
				// current_node->value is NULL when the search fails
				// we have to set the value now
				// TODO maybe save vstrs in a pool and use them, would provide a sane way to free memory
				vstr_t *vstr = (vstr_t *) malloc(sizeof(vstr_t));
				vstr->str = acc;
				vstr->len = j;
				current_node->value = vstr;
				//printf("%zu : %zu %d %.*s\n", current_node, vstr, j, j, acc);
			} else {
				//printf("Present already= %d %.*s\n", j, j, acc);
			}
			if (j != acc_size) {
				return account_add(&current_node, acc + j,
						acc_size - j);
			}
		}
		i++;
	}
	return -1;
}

#endif
