GENERAL_FLAGS=-fPIC
GUI_LDFLAGS=-lglfw -lGL
LDFLAGS:=$(GENERAL_FLAGS)  -lm
CFLAGS:=$(GENERAL_FLAGS) -O0 -I. -g -Werror #-Wpedantic
export LD_LIBRARY_PATH=.

.DEFAULT_GOAL=payredu

CC=gcc

payredu: payredu.c libbook.a
	$(CC) -o $@ $(CFLAGS) $(LDFLAGS) $> -L. -lbook

.o: .c
	$(CC) $(CFLAGS) -c $> -o $@

bal:
	#ledger -f october-2023.txt bal

balance: balance.c ledger.h

hot: hot.c libbalance.so

libbook.a: book.o account.o
	ar cr $@ $>

libbook.so: book.o  account.o
	$(CC) -shared -Wl,-soname,$@ -o $@ $>

refresh:
	git ls-files | entr sh hot.sh

format:
	indent -psl -pal --use-tabs -ts4 -br -brs -ce -cli0 book.c

include tests/tests.mk

clean:
	-rm *.so *.o hotbook libbook.so $(TESTS)
