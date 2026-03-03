@echo off
setlocal

powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0Tools\BuildProject.ps1" %*
set "EXITCODE=%ERRORLEVEL%"

echo.
if not "%EXITCODE%"=="0" (
    echo Build failed. Review the messages above.
    pause
    exit /b %EXITCODE%
)

echo Build completed successfully.
pause
exit /b 0