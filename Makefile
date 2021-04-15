CC = gcc
CFLAGS = -O0 -g -Wall -Wextra -Wconversion -std=c89 -pedantic-errors

CPPC = g++
CPPFLAGS = -O0 -g -Wall -Wextra -Wconversion

all: test testcpp parson

.PHONY: test testcpp parson

parson: parson.o
parson.o: parson.c
	$(CC) $(CFLAGS) -c $< -o $@

test: tests.c parson.c
	$(CC) $(CFLAGS) -o $@ tests.c parson.c
	./$@

testcpp: tests.c parson.c
	$(CPPC) $(CPPFLAGS) -o $@ tests.c parson.c
	./$@

clean:
	rm -f test *.o

