cmake_minimum_required(VERSION 3.10)

project(simpleCmd 
    VERSION 1.0 
    DESCRIPTION "one_test" 
    LANGUAGES C)


# 指定C编译器路径
#set (CMAKE_C_COMPILER "F:/llvm-mingw-20220906-msvcrt-i686/bin/i686-w64-mingw32-gcc")

# 指定C++编译器路径
#set (CMAKE_CXX_COMPILER "F:/llvm-mingw-20220906-msvcrt-i686/bin/i686-w64-mingw32-g++")

# 配置指定的编译器
#set within user supplied toolchain file
#set(CMAKE_C_COMPILER clang)



#[[
# 指定C编译器路径
set(CMAKE_C_COMPILER "F:/llvm-mingw-20220906-msvcrt-i686/bin/gcc.exe" CACHE FILEPATH "C compiler")

# 指定C++编译器路径
set(CMAKE_CXX_COMPILER "F:/llvm-mingw-20220906-msvcrt-i686/bin/g++.exe" CACHE FILEPATH "C++ compiler")
#]]

# 设置 C11 标准
#set(CMAKE_C_STANDARD 11)

# 设置头文件搜索路径
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/include)

# 遍历当前目录下的所有源文件, 并获取到 SOURCES
file(GLOB_RECURSE SOURCES ${PROJECT_SOURCE_DIR}/sources/*.c)

# 设置 "./build/bin" 目录作为可执行程序输出目录
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/build/bin)

add_executable(outputFile ${SOURCES})