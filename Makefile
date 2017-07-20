CC = gcc
CFLAGS = -O0 -g -Wall -Wextra -std=c89 -pedantic-errors

CPPC = g++
CPPFLAGS = -O0 -g -Wall -Wextra

#CFLAGS += -DPARSON_RETURN_ERROR_VALUES
#CPPFLAGS += -DPARSON_RETURN_ERROR_VALUES

all: test testcpp

.PHONY: test testcpp
test: tests.c parson.c
	$(CC) $(CFLAGS) -o $@ tests.c parson.c
	./$@

testcpp: tests.c parson.c
	$(CPPC) $(CPPFLAGS) -o $@ tests.c parson.c
	./$@

clean:
	rm -f test *.o

