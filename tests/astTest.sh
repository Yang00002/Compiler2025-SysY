source_file="$1"
target_file="$2"

mid_file="${target_file}.mid"

./build/ASTTest "$source_file" "$mid_file"

g++ -x c++ "$mid_file" -fsingle-precision-constant -include lib/sylib.h -L build/lib -l:sylib++.a -o "$target_file" 