source_file="$1"
target_file="$2"

mid_file="${target_file}.mid"

./build/astTest "$source_file" "$mid_file"

aarch64-linux-gnu-g++ -x c++ "$mid_file" -include lib/sylib.h -L build/lib -l:sylib++.a -o "$target_file" 