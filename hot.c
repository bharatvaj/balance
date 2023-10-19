#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <dlfcn.h>

typedef void* module_main_func(void* data);

int should_exit = 0;


void sig_handle() {
	should_exit = 1;
}

int main(int argc, char* argv[]) {
	void* state = NULL;
	// init gui state
	//signal(SIGINT, sig_handle);
	while(1) {
		void* module = dlopen("./libbalance.so", RTLD_NOW);
		while(module == NULL){
			fprintf(stderr, "Failed to load module. (%s)\n", dlerror());
			fprintf(stderr, "Press return to try again.\n");
			getchar();
			module = dlopen("./libbalance.so", RTLD_NOW);
		}
		module_main_func* module_main = (module_main_func*)dlsym(module, "module_main");
		state = module_main(state);
		dlclose(module);
	}

	return 0;
}
