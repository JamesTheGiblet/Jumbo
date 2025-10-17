# MICRO BOT Simple Deployment
Write-Host "🤖 MICRO BOT Deployment Package Creator" -ForegroundColor Magenta
Write-Host ""

# Check if pi directory exists
if (!(Test-Path ".\pi")) {
    Write-Host "❌ Error: pi directory not found" -ForegroundColor Red
    exit 1
}

# Create deployment zip
$zipName = "micro_bot_setup.zip"
Write-Host "📦 Creating deployment package..." -ForegroundColor Yellow

try {
    if (Test-Path $zipName) { Remove-Item $zipName }
    Compress-Archive -Path ".\pi\*" -DestinationPath $zipName -Force
    Write-Host "✅ Created: $zipName" -ForegroundColor Green
} catch {
    Write-Host "❌ Failed to create zip: $_" -ForegroundColor Red
    exit 1
}

Write-Host ""
Write-Host "📋 Deployment Instructions:" -ForegroundColor Blue
Write-Host "1. Copy $zipName to your Pi (using WinSCP/USB/etc)" -ForegroundColor Yellow
Write-Host "2. SSH to Pi: ssh pi@<pi-ip>" -ForegroundColor Yellow  
Write-Host "3. Extract: unzip $zipName -d jumbo_setup" -ForegroundColor Yellow
Write-Host "4. Setup: cd jumbo_setup && chmod +x *.sh scripts/*.sh && ./setup_micro_bot.sh" -ForegroundColor Yellow
Write-Host "5. Reboot: sudo reboot" -ForegroundColor Yellow
Write-Host ""
Write-Host "🌐 After setup, bots connect to WiFi: JumboBot_Network" -ForegroundColor Cyan
Write-Host "🔐 WiFi Password: JumboBot2025" -ForegroundColor Cyan
Write-Host ""
Write-Host "🤖 Deployment package ready!" -ForegroundColor Magenta