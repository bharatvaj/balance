
#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <signal.h>
#include <unistd.h>

#include "common.h"

#define MAX_MEMORY 4064
#define WINDOW_WIDTH 512
#define WINDOW_HEIGHT 512
#define BUFFER_SIZE 256

int should_exit = 0;

void sig_handle() {
	printf("Reloaded\n");
	system("date");
	should_exit = 1;
}

typedef void* (*module_main_func)(const char*, size_t);

int main(int argc, char* argv[]) {
	signal(SIGQUIT, sig_handle); while(1) {
		void* module = dlopen("./libbook.so", RTLD_NOW);
		while(module == NULL){
			fprintf(stderr, "Failed to load module. (%s)\n", dlerror());
			fprintf(stderr, "Press return to try again.\n");
			getchar();
			module = dlopen("./libbook.so", RTLD_NOW);
		}
		module_main_func module_main = dlsym(module, "module_main");
		FILE* in = fopen("october-2023.txt", "r");
		char* data = (char*)malloc(2048 * sizeof(char));
		size_t data_size = 0;
		size_t c_read =  0;
		while((c_read = fread(data, 1, BUFFER_SIZE, in)) != 0) {
			data_size += c_read;
		}
		if (ferror(in)) fprintf(stderr, "Error reading file\n");
		module_main(data, data_size);
		while(should_exit == 0) {
			sleep(1);
		}
		should_exit = 0;
		dlclose(module);
		fprintf(stderr, "Continue?\n");
	}

	return 0;
}
