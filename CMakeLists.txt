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


# util 源文件集 
aux_source_directory(src/util utilSource)
# ast 源文件集 
aux_source_directory(src/ast astSource)

# ir 源文件集 
aux_source_directory(src/ir irSource)

# codegen
aux_source_directory(src/codegen cgenSource)

# 添加源文件
add_executable(compiler src/compiler.cpp ${utilSource} ${astSource} ${irSource} ${cgenSource})

target_link_libraries(compiler PRIVATE antlr_lib)
