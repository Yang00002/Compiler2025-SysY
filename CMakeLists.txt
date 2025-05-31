# 指定 CMake 的最低版本
cmake_minimum_required(VERSION 3.10)

# 项目名称
project(SysYCompiler)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# antlr 文件集
aux_source_directory(${CMAKE_SOURCE_DIR}/antlr antlr_sources)
# 构建 antlr_lib 作为 antlr sysY 库
add_library(antlr_sysy SHARED ${antlr_sources})
# 定义一个通用的 antlr_lib 库目标
add_library(antlr_lib INTERFACE)
target_link_libraries(antlr_lib INTERFACE antlr_sysy antlr4-runtime)

# 添加头文件搜索路径
include_directories(${PROJECT_SOURCE_DIR}/include)

# ast 源文件集 
aux_source_directory(src/ast astSource)
# ir 源文件集 
aux_source_directory(src/ir IR)


# 添加源文件
add_executable(SysYCompiler src/main.cpp ${astSource} ${IR})
add_executable(ASTTest src/ast/main.cpp ${astSource})

target_link_libraries(SysYCompiler PRIVATE antlr_lib)
target_link_libraries(ASTTest PRIVATE antlr_lib)
