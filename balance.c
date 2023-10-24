#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <time.h>
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

#include "book.h"

float your_text_width_calculation(nk_handle handle, float height, const char *text, int len) {
	return 10.0f;
}


void edit_entry(int i) {
	printf("ledger_edit_entry(%d)\n", i);
}

void* module_main(struct nk_context* ctx, int width, int height) {
	// main render loop
	// handle file loading
	// handle rendering clicks etc
	Entry** new_list = ledger_read_file("october-2023.txt", time(NULL), time(NULL));
	if (nk_begin(ctx, "balance", nk_rect(0, 0, width, height), 0)) {
		float size[] = {0.10, 0.70, 0.30};
		nk_layout_row(ctx, NK_DYNAMIC, 0, 3, size);
		nk_layout_row_begin(ctx, NK_STATIC, 25, 1);
		nk_layout_row_push(ctx, 40);
		nk_label(ctx, "500", NK_TEXT_LEFT);
		nk_layout_row_end(ctx);

		int len = 12;
		for(int i = 0; i < len; i++) {
			nk_layout_row_begin(ctx, NK_DYNAMIC, 0, 2);
			nk_layout_row_push(ctx, 0.7f);
			char* name = (char*)malloc(sizeof(char) * 50);
			sprintf(name, "%s: %d", new_list[i]->to->name, 500);
			if(nk_button_label(ctx, name)) {
				printf("%s\n", name);
				edit_entry(i);
			}
			nk_layout_row_push(ctx, 0.3f);
			nk_label(ctx, "500", NK_TEXT_LEFT);
			nk_layout_row_end(ctx);
		}

		nk_layout_row_begin(ctx, NK_STATIC, 25, 1);
		nk_layout_row_push(ctx, 100);
		if (nk_button_label(ctx, " + Add Entry")) {
			// event handling
			printf("ledger_add_entry\n");
		}
		nk_layout_row_end(ctx);
	}
	nk_end(ctx);
	return NULL;
}
