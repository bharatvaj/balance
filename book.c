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



void* module_main(struct nk_context* ctx, int width, int height) {
    printf("this is us\n");
}
