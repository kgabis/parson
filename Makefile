COMMON_FLAGS = -O0 -g -Wall -Wextra -pedantic-errors

CFLAGS = $(COMMON_FLAGS) -std=c89
CXXFLAGS = $(COMMON_FLAGS) -std=c++98

all: test testcpp

tests: runtest runtestcpp

runtest: test
	./$<

runtestcpp: testcpp
	./$<

%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $^

test: tests.o parson.o
	$(CC) $(LDFLAGS) -o $@ $^

%.opp: %.c
	$(CXX) $(CXXFLAGS) -o $@ -c $^

testcpp: tests.opp parson.opp
	$(CXX) $(LDFLAGS) -o $@ $^

clean:
	rm -f test testcpp *.o *.opp

.PHONY: clean all tests runtest runtestcpp
