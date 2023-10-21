GENERAL_FLAGS=-fPIC
LDFLAGS=$(GENERAL_FLAGS) -lglfw -lGL -lm
CFLAGS=$(GENERAL_FLAGS) -O0 -I. -g -Werror #-Wpedantic

.DEFAULT_GOAL=hot

CC=tcc

%: %.c
	$(CC) -o $@ $(CFLAGS) $(LDFLAGS) $<

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

lib%.so: %.o
	$(CC) -shared -Wl,-soname,$@ -o $@ $<

balance: balance.c ledger.h

hot: hot.c libbalance.so

refresh:
	git ls-files | entr make libbalance.so

autorefresh:
	ls libbalance.so | entr kill -3 $$(pgrep hot)

clean:
	-rm $$(cat .gitignore)
