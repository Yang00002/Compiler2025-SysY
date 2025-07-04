source_file="$1"
target_file="$2"

mid_file_s="${target_file}.s"

./build/compiler "$source_file" -S -o "$mid_file_s"
aarch64-linux-gnu-gcc "$mid_file_s"  -L build/lib -l:sylib.a -static -g -o "$target_file"