#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <inttypes.h>
#include <ctype.h>
#include "common.h"
#include "strn.h"

#include <unistd.h>

#include <GLFW/glfw3.h>
#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#define NK_IMPLEMENTATION
#define NK_GLFW_GL2_IMPLEMENTATION
#define NK_KEYSTATE_BASED_INPUT
#include <nuklear.h>
#include <nuklear_glfw_gl2.h>

#define _XOPEN_SOURCE
#include <time.h>

#include "book.h"

#define warning(STR,...) \
	fprintf(stdout, "\033[31m"STR"\033[0m", __VA_ARGS__);

typedef struct {
	char *str;
	size_t len;
} vstr_t;

size_t tree_depth = 4;

struct map_tree;

struct map_tree {
	vstr_t *value;
	size_t children_cap;
	size_t children_len;
	struct map_tree *children;
};

typedef struct map_tree map_tree_t;

vstr_t tags[100] = { 0 };

size_t tags_size = 0;

/*
 * char* tags = { "Expenses:Auto:Gas", "Liabilities:MasterCard",
 * "Assets:Credit" };
 */

/**
 *
 read the file | parse for newlines
 2023/10/24 - 5, fptr + 0, fptr + 6, fptr + 9
 Entry* entry = get_entry("2023/10/24");

 skip \n
 read until date, and read until from

 **/

// commodity max size 256
// commodity name max size 4
char commodity_list[256][8];

map_tree_t *rootp = NULL;

typedef struct {
	vstr_t *denom;
	int amount;
} LedgerValue;

typedef struct {
	vstr_t *reg;
	LedgerValue val;
} LedgerRecord;

typedef struct {
	time_t date;		// the date associated with the entry
	vstr_t comment;		// comment associated with the entry
	LedgerRecord **records;
} LedgerEntry;

time_t ledger_timestamp_from_ledger_date(char *date_str)
{
	// converts string 'YYYY-MM-DD' to unix timestamp
	// date_str should be exactly 10 characters
	// printf("%.*s\n", 4, date_str);
	struct tm tm = { 0 };
	tm.tm_year = natoi(date_str, 4) - 1900;
	tm.tm_mon = natoi(date_str + 5, 2) - 1;
	tm.tm_mday = natoi(date_str + 8, 2);
	// warning("tm_year %d, tm_mon: %d, tm_mday: %d", natoi(date_str, 4),
	// tm.tm_mon, tm.tm_mday);
	return mktime(&tm);
}

const char *states_str[] = {
	"DATE",
	"COMMENT",
	"ENTRY START",
	"ENTRY SPACE",
	"ENTRY WHO",
	"ENTRY DENOM",
	"ENTRY AMOUNT",
	"ENTRY END",
};

typedef enum {
	DATE,
	COMMENT,
	ENTRY_START, // entry starts after a comment
	ENTRY_SPACE,
	ENTRY_WHO,
	ENTRY_DENOM,
	ENTRY_AMOUNT,
	ENTRY_END, // finish up entry if encountering any \n\n or \n text_len == i or text_len == i, otherwise set state to ENTRY_SPACE
} LedgerParseStates;

map_tree_t *account_search(map_tree_t *children, char *acc, size_t acc_size)
{
	if (children->children == NULL)
		return children;
	vstr_t *rk = children->value;
	if (rk != NULL && acc_size == rk->len
			&& (strncmp(acc, rk->str, acc_size) == 0)) {
		return children;
	}
	for (size_t i = 0; i < children->children_len; i++) {
		vstr_t *val = children->children[i].value;
		if (val != NULL && acc_size == val->len
				&& (strncmp(acc, val->str, acc_size) == 0)) {
			return children->children + i;
		}
	}
	// when the search is exhausted and nothing is found,
	// return the previously allocated child
	// TODO if len < cap allocate memory
	map_tree_t *child_to_return =
		children->children + children->children_len;
	children->children_len++;
	return child_to_return;
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
		(*rootp)->children_cap = records_needed;
	}
	size_t i = 0;
	while (i < acc_size) {
		if (acc[i] == ':' || i + 1 == acc_size) {
			size_t j = i + 1;
			map_tree_t *current_node =
				account_search(*rootp, acc, j);
			assert(current_node != NULL);
			if (current_node->value == NULL) {
				// current_node->value is NULL when the search fails
				// we have to set the value now
				// TODO maybe save vstrs in a pool and use them
				vstr_t *vstr =
					(vstr_t *) malloc(sizeof(vstr_t));
				vstr->str = acc;
				vstr->len = j;
				current_node->value = vstr;
				printf("%d %.*s\n", j, j, acc);
			} else {
				printf("Present already= %d %.*s\n", j, j, acc);
			}
			if (i + 1 != acc_size) {
				return account_add(&(*rootp)->children, acc + j,
						acc_size - j);
			}
		}
		i++;
	}
	return -1;
}

size_t tab_acc = 0;

void walk_it (map_tree_t* rootp)
{
	if (rootp == NULL)
		return;
	vstr_t *val = rootp->value;
	if (val != NULL) {
		for (size_t i = 0; i < tab_acc; i++) {
			printf("\t");
		}
		printf("-|%.*s|-\n", val->len, val->str);
	}
	tab_acc++;
	if (rootp->children == NULL)
		return;
	for (int i = 0; i < rootp->children_len; i++) {
		printf("|", i);
		walk_it(rootp->children + i);
	}
	tab_acc--;
}

