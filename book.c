#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <inttypes.h>
#include "common.h"

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

float your_text_width_calculation(nk_handle handle, float height, const char *text, int len) {
	return 10.0f;
}

char* tags[100];

/*
char* tags = {
	"Expenses:Auto:Gas",
	"Liabilities:MasterCard",
	"Assets:Credit"
	};
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
char commodity_list[256][4];

typedef struct {
	time_t date; // the date associated with the entry
	char* from;
	size_t from_len;
	short from_denom;
	long from_amount;
	char* to;
	size_t to_len;
	short to_denom;
	long to_amount;
} LedgerEntry;


time_t ledger_timestamp_from_ledger_date(const char* date_str) {
	// coneverts string 'YYYY-MM-DD' to unix timestamp
	// date_str should be exactly 10 characters
	assert(strlen(date_str) == 10);
	struct tm tm;
	tm.tm_year = 2023;//strtoi(date_str, date_str + 10, 0, 0, 3);
	printf("%d", tm.tm_year);
	return 0;
}

void ledger_parse_data(const char* text, size_t text_len) {
	time_t t = time(NULL);
	for(int c = 0; c < text_len; c++) {
		switch(text[c]) {
			case ';':
				printf("#");
				break;
			case ' ':
				printf(" ");
				break;
			case '	':
				printf("	");
				break;
			case '\n':
				printf(",\n");
				break;
			default:
				printf("*");
				break;
		}
	}
	return;
}

void* module_main(const char* data, size_t data_len) {
    printf("%s\n", data);

	ledger_parse_data(data, data_len);
}
