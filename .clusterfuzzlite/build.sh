#!/bin/bash
$CC $CFLAGS -c parson.c
llvm-ar rcs libfuzz.a *.o


$CC $CFLAGS $LIB_FUZZING_ENGINE $SRC/fuzzer.c -Wl,--whole-archive $SRC/parson/libfuzz.a -Wl,--allow-multiple-definition -I$SRC/parson/  -o $OUT/fuzzer
