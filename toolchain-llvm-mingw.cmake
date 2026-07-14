# LLVM MinGW (i686) 工具链文件
# 用法: cmake -B build -G "Ninja" -DCMAKE_TOOLCHAIN_FILE=toolchain-llvm-mingw.cmake

set(TOOLCHAIN_ROOT "F:/llvm-mingw-20220906-msvcrt-i686")

# 目标系统
set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_SYSTEM_PROCESSOR i686)

# 编译器
set(CMAKE_C_COMPILER    "${TOOLCHAIN_ROOT}/bin/gcc.exe"   CACHE FILEPATH "C compiler")
set(CMAKE_CXX_COMPILER  "${TOOLCHAIN_ROOT}/bin/g++.exe"   CACHE FILEPATH "C++ compiler")

# 工具链其他工具（可选，CMake 通常能自动推导）
set(CMAKE_AR            "${TOOLCHAIN_ROOT}/bin/llvm-ar.exe"      CACHE FILEPATH "Archiver")
set(CMAKE_RANLIB        "${TOOLCHAIN_ROOT}/bin/llvm-ranlib.exe"  CACHE FILEPATH "Ranlib")

# C 标准
set(CMAKE_C_STANDARD 99 CACHE STRING "C standard")

# 仅在工具链文件中强制指定编译器（跳过 CMake 编译器检测）
set(CMAKE_C_COMPILER_FORCED    TRUE)
set(CMAKE_CXX_COMPILER_FORCED  TRUE)
