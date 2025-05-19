source_file="$1"
target_file="$2"

g++ -x c++ "$source_file" -fsingle-precision-constant -include lib/sylib.h -L build/lib -l:sylib++.a -o "$target_file" 