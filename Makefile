GENERAL_FLAGS=-fPIC
LDFLAGS=$(GENERAL_FLAGS) -lglfw -lGL -lm
CFLAGS=$(GENERAL_FLAGS) -O0 -I. -g -Werror #-Wpedantic

.DEFAULT_GOAL=hotbook

CC=tcc

%: %.c
	$(CC) -o $@ $(CFLAGS) $(LDFLAGS) $<

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

lib%.so: %.o
	$(CC) -shared -Wl,-soname,$@ -o $@ $<

balance: balance.c ledger.h

hot: hot.c libbalance.so

libbook.so: book.c book.h

hotbook: hotbook.c libbook.so

refresh:
	git ls-files | entr sh hot.sh

format:
	indent -psl -pal --use-tabs -ts4 -br -brs -ce -cli0 book.c

clean:
	-rm *.so *.o hotbook libbook.so

