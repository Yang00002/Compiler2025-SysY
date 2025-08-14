source_file="$1"
target_file="$2"

aarch64-linux-gnu-g++ -x c++ "$source_file" -fsingle-precision-constant -include lib/sylib.h1 -L build/lib -l:sylib++.a -o "$target_file" 