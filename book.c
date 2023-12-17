#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <inttypes.h>
#include <ctype.h>
#include <limits.h>
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

#include "vstr.h"
#include "account.h"
#include "book.h"

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
	time_t hold_date;
	vstr_t hold_comment = { 0 };
	vstr_t hold_register = { 0 };
	long int hold_amount = LONG_MAX;
	short hold_sign = -1;
	size_t hold_denom_id = { 0 };
	short n_count = 0;

	while (i < text_len) {
		char c = text[i];
		// we use \n to identify entry done in ledger
		switch (c) {
			case '\n':
			case '\r':
				line_no++;
				n_count++;
				printf("\n%d| ", line_no);
				switch (state) {
					// after parsing the amount seq, we set the state to ENTRY_WHO
					case ENTRY_WHO:
					case ENTRY_END:
						warning("----- Entry End Marked -----\n");
						hold_sign = -1;
						hold_amount = LONG_MAX;
						// if entry_count <= 1 throw error
						if (text[i - 1] == '\n') {
							state = DATE;
							// TODO push the entries to stack or somethin
							warning("----- Posting End Marked -----\n");
							// state = POSTING_END;
						} else {
							state = ENTRY_WHO;
						}
						break;
					case COMMENT:
					case ENTRY_SIGN_DENOM_AMOUNT:
						state = ENTRY_WHO;
						break;
					case ENTRY_DENOM:
						warningf("%s\n", "denom not found, setting state WHO");
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
					warningf("date str: %.*s\n", 10, text + i);
					warningf("date: %ld\n", tn);
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
					warningf("Comment: %.*s\n", comment_len,
							comment);
					state = ENTRY_WHO;
				}
				break;
			case ENTRY_SPACE:
				{
					size_t original_i = i;
					while (i < text_len && isspace(text[i])) i++;
					int wsc = i - original_i;
					warningf("i: %ld, Spaces: %d\n", i, wsc);
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
					warningf("parsed: i=%d\n", i);
					account_add(&rootp, who.str, who_len);
					warningf("i=%d, Who: %.*s\n", i, who_len, who);
					state = ENTRY_SIGN_DENOM_AMOUNT;
					// add to tags here
				}
				break;
			case ENTRY_SIGN_DENOM_AMOUNT:
				if (*(text + i) == '-' ) {
					// TODO throw already set error
					if (hold_sign >= 0) goto ledger_parse_error_handle;
					state = ENTRY_SIGN;
				} else if (isdigit(*(text + i))) state = ENTRY_AMOUNT;
				else state = ENTRY_DENOM;
				continue;
			case ENTRY_SIGN_AMOUNT:
				if (*(text + i) == '-' ) {
					// TODO throw already set error
					if (hold_sign >= 0) goto ledger_parse_error_handle;
					state = ENTRY_SIGN;
				} else if (isdigit(*(text + i))) state = ENTRY_AMOUNT;
				else goto ledger_parse_error_handle;
				break;
			case ENTRY_SIGN: {
				if (*(text + i) == '-') {
					   i++;
					   // AMOUNT cannot be set before SIGN
					   if (hold_amount != LONG_MAX) goto ledger_parse_error_handle;
					   hold_sign = 1;
					   state = ENTRY_SIGN_DENOM_AMOUNT;
				}
			 } break;
			case ENTRY_DENOM: {
				char _c;
				warningf("denom-i: %d\n", i + 1);
				char *denom = text + i;
				size_t denom_len = 0;
				while (i < text_len &&
						( isalpha(*(text + i))
						 || *(text + i) == '$')) i++;
				denom_len = (text + i) - denom;
				if (hold_amount == LONG_MAX)
					state = hold_sign? ENTRY_AMOUNT: ENTRY_SIGN_AMOUNT;
				else
					state = ENTRY_END;
				warningf("%d> len: %d, denom: %.*s\n", i, denom_len, denom_len, denom);
				break;
			}
			case ENTRY_AMOUNT: {
				char _c;
				warningf("amount-i: %d\n", i + 1);
				char *amount = text + i;
				size_t amount_len = 0;
				while (i < text_len  &&  (_c = *(text + i)) == '.' || isdigit(_c) || _c == ',') i++;
				amount_len = (text + i) - amount;
				// TODO convert amount to hold_amount integer
				hold_amount = 0;
				state = hold_denom_id == 0? ENTRY_DENOM : ENTRY_END;
				warningf("%d> len: %d, amount: %.*s\n", i, amount_len, amount_len, amount);
				}
				break;
			default:
				goto ledger_parse_error_handle;
		}
	}
	warning("read complete\n");
	return;
ledger_parse_error_handle:
	warningf("Parse failed at %ld b:(%d), Expected %s, got '%c'",
			line_no, i, states_str[state], text[i]);
}

int main(int argc, char* argv[]) {
	FILE* in = fopen("october-2023.txt", "r");
	char* data = (char*)malloc(2048 * sizeof(char));
	size_t data_size = 0;
	size_t c_read =  0;
	while((c_read = fread(data + data_size + 0, 1, BUFFER_SIZE, in)) != 0) {
		data_size += c_read;
	}
	if (ferror(in)) fprintf(stderr, "Error reading file\n");
	fprintf(stdout, "Startig loop\n");
	ledger_parse_data(data, data_size);
	return 0;
}

void *module_main(char *data, size_t data_len)
{
	// printf("%s\n", data);
	warning("\n=======| Startality |=======\n");
	ledger_parse_data(data, data_len);
	warning("\n========| Fatality |========\n");
}
