@echo off
echo TLTB Mini Device Checker for Windows
echo ====================================
echo.

REM Check if Python is available
python --version >nul 2>&1
if errorlevel 1 (
    echo Python is not installed or not in PATH
    echo Please install Python from python.org
    pause
    exit /b 1
)

echo Testing mDNS resolution...
python -c "import socket; print('tltb-mini.local ->', socket.gethostbyname('tltb-mini.local'))" 2>nul
if errorlevel 1 (
    echo mDNS not supported - this is normal for Windows
    echo.
    echo To enable .local domain support on Windows:
    echo 1. Install iTunes from Apple
    echo 2. OR install Bonjour Print Services
    echo 3. OR use the IP address instead
    echo.
) else (
    echo mDNS works! You can use tltb-mini.local
    echo.
)

echo Scanning network for TLTB Mini devices...
python find_device.py

echo.
echo Press any key to exit...
pause >nul