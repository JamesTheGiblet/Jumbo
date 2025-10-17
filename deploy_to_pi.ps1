# 🤖 MICRO BOT Deployment Script (PowerShell)
# Run this on Windows PC to deploy to Raspberry Pi

param(
    [string]$PiHost = "",
    [string]$PiUser = "pi"
)

Write-Host "🤖 MICRO BOT Deployment Script" -ForegroundColor Magenta
Write-Host "==============================" -ForegroundColor Blue
Write-Host ""

# Check if pi directory exists
if (!(Test-Path ".\pi")) {
    Write-Host "❌ Error: pi\ directory not found" -ForegroundColor Red
    Write-Host "Please run this script from the project root directory" -ForegroundColor Yellow
    exit 1
}

# Get Pi IP address if not provided
if ([string]::IsNullOrEmpty($PiHost)) {
    $PiHost = Read-Host "🔍 Enter Raspberry Pi IP address"
}

if ([string]::IsNullOrEmpty($PiHost)) {
    Write-Host "❌ Error: Pi IP address is required" -ForegroundColor Red
    exit 1
}

Write-Host "📡 Deploying to Pi at: $PiHost" -ForegroundColor Yellow
Write-Host ""

# Check if SSH is available (requires OpenSSH or Git Bash)
try {
    $sshTest = & ssh -o ConnectTimeout=10 -o BatchMode=yes "$PiUser@$PiHost" "exit" 2>$null
    if ($LASTEXITCODE -ne 0) {
        throw "SSH connection failed"
    }
    Write-Host "✅ SSH connection successful" -ForegroundColor Green
} catch {
    Write-Host "❌ Error: Cannot connect to Pi via SSH" -ForegroundColor Red
    Write-Host "Please ensure:" -ForegroundColor Yellow
    Write-Host "- Pi is powered on and connected to network" -ForegroundColor Cyan
    Write-Host "- SSH is enabled on Pi" -ForegroundColor Cyan
    Write-Host "- OpenSSH is installed on Windows" -ForegroundColor Cyan
    Write-Host "- Correct IP address and credentials" -ForegroundColor Cyan
    exit 1
}

# Create remote directory
Write-Host "📁 Creating remote directory..." -ForegroundColor Yellow
& ssh "$PiUser@$PiHost" "mkdir -p /home/pi/jumbo_setup"

# Copy files to Pi using SCP
Write-Host "📋 Copying files to Pi..." -ForegroundColor Yellow
try {
    # Use robocopy equivalent with SCP
    & scp -r ".\pi\*" "$PiUser@${PiHost}:/home/pi/jumbo_setup/"
    
    if ($LASTEXITCODE -eq 0) {
        Write-Host "✅ Files copied successfully" -ForegroundColor Green
    } else {
        throw "SCP copy failed"
    }
} catch {
    Write-Host "❌ Error: File copy failed" -ForegroundColor Red
    Write-Host "Trying alternative method with individual files..." -ForegroundColor Yellow
    
    # Alternative: copy individual files
    $files = @(
        "README.md",
        "setup_micro_bot.sh",
        "bridge_server.py"
    )
    
    foreach ($file in $files) {
        if (Test-Path ".\pi\$file") {
            & scp ".\pi\$file" "$PiUser@${PiHost}:/home/pi/jumbo_setup/"
        }
    }
    
    # Copy directories
    & scp -r ".\pi\config" "$PiUser@${PiHost}:/home/pi/jumbo_setup/"
    & scp -r ".\pi\scripts" "$PiUser@${PiHost}:/home/pi/jumbo_setup/"
    & scp -r ".\pi\docs" "$PiUser@${PiHost}:/home/pi/jumbo_setup/"
}

# Make scripts executable
Write-Host "🔧 Setting up permissions..." -ForegroundColor Yellow
& ssh "$PiUser@$PiHost" "chmod +x /home/pi/jumbo_setup/*.sh /home/pi/jumbo_setup/scripts/*.sh"

# Display next steps
Write-Host ""
Write-Host "✅ MICRO BOT Deployment Complete!" -ForegroundColor Green
Write-Host ""
Write-Host "📋 Next Steps:" -ForegroundColor Blue
Write-Host "1. SSH to your Pi:" -ForegroundColor Yellow
Write-Host "   ssh $PiUser@$PiHost" -ForegroundColor Cyan
Write-Host ""
Write-Host "2. Navigate to setup directory:" -ForegroundColor Yellow
Write-Host "   cd /home/pi/jumbo_setup" -ForegroundColor Cyan
Write-Host ""
Write-Host "3. Run the setup script:" -ForegroundColor Yellow
Write-Host "   ./setup_micro_bot.sh" -ForegroundColor Cyan
Write-Host ""
Write-Host "4. After setup, reboot the Pi:" -ForegroundColor Yellow
Write-Host "   sudo reboot" -ForegroundColor Cyan
Write-Host ""
Write-Host "5. Check status after reboot:" -ForegroundColor Yellow
Write-Host "   cd /home/pi/micro_bot" -ForegroundColor Cyan
Write-Host "   ./check_status.sh" -ForegroundColor Cyan
Write-Host ""
Write-Host "🌐 Network Information:" -ForegroundColor Blue
Write-Host "- WiFi Network: JumboBot_Network" -ForegroundColor Cyan
Write-Host "- WiFi Password: JumboBot2025" -ForegroundColor Cyan
Write-Host "- Bridge IP: 192.168.4.1" -ForegroundColor Cyan
Write-Host "- Bot WebSocket: ws://192.168.4.1:8765" -ForegroundColor Cyan
Write-Host "- PC WebSocket: ws://192.168.4.1:8766" -ForegroundColor Cyan
Write-Host ""
Write-Host "🤖 Ready to set up MICRO BOT bridge!" -ForegroundColor Magenta

# Optional: Open SSH session
$openSSH = Read-Host "Would you like to open SSH session to Pi now? (y/n)"
if ($openSSH -eq "y" -or $openSSH -eq "Y") {
    Write-Host "🔐 Opening SSH session to Pi..." -ForegroundColor Yellow
    & ssh "$PiUser@$PiHost"
}