@echo off
echo ================================================
echo   CODE PLAGIARISM DETECTOR + CODE FORMATTER
echo   Build and Run Script for Windows
echo ================================================
echo.

REM Check if g++ is available
where g++ >nul 2>&1
if %errorlevel% neq 0 (
    echo [ERROR] g++ not found!
    echo         Please install MinGW-w64 and add it to your PATH.
    echo         Download: https://www.mingw-w64.org/
    pause
    exit /b 1
)

echo [1/2] Compiling main.cpp...
g++ main.cpp -o detector -std=c++11

if %errorlevel% neq 0 (
    echo.
    echo [ERROR] Compilation failed. See errors above.
    pause
    exit /b 1
)

echo [2/2] Compiled successfully!
echo.
echo ================================================
echo   Launching detector...
echo ================================================
echo.

detector.exe

pause