void ledger_parse_data(char *text, size_t text_len)
{
	setvbuf(stdout, NULL, _IONBF, 0);
	LedgerParseStates state = DATE;
	size_t line_no = 1;
	size_t current_column = 1;
	time_t t = time(NULL);
	// printf("1|");
	size_t i = 0;
	// TODO it may be possible to push these to the tree itself, explore the possibility
	// these act as temporary register until we push back the entry to a tree
	time_t hold_date;
	vstr_t hold_comment = { 0 };
	vstr_t hold_register = { 0 };
	size_t hold_denom_id = { 0 };
	short n_count = 0;

	while (i < text_len) {
		char c = text[i];
		// we use \n to identify entry done in ledger
		//
		switch (c) {
			case '\n':
			case '\r':
				line_no++;
				n_count++;
				printf("\n%d| ", line_no);
				switch (state) {
					case ENTRY_WHO:
					case ENTRY_END:
						// push the entries to stack or somethin
						printf("case ENTRY_END: '%c', prev: '%c'\n", c, text[i-1]);
						if (text[i - 1] == '\n') {
							printf("----- Entry End Marked -----\n");
							state = DATE;
						} else {
							state = ENTRY_WHO;
						}
						break;
					case COMMENT:
						state = ENTRY_WHO;
						break;
					case ENTRY_DENOM:
						warning("%s\n", "denom not found, setting state WHO");
						state = ENTRY_WHO;
						break;
					case ENTRY_AMOUNT:
						goto ledger_parse_error_handle;
						break;
				}
				i++;
				continue;
			case ' ':
			case '\t':
				n_count = 0;
				i++;
				continue;
				break;
		}
		n_count = 0;
		// next state
		switch (state) {
			case DATE:
				if (isdigit(c)) {
					// try to parse a date
					time_t tn = ledger_timestamp_from_ledger_date(text + i);
					warning("date str: %.*s\n", 10, text + i);
					warning("date: %ld\n", tn);
					// date is expected to have the form DD/MM/YYYY (10)
					i += 10;
					if (tn == (time_t) - 1) goto ledger_parse_error_handle;
					state = COMMENT;
				}
				break;
			case COMMENT:
				if (isalnum(c)) {
					// we hit alphanumerical after whitespace
					size_t comment_len = 0;
					vstr_t comment = {
						.str = text + i,
						.len = 0
					};
					while (i < text_len && *(text + i) != '\n') {
						i++;
						comment_len++;
					}
					comment.len = comment_len;
					warning("Comment: %.*s\n", comment_len,
							comment);
					state = ENTRY_WHO;
				}
				break;
			case ENTRY_SPACE:
				{
					size_t original_i = i;
					while (i < text_len && isspace(text[i])) i++;
					int wsc = i - original_i;
					warning("i: %ld, Spaces: %d\n", i, wsc);
					if (wsc < 2) {
						goto ledger_parse_error_handle;
					}
					state = ENTRY_WHO;
				}
				break;
			case ENTRY_WHO:
				{
					// add this to register
					size_t who_len = i;
					vstr_t who = {
						.str = text + i,
						.len = 0
					};
					while (i < text_len) {
						switch (text[i]) {
							case '\n':
							case '\r':
								goto ledger_who_parsed;
								break;
							case '\t':
							case ' ':
								goto ledger_who_parsed;
								break;
							default:
								i++;
								break;
						}
					}
ledger_who_parsed:
					who_len = i - who_len;
					printf("parsed: i=%d\n", i);
					account_add(&rootp, who.str, who_len);
					warning("i=%d, Who: %.*s\n", i, who_len, who);
					state = ENTRY_DENOM;
					// add to tags here
				}
				break;
			case ENTRY_DENOM:
				{
					warning("denom-i: %d\n", i + 1);
					size_t denom_len = i;
					vstr_t denom = {
						.str = text + i,
						.len = 0
					};
					while (i < text_len && !isdigit(*(text + i)))
						i++;
					state = ENTRY_AMOUNT;
					denom_len = i - denom_len;
					denom.len = denom_len;
					warning("len: %d, denom: %.*s, i: %d\n", denom_len, denom_len, denom.str, i);
				}
				break;
			case ENTRY_AMOUNT:
				{
					warning("amount-i: %d\n", i + 1);
					char *amount = text + i;
					size_t amount_len = i;
					char _c = *(text + i);
					while (i < text_len && (isdigit(_c) || _c == '.' || _c == ',')) {
						i++;
						_c = *(text + i);
					}
					state = ENTRY_END;
					amount_len = i - amount_len;
					warning("%d> len: %d, amount: %.*s\n", i, amount_len, amount_len, amount);
				}
		}
	}
	printf("read complete\n");
	return;
ledger_parse_error_handle:
	warning("Parse failed at line %ld(%d)\n, Expected %s, got '%c'",
			line_no, i, states_str[state], text[i]);
}

void *module_main(char *data, size_t data_len)
{
	// printf("%s\n", data);
	printf("\n=======| Startality |=======\n");
	ledger_parse_data(data, data_len);
	printf("\n========| Fatality |========\n");
}
