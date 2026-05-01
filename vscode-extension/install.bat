@echo off
echo ================================================
echo   Installing Code Plagiarism Detector Extension
echo   into VS Code...
echo ================================================
echo.

REM Find VS Code extensions folder
set EXT_DIR=%USERPROFILE%\.vscode\extensions\plagiarism-detector-1.0.0

echo Installing to: %EXT_DIR%
echo.

REM Copy extension files
mkdir "%EXT_DIR%" 2>nul
copy /Y "package.json"  "%EXT_DIR%\package.json"  >nul
copy /Y "extension.js"  "%EXT_DIR%\extension.js"  >nul

if %errorlevel% neq 0 (
    echo [ERROR] Could not copy files. Try running as Administrator.
    pause
    exit /b 1
)

echo [OK] Extension files copied successfully!
echo.
echo ================================================
echo   NEXT STEPS:
echo   1. Close and reopen VS Code
echo   2. Open a .cpp file
echo   3. Press Ctrl+Shift+P
echo   4. Type: Plagiarism Detector
echo   5. Choose Compare Files OR Beautify Current File
echo.
echo   TIP: Right-click in any .cpp file to see
echo        the two commands in the context menu!
echo ================================================
echo.
pause
