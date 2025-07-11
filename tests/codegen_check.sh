#!/bin/bash

COMPILER=../build/compiler

tmp=../build/AArch64_CodegenTest

TEST_DIRS=(
    "functional" "arm"
)

CFLAGS='-O1'

LIB_DIR=../build/lib

rm -rf $tmp
mkdir -p $tmp 2>/dev/null
for dir in "${TEST_DIRS[@]}"; do
    echo "Working Directory: $dir"
    
    if [ ! -d $dir ]; then
        echo "	$dir does NOT EXIST, skipped."
        continue
    fi
    
    for sy_file in $dir/*.sy; do
        base=$(basename "$sy_file" .sy)
        
        echo -e "\033[32m\033[1m Test: $base \033[0m"
        
        in_file="$dir/$base.in"
        out_file="$dir/$base.out"
        
        if [ ! -f "$sy_file" ]; then
            echo "$base.sy does NOT EXIST, skipped."
            continue
        fi
        
	if ! ($COMPILER $sy_file -S -o $tmp/$base.s $CFLAGS && aarch64-linux-gnu-gcc $tmp/$base.s -L$LIB_DIR -l:sylib.a -static -g -o $tmp/$base ); then
            echo "ERROR: Compile failed."
            continue
        fi
        
        bin_file="$tmp/$base"
        if [ ! -f "$bin_file" ]; then
            echo "ERROR: Binary not available."
            continue
        fi
        
        if [ -f "$in_file" ]; then
            input=$(cat "$in_file")
        else
            input=""
        fi
        
	timeout=180

	if [ -n "$input" ]; then
    		output=$(timeout $timeout sh -c "qemu-aarch64 -cpu cortex-a53 \"$bin_file\" < \"$in_file\" 2>&1")
	else
    		output=$(timeout $timeout qemu-aarch64 -cpu cortex-a53 "$bin_file" 2>&1)
	fi
	ret_code=$?

	if [ $ret_code -eq 124 ]; then
    		echo "FAIL: TIMEOUT."
    		pkill -f "qemu-aarch64 $bin_file" 2>/dev/null
            	cp $sy_file $tmp/
		continue
	fi

	if [ ! -f "$out_file" ]; then
            echo "	WARNING: Reference output missing."
            echo "	Program stdout:"
            echo "	$output"
            echo "	Program returns: $ret_code"
            continue
        fi
        
        expected_output=$(head -n -1 "$out_file" 2>/dev/null)
        expected_ret_code=$(tail -n 1 "$out_file" 2>/dev/null)
        
#       echo "EXPECTED OUTPUT: $expected_output"
#       echo "OUTPUT: $output"
	         
        if [ "$ret_code" != "$expected_ret_code" ]; then
            echo "RETURN MISMATCH (EXPECTED: $expected_ret_code, PROGRAM: $ret_code)"
            cp $sy_file $tmp/
    	else
            echo "RETURN CHECK SUCCESS!"
#           rm $bin_file $tmp/$base.s
	fi
    done
done
rmdir $tmp 2>/dev/null
echo "All tests finished."
