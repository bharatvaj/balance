GENERAL_FLAGS=-fPIC
LDFLAGS=$(GENERAL_FLAGS) -lglfw -lGL -lm -lGLEW
CFLAGS=$(GENERAL_FLAGS) -I. -g -Werror #-Wpedantic

.DEFAULT_GOAL=hot

%: %.c
	$(CC) -o $@ $(CFLAGS) $(LDFLAGS) $<

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

lib%.so: %.o
	$(CC) -shared -Wl,-soname,$@ -o $@ $<

balance: balance.c

libhot-reload.so: hot-reload.c

hot: hot.c libhot-reload.so libbalance.so

clean:
	-rm $(cat .gitignore)
