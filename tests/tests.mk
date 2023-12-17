awk_query = $$(awk '/\/\* $1/{flag=1; next}/$1 \*\//{flag=0}flag' $2.c)
test_cmd = if [ "$$($< $(call awk_query,TEST_INPUT,$<))" = "$(call awk_query,TEST_OUTPUT,$<)" ]; \
			   then echo Passed; \
			   else echo Failed; \
		   fi

%.test: %
	@printf "Test: $<... "
	@$(call test_cmd)

test: $(addsuffix .test,$(basename $(wildcard tests/*.c)))
