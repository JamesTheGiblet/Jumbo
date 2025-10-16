@echo off
echo 🖥️ PC MCU Setup Script
echo =======================
echo Setting up Project Jumbo Control Center
echo.

REM Check if Python is installed
python --version >nul 2>&1
if %errorlevel% neq 0 (
    echo ❌ Python is not installed or not in PATH
    echo Please install Python 3.8+ from https://python.org
    pause
    exit /b 1
)

echo ✅ Python found
python --version

REM Create virtual environment if it doesn't exist
if not exist "pc_mcu_env" (
    echo 📦 Creating virtual environment...
    python -m venv pc_mcu_env
)

REM Activate virtual environment
echo 🔄 Activating virtual environment...
call pc_mcu_env\Scripts\activate.bat

REM Install requirements
echo 📥 Installing requirements...
pip install -r requirements.txt

REM Create desktop shortcut
echo 🔗 Creating desktop shortcut...
set SCRIPT_DIR=%~dp0
set DESKTOP=%USERPROFILE%\Desktop
set SHORTCUT=%DESKTOP%\PC MCU.bat

echo @echo off > "%SHORTCUT%"
echo cd /d "%SCRIPT_DIR%" >> "%SHORTCUT%"
echo call pc_mcu_env\Scripts\activate.bat >> "%SHORTCUT%"
echo python pc_mcu_main.py >> "%SHORTCUT%"
echo pause >> "%SHORTCUT%"

echo.
echo ✅ Setup complete!
echo.
echo To run PC MCU:
echo   1. Double-click "PC MCU.bat" on your desktop
echo   2. Or run: python pc_mcu_main.py
echo.
echo Make sure MICRO BOT is running before connecting.
echo Default connection: 192.168.4.1:8766
echo.
pause