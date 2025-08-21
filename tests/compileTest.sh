source_file="$1"
target_file="$2"

aarch64-linux-gnu-g++ -x c++ "$source_file" -fsingle-precision-constant -O2 -include lib/sylib.h -L build/lib -l:sylib++.a -o "$target_file" 