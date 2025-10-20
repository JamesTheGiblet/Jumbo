@echo off
REM Quick MICRO BOT deployment batch file

echo.
echo 🤖 MICRO BOT Quick Deploy
echo =========================
echo.

REM Run the PowerShell deployment script
powershell.exe -ExecutionPolicy Bypass -File "deploy_microbot.ps1"

echo.
pause