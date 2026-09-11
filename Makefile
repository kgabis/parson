CC = gcc
CFLAGS = -O0 -g -Wall -Wextra -std=c89 -pedantic-errors -DTESTS_MAIN

CPPC = g++
CPPFLAGS = -O0 -g -Wall -Wextra -DTESTS_MAIN 

all: test testcpp test_hash_collisions test_strtod_l

.PHONY: test testcpp test_hash_collisions test_strtod_l
test: tests.c parson.c
	$(CC) $(CFLAGS) -o $@ tests.c parson.c
	./$@

testcpp: tests.c parson.c
	$(CPPC) $(CPPFLAGS) -o $@ tests.c parson.c
	./$@

test_hash_collisions: tests.c parson.c
	$(CC) $(CFLAGS) -DPARSON_FORCE_HASH_COLLISIONS -o $@ tests.c parson.c
	./$@

# Builds with locale-independent number parsing enabled (see issue #219), so that
# test_number_parsing_is_locale_independent() in tests.c exercises the fix.
test_strtod_l: tests.c parson.c
	$(CC) $(CFLAGS) -DPARSON_USE_STRTOD_L -o $@ tests.c parson.c
	./$@

clean:
	rm -f test test_strtod_l *.o

