UNAME := $(shell uname -s)

ifeq ($(UNAME), Darwin)
	CC   := $(shell which gcc-12)
	CPPC := $(shell which g++-12)
else
	CC   := $(shell which gcc)
	CPPC := $(shell which g++)
endif

CFLAGS   := -O0 -g -Wall -Wextra -Wno-deprecated-declarations -DTESTS_MAIN -std=c89 -pedantic-errors
CPPFLAGS := -O0 -g -Wall -Wextra -Wno-deprecated-declarations -DTESTS_MAIN

all: test testcpp test_hash_collisions

.PHONY: test testcpp test_hash_collisions
test: tests.c parson.c
	$(CC) $(CFLAGS) -o $@ tests.c parson.c
	./$@

testcpp: tests.c parson.c
	$(CPPC) $(CPPFLAGS) -o $@ tests.c parson.c
	./$@

test_hash_collisions: tests.c parson.c
	$(CC) $(CFLAGS) -DPARSON_FORCE_HASH_COLLISIONS -o $@ tests.c parson.c
	./$@

clean:
	rm -f test *.o
