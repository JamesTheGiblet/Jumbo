@echo off
title PC MCU - Project Jumbo Control Center
echo 🖥️ Starting PC MCU...
echo.

REM Check if virtual environment exists
if not exist "pc_mcu_env" (
    echo ❌ PC MCU not set up. Please run setup_pc_mcu.bat first.
    pause
    exit /b 1
)

REM Activate virtual environment and run
cd /d "%~dp0"
call pc_mcu_env\Scripts\activate.bat
python pc_mcu_main.py

pause