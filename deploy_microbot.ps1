# 🤖 MICRO BOT - Simple Deployment Script
# Modern deployment matching current working method
param(
    [string]$PiIP = "192.168.1.207",
    [string]$PiUser = "pi"
)

Write-Host "🤖 MICRO BOT Simple Deployment" -ForegroundColor Magenta
Write-Host "==============================" -ForegroundColor Blue
Write-Host ""

# Paths
$bridgeFile = "src\MICRO_BOT\real_bridge.py"
$piPath = "/home/pi/micro_bot/"

# Check if bridge file exists
if (!(Test-Path $bridgeFile)) {
    Write-Host "❌ Error: $bridgeFile not found" -ForegroundColor Red
    exit 1
}

Write-Host "📡 Deploying to Pi: $PiUser@$PiIP" -ForegroundColor Yellow
Write-Host ""

# Test SSH connection
Write-Host "🔍 Testing SSH connection..." -ForegroundColor Cyan
try {
    & ssh -o ConnectTimeout=10 "$PiUser@$PiIP" "echo 'SSH OK'" | Out-Null
    if ($LASTEXITCODE -ne 0) { throw "Connection failed" }
    Write-Host "✅ SSH connection successful" -ForegroundColor Green
} catch {
    Write-Host "❌ Cannot connect to Pi" -ForegroundColor Red
    Write-Host "Please check:" -ForegroundColor Yellow
    Write-Host "- Pi is online at $PiIP" -ForegroundColor Cyan
    Write-Host "- SSH is enabled on Pi" -ForegroundColor Cyan
    exit 1
}

# Create directory if needed
Write-Host "📁 Ensuring remote directory exists..." -ForegroundColor Cyan
& ssh "$PiUser@$PiIP" "mkdir -p $piPath"

# Copy bridge file
Write-Host "📤 Uploading bridge server..." -ForegroundColor Cyan
try {
    & scp "$bridgeFile" "$PiUser@${PiIP}:$piPath"
    if ($LASTEXITCODE -ne 0) { throw "SCP failed" }
    Write-Host "✅ Bridge server uploaded" -ForegroundColor Green
} catch {
    Write-Host "❌ Failed to upload bridge server" -ForegroundColor Red
    exit 1
}

# Check if bridge is running and stop it
Write-Host "🛑 Stopping any existing bridge..." -ForegroundColor Cyan
& ssh "$PiUser@$PiIP" "pkill -f real_bridge.py 2>/dev/null || true"

# Start bridge in background
Write-Host "🚀 Starting MICRO BOT bridge..." -ForegroundColor Cyan
& ssh "$PiUser@$PiIP" "cd $piPath && nohup python3 real_bridge.py > bridge.log 2>&1 &"

# Wait a moment and check status
Start-Sleep -Seconds 2
Write-Host "📋 Bridge status:" -ForegroundColor Cyan
& ssh "$PiUser@$PiIP" "cd $piPath && tail -5 bridge.log"

Write-Host ""
Write-Host "✅ Deployment Complete!" -ForegroundColor Green
Write-Host ""
Write-Host "🌐 Network Configuration:" -ForegroundColor Blue
Write-Host "- Pi IP: $PiIP" -ForegroundColor Cyan
Write-Host "- Bot Port: 8765 (ESP32 connections)" -ForegroundColor Cyan  
Write-Host "- PC Port: 8766 (PC MCU connections)" -ForegroundColor Cyan
Write-Host ""
Write-Host "🎛️ Next Steps:" -ForegroundColor Blue
Write-Host "1. Start PC MCU: cd src\PC_MCU && start_pc_mcu.bat" -ForegroundColor Yellow
Write-Host "2. Power on your WHEELIE/SPEEDIE bots" -ForegroundColor Yellow
Write-Host "3. Monitor connections in PC MCU GUI" -ForegroundColor Yellow
Write-Host ""
Write-Host "🔍 Monitor bridge: ssh $PiUser@$PiIP 'tail -f $piPath/bridge.log'" -ForegroundColor Gray