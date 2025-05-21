# 指定 CMake 的最低版本
cmake_minimum_required(VERSION 3.10)

# 项目名称
project(SysYCompiler)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# 添加头文件
include_directories(${PROJECT_SOURCE_DIR}/include)

# 链接antlr
set(ANTLR_SOURCES src/antlr/SysYLexer.cpp src/antlr/SysYParser.cpp)
add_library(antlr_lib SHARED ${ANTLR_SOURCES})

# 添加源文件
add_executable(SysYCompiler src/main.cpp)
target_link_libraries(SysYCompiler PRIVATE antlr_lib)

# 链接 antlr4-runtime 库
target_link_libraries(SysYCompiler PRIVATE antlr4-runtime)