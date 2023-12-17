#include <stdio.h>
#include <stdlib.h>

#include <account.h>

/* TEST_INPUT
this:is:a:account
virtualaccount:
virtualaccount
this:is:
TEST_INPUT */

void account_walk (map_tree_t* rootp)
{
	static int tab_acc;
	if (rootp == NULL) return;
	for(int i = 0; i < tab_acc; i++) {
		printf("  ");
	}
	vstr_t *val = rootp->value;
	if (val != NULL) {
		printf("%.*s\n", val->len, val->str);
		tab_acc++;
	}
	for (int i = 0; i < rootp->children_len; i++) {
		account_walk(rootp->children + i);
	}
	tab_acc--;
}


int main(int argc, char* argv[]) {
	map_tree_t* account_tree = NULL;
	for(int i = 1; i < argc; i++) {
		account_add(&account_tree, argv[i], strlen(argv[i]));
	}
	account_walk(account_tree);
}

/* TEST_OUTPUT
this:
  is:
    a:
      account
virtualaccount:
virtualaccount
TEST_OUTPUT */
