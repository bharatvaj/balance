#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <signal.h>

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
#include "common.h"

#define MAX_MEMORY 4064
#define WINDOW_WIDTH 512
#define WINDOW_HEIGHT 512

void* state = NULL;
int should_exit = 0;
// init gui state
int width,height;
struct nk_context* ctx;
GLFWwindow* win;

void sig_handle() {
	printf("Reloaded\n");
	system("date");
	should_exit = 1;
}

typedef void* (*module_main_func)(void*, int, int);

static void error_callback(int e, const char *d)
{printf("Error %d: %s\n", e, d);}

int main(int argc, char* argv[]) {
	signal(SIGQUIT, sig_handle);
	glfwSetErrorCallback(error_callback);
	if (!glfwInit()) {
		fprintf(stdout, "[GFLW] failed to init!\n");
		exit(1);
	}
	/***********************/
	win = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Balance", NULL, NULL);
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
	while(1) {
		void* module = dlopen("./libbalance.so", RTLD_NOW);
		while(module == NULL){
			fprintf(stderr, "Failed to load module. (%s)\n", dlerror());
			fprintf(stderr, "Press return to try again.\n");
			getchar();
			module = dlopen("./libbalance.so", RTLD_NOW);
		}
		module_main_func module_main = dlsym(module, "module_main");
		while (!glfwWindowShouldClose(win) && !should_exit) {
			glfwPollEvents();
			nk_glfw3_new_frame();
			state = module_main((void*)ctx, width, height);
			glfwGetWindowSize(win, &width, &height);
			glViewport(0, 0, width, height);
			glClearColor(0.10f, 0.18f, 1.0f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT);
			/* IMPORTANT: `nk_glfw_render` modifies some global OpenGL state
			 * with blending, scissor, face culling and depth test and defaults everything
			 * back into a default state. Make sure to either save and restore or
			 * reset your own state after drawing rendering the UI. */
			nk_glfw3_render(NK_ANTI_ALIASING_ON);
			glfwSwapBuffers(win);
		}
		dlclose(module);
		should_exit = 0;
	}

	return 0;
}
