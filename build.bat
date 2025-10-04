@echo off
setlocal enabledelayedexpansion
taskkill /IM "outputFile.exe"

cmake -B build -G "Ninja"
if !ERRORLEVEL! neq 0 goto :err_jump

cmake --build build --config Debug
if !ERRORLEVEL! neq 0 goto :err_jump

.\build\bin\outputFile.exe
:err_jump
exit /b 0