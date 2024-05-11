#!/bin/bash
for file in "parson.c"; do
  $CC $CFLAGS -c ${file}
done

rm -f ./test*.o
llvm-ar rcs libfuzz.a *.o


$CC $CFLAGS $LIB_FUZZING_ENGINE $SRC/fuzzer.c -Wl,--whole-archive $SRC/parson/libfuzz.a -Wl,--allow-multiple-definition -I$SRC/parson/  -o $OUT/fuzzer