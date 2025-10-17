#!/bin/bash
# 🤖 MICRO BOT Deployment Script
# Run this on your PC to deploy to Raspberry Pi

# Color codes
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
PURPLE='\033[0;35m'
CYAN='\033[0;36m'
NC='\033[0m'

# Configuration
PI_USER="pi"
PI_HOST=""
PI_DIR="/home/pi/jumbo_setup"
LOCAL_PI_DIR="./pi"

echo -e "${PURPLE}🤖 MICRO BOT Deployment Script${NC}"
echo -e "${BLUE}==============================${NC}"
echo ""

# Check if pi directory exists
if [ ! -d "$LOCAL_PI_DIR" ]; then
    echo -e "${RED}❌ Error: pi/ directory not found${NC}"
    echo -e "${YELLOW}Please run this script from the project root directory${NC}"
    exit 1
fi

# Get Pi IP address if not provided
if [ -z "$PI_HOST" ]; then
    echo -e "${CYAN}🔍 Enter Raspberry Pi IP address:${NC}"
    read -p "Pi IP: " PI_HOST
fi

if [ -z "$PI_HOST" ]; then
    echo -e "${RED}❌ Error: Pi IP address is required${NC}"
    exit 1
fi

echo -e "${YELLOW}📡 Deploying to Pi at: $PI_HOST${NC}"
echo ""

# Test SSH connection
echo -e "${YELLOW}🔐 Testing SSH connection...${NC}"
if ! ssh -o ConnectTimeout=10 -o BatchMode=yes "$PI_USER@$PI_HOST" exit 2>/dev/null; then
    echo -e "${RED}❌ Error: Cannot connect to Pi via SSH${NC}"
    echo -e "${YELLOW}Please ensure:${NC}"
    echo -e "${CYAN}- Pi is powered on and connected to network${NC}"
    echo -e "${CYAN}- SSH is enabled on Pi${NC}"
    echo -e "${CYAN}- Correct IP address and credentials${NC}"
    exit 1
fi
echo -e "${GREEN}✅ SSH connection successful${NC}"

# Create remote directory
echo -e "${YELLOW}📁 Creating remote directory...${NC}"
ssh "$PI_USER@$PI_HOST" "mkdir -p $PI_DIR"

# Copy files to Pi
echo -e "${YELLOW}📋 Copying files to Pi...${NC}"
scp -r "$LOCAL_PI_DIR/"* "$PI_USER@$PI_HOST:$PI_DIR/"

if [ $? -eq 0 ]; then
    echo -e "${GREEN}✅ Files copied successfully${NC}"
else
    echo -e "${RED}❌ Error: File copy failed${NC}"
    exit 1
fi

# Make scripts executable
echo -e "${YELLOW}🔧 Setting up permissions...${NC}"
ssh "$PI_USER@$PI_HOST" "chmod +x $PI_DIR/*.sh $PI_DIR/scripts/*.sh"

# Display next steps
echo ""
echo -e "${GREEN}✅ MICRO BOT Deployment Complete!${NC}"
echo ""
echo -e "${BLUE}📋 Next Steps:${NC}"
echo -e "${YELLOW}1. SSH to your Pi:${NC}"
echo -e "${CYAN}   ssh $PI_USER@$PI_HOST${NC}"
echo ""
echo -e "${YELLOW}2. Navigate to setup directory:${NC}"
echo -e "${CYAN}   cd $PI_DIR${NC}"
echo ""
echo -e "${YELLOW}3. Run the setup script:${NC}"
echo -e "${CYAN}   ./setup_micro_bot.sh${NC}"
echo ""
echo -e "${YELLOW}4. After setup, reboot the Pi:${NC}"
echo -e "${CYAN}   sudo reboot${NC}"
echo ""
echo -e "${YELLOW}5. Check status after reboot:${NC}"
echo -e "${CYAN}   cd /home/pi/micro_bot${NC}"
echo -e "${CYAN}   ./check_status.sh${NC}"
echo ""
echo -e "${BLUE}🌐 Network Information:${NC}"
echo -e "${CYAN}- WiFi Network: JumboBot_Network${NC}"
echo -e "${CYAN}- WiFi Password: JumboBot2025${NC}"
echo -e "${CYAN}- Bridge IP: 192.168.4.1${NC}"
echo -e "${CYAN}- Bot WebSocket: ws://192.168.4.1:8765${NC}"
echo -e "${CYAN}- PC WebSocket: ws://192.168.4.1:8766${NC}"
echo ""
echo -e "${PURPLE}🤖 Ready to set up MICRO BOT bridge!${NC}"