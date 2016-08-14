CC = gcc
CFLAGS = -O0 -g -Wall -Wextra -std=c89 -pedantic-errors

CPPC = g++
CPPFLAGS = -O0 -g -Wall -Wextra

LIB_FILE=libparson.so
LIB_DIR=/lib
INCLUDE_DIR=/usr/include

all: test testcpp

.PHONY: test testcpp build install
test: tests.c parson.c
	$(CC) $(CFLAGS) -o $@ tests.c parson.c
	./$@

testcpp: tests.c parson.c
	$(CPPC) $(CPPFLAGS) -o $@ tests.c parson.c
	./$@

build:
	$(CC) $(CFLAGS) parson.c -c -fpic
	$(CC) -shared -o $(LIB_FILE) *.o

install:
	rm -rf $(INCLUDE_DIR)/parson.h
	mkdir -p $(INCLUDE_DIR)
	cp *.h $(INCLUDE_DIR)
	mv $(LIB_FILE) $(LIB_DIR)

clean:
	rm -f test *.o
