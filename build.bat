@echo off
setlocal

REM ==============================
REM 安全终止旧进程（静默处理）
REM ==============================
taskkill /IM "outputFile.exe" /F >nul 2>&1

REM ==============================
REM 智能配置：校验生成器一致性，避免缓存冲突
REM ==============================
set "BUILD_DIR=build"
set "CACHE_FILE=%BUILD_DIR%\CMakeCache.txt"
set "TARGET_EXE=%BUILD_DIR%\bin\outputFile.exe"

if exist "%CACHE_FILE%" (
    findstr /C:"CMAKE_GENERATOR:STATIC=Ninja" "%CACHE_FILE%" >nul 2>&1
    if errorlevel 1 (
        echo [WARN] Existing cache uses a different generator ^(not Ninja^).
        echo        To avoid conflicts, the build directory will be recreated.
        echo [INFO] Cleaning '%BUILD_DIR%'...
        rmdir /s /q "%BUILD_DIR%" 2>nul
    )
)

echo [INFO] Configuring project ^(Ninja, Debug^)...
cmake -B "%BUILD_DIR%" -G "Ninja" -DCMAKE_BUILD_TYPE=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
if %ERRORLEVEL% neq 0 (
    echo [ERROR] CMake configuration failed.
    echo        - Check CMakeLists.txt syntax
    echo        - Ensure Ninja is in PATH ^(run: where ninja^)
    echo        - Manual fix: delete '%BUILD_DIR%' and retry
    exit /b 1
)

REM ==============================
REM 并行构建
REM ==============================
echo [INFO] Building project ^(parallel^)...
cmake --build "%BUILD_DIR%" --parallel
if %ERRORLEVEL% neq 0 (
    echo [ERROR] Build failed. Review compiler errors above.
    exit /b 1
)

REM ==============================
REM 执行验证与启动
REM ==============================
if not exist "%TARGET_EXE%" (
    echo [ERROR] Executable not found: %TARGET_EXE%
    echo        Verify in CMakeLists.txt:
    echo        - Target name is 'outputFile'
    echo        - Runtime output directory set to ^${CMAKE_BINARY_DIR}/bin
    exit /b 1
)

echo [INFO] Launching application...
"%TARGET_EXE%"
exit /b %ERRORLEVEL%