source_file="$1"
target_file="$2"

mid_file_ll="${target_file}.ll"
mid_file_s="${target_file}.s"

./build/irTest "$source_file" "$mid_file_ll"

llc -march=aarch64 -mcpu=cortex-a53 -filetype=asm "$mid_file_ll" -o "${mid_file_s}"

aarch64-linux-gnu-g++ "$mid_file_s" -include lib/sylib.h -L build/lib -l:sylib.a -o "$target_file" 