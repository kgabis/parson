CFLAGS = -O0 -ggdb -Wall -Wextra -std=c89

all: test

.PHONY: test
test: tests.c parson.c
	$(CC) $(CFLAGS) -o $@ tests.c parson.c
	./$@

clean:
	rm -f test *.o