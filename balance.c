#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>

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

#define MAX_MEMORY 4064
#define WINDOW_WIDTH 512
#define WINDOW_HEIGHT 512

float your_text_width_calculation(nk_handle handle, float height, const char *text, int len) {
	return 10.0f;
}

static void error_callback(int e, const char *d)
{printf("Error %d: %s\n", e, d);}

void* module_main(void* data) {
	int width,height;
	struct nk_context* ctx;
	struct nk_colorf bg;
	glfwSetErrorCallback(error_callback);
    if (!glfwInit()) {
        fprintf(stdout, "[GFLW] failed to init!\n");
        exit(1);
    }
	/***********************/
    GLFWwindow* win = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Balance", NULL, NULL);
    glfwMakeContextCurrent(win);
    glfwGetWindowSize(win, &width, &height);

    /* GUI */
    ctx = nk_glfw3_init(win, NK_GLFW3_INSTALL_CALLBACKS);
	/***********************/
	//nk_init_fixed(ctx, calloc(1, MAX_MEMORY), MAX_MEMORY, &font);
	{struct nk_font_atlas *atlas;
    nk_glfw3_font_stash_begin(&atlas);
    struct nk_font *droid = nk_font_atlas_add_from_file(atlas, "./Ubuntu-Medium.ttf", 14, 0);
    nk_glfw3_font_stash_end();
	nk_style_set_font(ctx, &droid->handle);
	}
    bg.r = 0.10f, bg.g = 0.18f, bg.b = 1.00f, bg.a = 1.0f;
    while (!glfwWindowShouldClose(win))
    {
		glfwPollEvents();
		nk_glfw3_new_frame();
		//struct nk_context* ctx = (struct nk_context*)_ctx;
		/* Input */
		if (nk_begin(ctx, "balance", nk_rect(0, 0, width, height),
					NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|NK_WINDOW_CLOSABLE)) {
			nk_layout_space_begin(ctx, NK_DYNAMIC, height, 3);
			// fixed widget pixel width
			//nk_layout_row_static(ctx, 30, 80, 1);
			nk_layout_space_push(ctx, nk_rect(0,0,width, height*0.1));
			if (nk_button_label(ctx, "Refresh")) {
				// event handling
				goto end;
				//printf("ledger_add_entry\n");
			}
			nk_label(ctx, "500", NK_TEXT_LEFT);
			static const char *list[12] = {
				"Salary Deposited - Stonks +",
				"New Stonks",
				"New Stonks",
				"New Stonks",
				"New Stonks",
				"New Stonks",
				"New Stonks",
				"New Stonks",
				"New Stonks",
				"New Stonks",
				"New Stonks",
				"New Stonks",
			};
			nk_layout_space_push(ctx, nk_rect(0,0,width, height*0.7));
			nk_layout_row_dynamic(ctx, 40, 12);
			for(int i = 0; i < 12; i++) {
				nk_layout_row_begin(ctx, NK_DYNAMIC, 0, 2);
				nk_layout_row_push(ctx, 50);
				nk_label(ctx, list[i], NK_TEXT_CENTERED);
				nk_layout_row_push(ctx, 50);
				nk_label(ctx, "500", NK_TEXT_CENTERED);
				nk_layout_row_end(ctx);
			}
			nk_layout_space_push(ctx, nk_rect(0,0,width, height*0.2));
			if (nk_button_label(ctx, " + Add Entry")) {
				// event handling
				printf("ledger_add_entry\n");
			}
			nk_layout_space_end(ctx);
		}
		nk_end(ctx);
		glfwGetWindowSize(win, &width, &height);
		glViewport(0, 0, width, height);
		glClear(GL_COLOR_BUFFER_BIT);
		glClearColor(bg.r, bg.g, bg.b, bg.a);
		/* IMPORTANT: `nk_glfw_render` modifies some global OpenGL state
		 * with blending, scissor, face culling and depth test and defaults everything
		 * back into a default state. Make sure to either save and restore or
		 * reset your own state after drawing rendering the UI. */
		nk_glfw3_render(NK_ANTI_ALIASING_ON);
		glfwSwapBuffers(win);
	}
	nk_glfw3_shutdown();
	glfwTerminate();
end:
	printf("End\n");
}
