GENERAL_FLAGS=-fPIC
LDFLAGS:=$(GENERAL_FLAGS) -lglfw -lGL -lm
CFLAGS:=$(GENERAL_FLAGS) -O0 -I. -g -Werror #-Wpedantic

.DEFAULT_GOAL=book

CC=gcc

%: %.c account.h
	$(CC) -o $@ $(CFLAGS) $(LDFLAGS) $<

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

bal:
	ledger -f october-2023.txt bal

lib%.so: %.o
	$(CC) -shared -Wl,-soname,$@ -o $@ $<

balance: balance.c ledger.h

hot: hot.c libbalance.so

libbook.so: book.c book.h

hotbook: hotbook.c

refresh:
	git ls-files | entr sh hot.sh

format:
	indent -psl -pal --use-tabs -ts4 -br -brs -ce -cli0 book.c

awk_query = $$(awk '/\/\* $1/{flag=1; next}/$1 \*\//{flag=0}flag' $2.c)
test_cmd = if [ "$$($< $(call awk_query,TEST_INPUT,$<))" = "$(call awk_query,TEST_OUTPUT,$<)" ]; \
			   then echo Passed; \
			   else echo Failed; \
		   fi

#include tests/tests.mk

clean:
	-rm *.so *.o hotbook libbook.so test/account_tree
