# 🤖 MICRO BOT Simple Deployment Script (PowerShell)
# Alternative deployment method without SSH dependencies

Write-Host "🤖 MICRO BOT Simple Deployment" -ForegroundColor Magenta
Write-Host "==============================" -ForegroundColor Blue
Write-Host ""

# Check if pi directory exists
if (!(Test-Path ".\pi")) {
    Write-Host "❌ Error: pi\ directory not found" -ForegroundColor Red
    Write-Host "Please run this script from the project root directory" -ForegroundColor Yellow
    exit 1
}

Write-Host "📦 Creating deployment package..." -ForegroundColor Yellow

# Create deployment archive
$deploymentName = "micro_bot_deployment_$(Get-Date -Format 'yyyyMMdd_HHmmss').zip"

try {
    # Compress pi directory to zip file
    Compress-Archive -Path ".\pi\*" -DestinationPath ".\$deploymentName" -Force
    Write-Host "✅ Deployment package created: $deploymentName" -ForegroundColor Green
} catch {
    Write-Host "❌ Error creating deployment package: $_" -ForegroundColor Red
    exit 1
}

# Display deployment instructions
Write-Host ""
Write-Host "📋 Manual Deployment Instructions:" -ForegroundColor Blue
Write-Host ""

Write-Host "1. Copy the deployment package to your Pi:" -ForegroundColor Yellow
Write-Host "   - Use WinSCP, FileZilla, or USB drive" -ForegroundColor Cyan
Write-Host "   - Transfer: $deploymentName" -ForegroundColor Cyan
Write-Host "   - To Pi location: /home/pi/" -ForegroundColor Cyan
Write-Host ""

Write-Host "2. SSH to your Pi and extract:" -ForegroundColor Yellow
Write-Host "   ssh pi@<pi-ip-address>" -ForegroundColor Cyan
Write-Host "   cd ~" -ForegroundColor Cyan
Write-Host "   unzip $deploymentName -d jumbo_setup" -ForegroundColor Cyan
Write-Host "   cd jumbo_setup" -ForegroundColor Cyan
Write-Host ""

Write-Host "3. Run setup script:" -ForegroundColor Yellow
Write-Host "   chmod +x *.sh scripts/*.sh" -ForegroundColor Cyan
Write-Host "   ./setup_micro_bot.sh" -ForegroundColor Cyan
Write-Host ""

Write-Host "4. Reboot Pi after setup:" -ForegroundColor Yellow
Write-Host "   sudo reboot" -ForegroundColor Cyan
Write-Host ""

Write-Host "5. Check status after reboot:" -ForegroundColor Yellow
Write-Host "   cd /home/pi/micro_bot" -ForegroundColor Cyan
Write-Host "   ./check_status.sh" -ForegroundColor Cyan
Write-Host ""

# Alternative: Try automatic deployment if SSH is available
Write-Host "🔧 Alternative: Automatic Deployment" -ForegroundColor Blue
Write-Host ""
$tryAuto = Read-Host "Do you want to try automatic deployment via SSH? (y/n)"

if ($tryAuto -eq "y" -or $tryAuto -eq "Y") {
    $PiHost = Read-Host "Enter Pi IP address"
    $PiUser = "pi"
    
    if (![string]::IsNullOrEmpty($PiHost)) {
        Write-Host "📡 Attempting automatic deployment to $PiHost..." -ForegroundColor Yellow
        
        try {
            # Test SSH connection
            $null = & ssh -o ConnectTimeout=5 -o BatchMode=yes "$PiUser@$PiHost" "exit" 2>$null
            if ($LASTEXITCODE -eq 0) {
                Write-Host "✅ SSH connection successful" -ForegroundColor Green
                
                # Upload and extract
                & scp ".\$deploymentName" "$PiUser@${PiHost}:~/"
                & ssh "$PiUser@$PiHost" "cd ~ && unzip -o $deploymentName -d jumbo_setup && chmod +x jumbo_setup/*.sh jumbo_setup/scripts/*.sh"
                
                Write-Host "✅ Automatic deployment completed!" -ForegroundColor Green
                Write-Host "Now SSH to your Pi and run: cd jumbo_setup && ./setup_micro_bot.sh" -ForegroundColor Cyan
            } else {
                throw "SSH connection failed"
            }
        } catch {
            Write-Host "❌ Automatic deployment failed: $_" -ForegroundColor Red
            Write-Host "Please use manual deployment method above" -ForegroundColor Yellow
        }
    }
}

Write-Host ""
Write-Host "🌐 Network Information:" -ForegroundColor Blue
Write-Host "- WiFi Network: JumboBot_Network" -ForegroundColor Cyan
Write-Host "- WiFi Password: JumboBot2025" -ForegroundColor Cyan
Write-Host "- Bridge IP: 192.168.4.1" -ForegroundColor Cyan
Write-Host "- Bot WebSocket: ws://192.168.4.1:8765" -ForegroundColor Cyan
Write-Host "- PC WebSocket: ws://192.168.4.1:8766" -ForegroundColor Cyan
Write-Host ""
Write-Host "🤖 Deployment package ready!" -ForegroundColor Magenta