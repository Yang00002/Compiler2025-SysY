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

include_directories(${PROJECT_SOURCE_DIR}/include/passes)
include_directories(${PROJECT_SOURCE_DIR}/include/ast)
include_directories(${PROJECT_SOURCE_DIR}/include/codegen)
include_directories(${PROJECT_SOURCE_DIR}/include/ir)
include_directories(${PROJECT_SOURCE_DIR}/include/util)
include_directories(${PROJECT_SOURCE_DIR}/antlr)

# util 源文件集 
aux_source_directory(src/util utilSource)
# ast 源文件集 
aux_source_directory(src/ast astSource)
list(REMOVE_ITEM astSource "src/ast/main.cpp")
# ir 源文件集 
aux_source_directory(src/ir irSource)
list(REMOVE_ITEM irSource "src/ir/main.cpp")
# codegen 源文件集 
aux_source_directory(src/codegen codegenSource)
# passes 源文件集
aux_source_directory(src/passes passesSource)
list(REMOVE_ITEM passesSource "src/passes/main.cpp")


# 添加源文件
add_executable(compiler src/compiler.cpp ${utilSource} ${astSource} ${irSource} ${codegenSource})
add_executable(astTest src/ast/main.cpp ${utilSource} ${astSource}  ${irSource})
add_executable(irTest src/ir/main.cpp ${utilSource} ${astSource} ${irSource})
add_executable(passesTest src/passes/main.cpp ${utilSource} ${astSource} ${irSource} ${passesSource})

target_link_libraries(compiler PRIVATE antlr_lib)
target_link_libraries(astTest PRIVATE antlr_lib)
target_link_libraries(irTest PRIVATE antlr_lib)
target_link_libraries(passesTest PRIVATE antlr_lib)