CC = gcc
CFLAGS = -O0 -g -Wall -Wextra -std=c89 -pedantic-errors

all: test

.PHONY: test
test: tests.c parson.c
	$(CC) $(CFLAGS) -o $@ tests.c parson.c
	./$@

clean:
	rm -f test *.o