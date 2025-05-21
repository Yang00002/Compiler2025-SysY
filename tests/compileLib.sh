#!/bin/bash

output_dir="./build/lib"

if [ ! -d "$output_dir" ]; then
    mkdir -p "$output_dir"
fi

gcc -c ./lib/sylib.c -fsingle-precision-constant -o ./build/sylib.o
g++ -c ./lib/sylib.c -fsingle-precision-constant -o ./build/sylib++.o
ar rcs ./build/lib/sylib.a ./build/sylib.o
ar rcs ./build/lib/sylib++.a ./build/sylib++.o
rm ./build/sylib.o
rm ./build/sylib++.o