#ifndef _LEDGER_BOOK_H
#define _LEDGER_BOOK_H

#include <string.h>
#include <stdlib.h>

typedef struct {
	char* name;
} Entity;

typedef struct {
	Entity* from;
	Entity* to;
	long long amt;
} Entry;


void ledger_parse_data(char *text, size_t text_len);

Entry** ledger_read_file(const char* filename, time_t date_start, time_t date_end);

#endif
