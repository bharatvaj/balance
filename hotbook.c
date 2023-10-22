
#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <signal.h>

#include "common.h"

#define MAX_MEMORY 4064
#define WINDOW_WIDTH 512
#define WINDOW_HEIGHT 512

int should_exit = 1;

void sig_handle() {
	printf("Reloaded\n");
	system("date");
	should_exit = 1;
}

typedef void* (*module_main_func)(void*, int, int);

int main(int argc, char* argv[]) {
	signal(SIGQUIT, sig_handle);
	while(1) {
		void* module = dlopen("./libbalance.so", RTLD_NOW);
		while(module == NULL){
			fprintf(stderr, "Failed to load module. (%s)\n", dlerror());
			fprintf(stderr, "Press return to try again.\n");
			getchar();
			module = dlopen("./libbalance.so", RTLD_NOW);
		}
		module_main_func module_main = dlsym(module, "module_main");
		dlclose(module);
		should_exit = 0;
	}

	return 0;
}
