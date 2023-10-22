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

Entity me = {"Account:Income"};

Entry** ledger_read_file(const char* filename, time_t date_start, time_t date_end) {
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

