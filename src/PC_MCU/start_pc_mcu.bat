@echo off
echo ========================================
echo   PROJECT JUMBO PC MCU CONTROL CENTER
echo ========================================
echo.
echo Starting PC MCU Control Center...
echo.

REM Check if Python is available
python --version >nul 2>&1
if %errorlevel% neq 0 (
    echo ERROR: Python is not installed or not in PATH
    echo Please install Python 3.9+ from python.org
    pause
    exit /b 1
)

REM Check if required packages are installed
echo Checking dependencies...
python -c "import websockets, matplotlib, tkinter" >nul 2>&1
if %errorlevel% neq 0 (
    echo Installing required packages...
    pip install -r requirements.txt
)

REM Start PC MCU
echo.
echo Launching PC MCU Control Center...
python pc_mcu_main.py

echo.
echo PC MCU Control Center has closed.
pause