#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <inttypes.h>
#include <ctype.h>
#include <limits.h>
#include "common.h"
#include "strn.h"

#include <unistd.h>

#ifdef ENABLE_GUI
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
#endif

#define _XOPEN_SOURCE
#include <time.h>

#include "vstr.h"
#include <account.h>
#include <book.h>

#define BUFFER_SIZE 256

#define warning(STR) \
	fprintf(stdout, "\033[31m"STR"\033[0m");

#define warningf(STR,...) \
	fprintf(stdout, "\033[31m"STR"\033[0m", __VA_ARGS__);

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


// store numbers in the least denom
// 1.50$ == 150
// 2$ == 200
// 2.23$ == 200

typedef struct {
	vstr_t *denom;
	size_t amount;
} LedgerValue;

typedef struct {
	vstr_t *reg;
	LedgerValue val;
} LedgerPosting;

typedef struct {
	time_t date;
	vstr_t comment;
	LedgerPosting *postings;
} LedgerEntry;

time_t ledger_timestamp_from_ledger_date(char *date_str)
{
	/* converts string 'YYYY-MM-DD' to unix timestamp
	 * date_str should be exactly 10 characters
	 * printf("%.*s\n", 4, date_str); */
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
	"ENTRY SIGN",
	"ENTRY SIGN OR AMOUNT",
	"ENTRY AMOUNT",
	"ENTRY DENOM",
	"ENTRY DENOM OR AMOUNT",
	"ENTRY SIGN OR DENOM OR AMOUNT",
	"ENTRY END",
};

typedef enum {
	DATE,
	COMMENT,
	ENTRY_START, // entry starts after a comment
	ENTRY_SPACE,
	ENTRY_WHO,
	ENTRY_SIGN,
	ENTRY_SIGN_AMOUNT,
	ENTRY_AMOUNT,
	ENTRY_DENOM ,
	ENTRY_DENOM_AMOUNT,
	ENTRY_SIGN_DENOM_AMOUNT,
	ENTRY_END, // finish up entry if encountering any \n\n or \n text_len == i or text_len == i, otherwise set state to ENTRY_SPACE
	POSTING_END,
} LedgerParseStates;


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
	LedgerEntry he; /* hold entry */
	short hsign = -1;
	he.postings = (LedgerPosting *) calloc(1, sizeof(LedgerPosting) * 2);
	short entry_count = 0;

	short n_count = 0;

	while (i < text_len) {
		char c = text[i];
		// we use \n to identify entry done in ledger
		switch (c) {
			case '\n':
			case '\r':
				line_no++;
				n_count++;
				switch (state) {
					// after parsing the amount seq, we set the state to ENTRY_WHO
					case ENTRY_WHO:
					    he.postings[entry_count].val.amount	= LONG_MAX;
					case ENTRY_END:
						entry_count++;
						// if entry_count <= 1 throw error
						if (text[i - 1] == '\n') {
							state = DATE;
							// TODO push the entries to stack or somethin
							warning("\n==\n");
							// state = POSTING_END;
						} else {
							state = ENTRY_WHO;
							warning(",");
						}
						break;
					case COMMENT:
						state = ENTRY_START;
						break;
					case ENTRY_SIGN_DENOM_AMOUNT:
						state = ENTRY_WHO;
						break;
					case ENTRY_DENOM:
						//warningf("%s", "denom not found, setting state WHO");
						state = ENTRY_WHO;
						break;
					case ENTRY_AMOUNT:
						goto ledger_parse_error_handle;
						break;
					default:
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
					warningf("%.*s: %ld	", 10, text + i, tn);
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
					warningf("Comment: %.*s", (int)comment_len,
							comment.str);
					state = ENTRY_START;
				}
				break;
			case ENTRY_START:
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
					account_add(&rootp, who.str, who_len);
					warningf("\n\tl#%zu Who: %.*s ", i, (int)who_len, who.str);
					state = ENTRY_SIGN_DENOM_AMOUNT;
					// add to tags here
				}
				break;
			case ENTRY_SIGN_DENOM_AMOUNT:
				if (*(text + i) == '-' ) {
					// TODO throw already set error
					if (hsign >= 0) goto ledger_parse_error_handle;
					state = ENTRY_SIGN;
				} else if (isdigit(*(text + i))) state = ENTRY_AMOUNT;
				else state = ENTRY_DENOM;
				continue;
			case ENTRY_SIGN_AMOUNT:
				if (*(text + i) == '-' ) {
					// TODO throw already set error
					if (hsign >= 0) goto ledger_parse_error_handle;
					state = ENTRY_SIGN;
				} else if (isdigit(*(text + i))) state = ENTRY_AMOUNT;
				else goto ledger_parse_error_handle;
				break;
			case ENTRY_SIGN: {
				if (*(text + i) == '-') {
					   i++;
					   // AMOUNT cannot be set before SIGN
					   if (he.postings[entry_count].val.amount != LONG_MAX) goto ledger_parse_error_handle;
					   hsign = 1;
					   state = ENTRY_SIGN_DENOM_AMOUNT;
				}
			 } break;
			case ENTRY_DENOM: {
				char _c;
				char *denom = text + i;
				size_t denom_len = 0;
				while (i < text_len &&
						( isalpha(*(text + i))
						 || *(text + i) == '$')) i++;
				denom_len = (text + i) - denom;
				if (he.postings[entry_count].val.amount == LONG_MAX)
					state = hsign? ENTRY_AMOUNT: ENTRY_SIGN_AMOUNT;
				else
					state = ENTRY_END;
				warningf("D#%zu: %.*s ", denom_len, (int)denom_len, denom);
				break;
			}
			case ENTRY_AMOUNT: {
				char _c;
				char *amount = text + i;
				size_t amount_len = 0;
				while (i < text_len  &&  (_c = *(text + i)) == '.' || isdigit(_c) || _c == ',') i++;
				amount_len = (text + i) - amount;
				// TODO convert amount to he.postings[entry_count].amount integer
				he.postings[entry_count].val.amount = 0;
				//state = hold_denom_id == 0? ENTRY_DENOM : ENTRY_END;
				warningf("A#%zu: %.*s ", amount_len, (int)amount_len, amount);
				}
				break;
			default:
				goto ledger_parse_error_handle;
		}
	}
	warning("read complete\n");
	return;
ledger_parse_error_handle:
	warningf("\nParse failed at l#%ld 0x%zu, Expected a %s, got '%c'\n",
			line_no, i + 1, states_str[state], text[i]);
}
Entry** ledger_read_file(const char* filename, time_t date_start, time_t date_end) {
	Entity me = {"Account:Income"};
	// list population, read from
	FILE* file = fopen(filename, "r");
	if (file == NULL) {
		printf("Failed to open file %s\n", filename);
	}
	Entry** new_list = (Entry**)malloc(sizeof(Entry*) * 12);
	for(int i = 0; i < 12; i++) {
		Entry* entry = (Entry*)malloc(sizeof(Entry));
		new_list[i] = entry;
		entry->from = &me;
		entry->to = (Entity*)malloc(sizeof(Entity));
		entry->to->name = (char*)malloc(sizeof(char*) * 20);
		strcpy(entry->to->name, "Man");
	}
	return new_list;
}

void *module_main(char *data, size_t data_len)
{
	// printf("%s\n", data);
	warning("\n=======| Startality |=======\n");
	ledger_parse_data(data, data_len);
	warning("\n========| Fatality |========\n");
	return NULL;
}
