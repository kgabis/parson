#!/bin/bash
gcc tests.c parson.c -Wall -pedantic-errors -std=c89 -o test
./test
rm -f *.o
rm test
