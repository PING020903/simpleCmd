cmake_minimum_required(VERSION 3.10)

project(simpleCmd 
    VERSION 1.0 
    DESCRIPTION "one_test" 
    LANGUAGES C)


# 工具链: 使用 toolchain-llvm-mingw.cmake 指定 LLVM MinGW 编译器
#   命令行: cmake -B build -G "Ninja" -DCMAKE_TOOLCHAIN_FILE=toolchain-llvm-mingw.cmake
#   或直接运行 build.bat

# 设置 C99 标准
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# 导出 compile_commands.json，供 clangd/IDE 获取编译参数（include 路径等）
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# 设置头文件搜索路径
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/include)
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/third_party/c-linked-list)

# cmdTree v2.0 源文件（旧 CommandParseNode.c / CommandParseTable.c 已弃用）
set(SOURCES
    ${PROJECT_SOURCE_DIR}/sources/main.c
    ${PROJECT_SOURCE_DIR}/sources/cmdUserStringParser.c
    ${PROJECT_SOURCE_DIR}/sources/CommandParseTree.c
)

# 设置 "./build/bin" 目录作为可执行程序输出目录
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/build/bin)

add_executable(outputFile ${SOURCES})

# tokenizer 独立测试
add_executable(test_tokenizer
    ${PROJECT_SOURCE_DIR}/sources/cmdUserStringParser.c
    ${PROJECT_SOURCE_DIR}/test_tokenizer.c
)

# cmdTree 独立测试
add_executable(test_cmdTree
    ${PROJECT_SOURCE_DIR}/sources/cmdUserStringParser.c
    ${PROJECT_SOURCE_DIR}/sources/CommandParseTree.c
    ${PROJECT_SOURCE_DIR}/test_cmdTree.c
)
