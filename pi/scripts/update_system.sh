#!/bin/bash
# MICRO BOT System Update and Maintenance Script

# Color codes
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
PURPLE='\033[0;35m'
CYAN='\033[0;36m'
NC='\033[0m'

MICRO_BOT_DIR="/home/pi/micro_bot"
BACKUP_DIR="$MICRO_BOT_DIR/backups/$(date +%Y%m%d_%H%M%S)"

echo -e "${PURPLE}🤖 MICRO BOT System Update${NC}"
echo -e "${BLUE}===========================${NC}"
echo -e "${CYAN}$(date)${NC}"
echo ""

# Check if running in correct directory
if [ ! -f "$MICRO_BOT_DIR/bridge_server.py" ]; then
    echo -e "${RED}❌ Error: MICRO BOT not found in $MICRO_BOT_DIR${NC}"
    exit 1
fi

# Create backup directory
echo -e "${YELLOW}📋 Creating configuration backup...${NC}"
mkdir -p "$BACKUP_DIR"

# Backup critical configurations
cp /etc/hostapd/hostapd.conf "$BACKUP_DIR/" 2>/dev/null
cp /etc/dnsmasq.conf "$BACKUP_DIR/" 2>/dev/null
cp /etc/systemd/system/micro-bot.service "$BACKUP_DIR/" 2>/dev/null
cp /etc/dhcpcd.conf "$BACKUP_DIR/" 2>/dev/null
cp "$MICRO_BOT_DIR/bridge_server.py" "$BACKUP_DIR/" 2>/dev/null

echo -e "${GREEN}✅ Backup created at: $BACKUP_DIR${NC}"

# Stop bridge service for updates
echo -e "${YELLOW}⏸️ Stopping bridge service...${NC}"
sudo systemctl stop micro-bot.service

# Update system packages
echo -e "${YELLOW}📦 Updating system packages...${NC}"
sudo apt update && sudo apt upgrade -y

# Update additional packages that might be useful
echo -e "${YELLOW}🔧 Installing additional tools...${NC}"
sudo apt install -y htop iftop iotop tree jq curl wget git vim nano

# Update Python and pip
echo -e "${YELLOW}🐍 Updating Python environment...${NC}"
cd "$MICRO_BOT_DIR"

# Activate virtual environment
if [ -f "venv/bin/activate" ]; then
    source venv/bin/activate
    echo -e "${GREEN}✅ Virtual environment activated${NC}"
    
    # Update pip and packages
    pip install --upgrade pip
    pip install --upgrade websockets asyncio aiofiles python-dateutil psutil
    
    # Install additional useful packages
    pip install requests aiohttp netifaces
    
    echo -e "${GREEN}✅ Python packages updated${NC}"
else
    echo -e "${RED}❌ Virtual environment not found${NC}"
    echo -e "${YELLOW}🔧 Recreating virtual environment...${NC}"
    python3 -m venv venv
    source venv/bin/activate
    pip install --upgrade pip
    pip install websockets asyncio aiofiles python-dateutil psutil requests aiohttp netifaces
fi

# Clean up old logs (keep last 30 days)
echo -e "${YELLOW}🧹 Cleaning up old logs...${NC}"
find "$MICRO_BOT_DIR/logs" -name "*.log" -mtime +30 -delete 2>/dev/null || true
find "$MICRO_BOT_DIR/backups" -type d -mtime +60 -exec rm -rf {} \; 2>/dev/null || true

# Update system configuration if needed
echo -e "${YELLOW}⚙️ Checking system configuration...${NC}"

# Ensure IP forwarding is enabled
if ! grep -q "net.ipv4.ip_forward=1" /etc/sysctl.conf; then
    echo "net.ipv4.ip_forward=1" | sudo tee -a /etc/sysctl.conf
    echo -e "${GREEN}✅ IP forwarding enabled${NC}"
fi

# Check and restore iptables rules if needed
if [ ! -f "/etc/iptables.ipv4.nat" ]; then
    echo -e "${YELLOW}🛡️ Recreating firewall rules...${NC}"
    # Basic NAT and forwarding rules
    sudo iptables -t nat -A POSTROUTING -o eth0 -j MASQUERADE
    sudo iptables -A FORWARD -i eth0 -o wlan0 -m state --state RELATED,ESTABLISHED -j ACCEPT
    sudo iptables -A FORWARD -i wlan0 -o eth0 -j ACCEPT
    sudo sh -c "iptables-save > /etc/iptables.ipv4.nat"
    echo -e "${GREEN}✅ Firewall rules recreated${NC}"
fi

# Update systemd service if needed
echo -e "${YELLOW}🔄 Updating systemd service...${NC}"
sudo systemctl daemon-reload
sudo systemctl enable micro-bot.service hostapd.service dnsmasq.service

# Restart services
echo -e "${YELLOW}🚀 Restarting services...${NC}"
sudo systemctl restart hostapd
sudo systemctl restart dnsmasq
sudo systemctl start micro-bot.service

# Wait for services to start
sleep 5

# Check service status
echo -e "${YELLOW}📊 Checking service status...${NC}"
if sudo systemctl is-active micro-bot.service >/dev/null 2>&1; then
    echo -e "  Bridge Service: ${GREEN}✅ Running${NC}"
else
    echo -e "  Bridge Service: ${RED}❌ Failed to start${NC}"
fi

if sudo systemctl is-active hostapd.service >/dev/null 2>&1; then
    echo -e "  WiFi AP: ${GREEN}✅ Running${NC}"
else
    echo -e "  WiFi AP: ${RED}❌ Failed to start${NC}"
fi

if sudo systemctl is-active dnsmasq.service >/dev/null 2>&1; then
    echo -e "  DHCP Server: ${GREEN}✅ Running${NC}"
else
    echo -e "  DHCP Server: ${RED}❌ Failed to start${NC}"
fi

# Update system information
echo -e "${YELLOW}📝 Updating system information...${NC}"
tee "$MICRO_BOT_DIR/UPDATE_LOG.txt" > /dev/null <<EOF
MICRO BOT System Update Log
===========================

Update Date: $(date)
Backup Location: $BACKUP_DIR

System Information:
- Hostname: $(hostname)
- Kernel: $(uname -r)
- OS: $(lsb_release -d 2>/dev/null | cut -f2 || echo "Unknown")
- Python Version: $(python --version 2>&1)

Service Status:
- Bridge Service: $(sudo systemctl is-active micro-bot.service)
- WiFi AP: $(sudo systemctl is-active hostapd.service)
- DHCP Server: $(sudo systemctl is-active dnsmasq.service)

Network Configuration:
- Bridge IP: $(ip addr show wlan0 | grep "inet " | awk '{print $2}' | head -1)
- WiFi Mode: $(iwconfig wlan0 2>/dev/null | grep "Mode:" | awk -F'Mode:' '{print $2}' | awk '{print $1}')

Recent Updates:
- System packages updated
- Python packages updated
- Configuration backed up
- Services restarted
EOF

# Display summary
echo ""
echo -e "${GREEN}✅ MICRO BOT Update Complete!${NC}"
echo ""
echo -e "${BLUE}📋 Update Summary:${NC}"
echo -e "${CYAN}- System packages: Updated${NC}"
echo -e "${CYAN}- Python packages: Updated${NC}"
echo -e "${CYAN}- Configuration: Backed up to $BACKUP_DIR${NC}"
echo -e "${CYAN}- Services: Restarted${NC}"
echo -e "${CYAN}- Logs: Cleaned up (kept last 30 days)${NC}"
echo ""
echo -e "${BLUE}🛠️ Next Steps:${NC}"
echo -e "${YELLOW}1. Check system status: ./check_status.sh${NC}"
echo -e "${YELLOW}2. Monitor logs: sudo journalctl -u micro-bot.service -f${NC}"
echo -e "${YELLOW}3. Test bot connections${NC}"
echo ""
echo -e "${PURPLE}🤖 MICRO BOT is ready for continued operation!${NC}"

# Optional reboot prompt
echo ""
read -p "Would you like to reboot the system now for all changes to take effect? (y/n): " -n 1 -r
echo
if [[ $REPLY =~ ^[Yy]$ ]]; then
    echo -e "${YELLOW}🔄 Rebooting system in 10 seconds...${NC}"
    echo -e "${RED}Press Ctrl+C to cancel${NC}"
    sleep 10
    sudo reboot
else
    echo -e "${GREEN}✅ Update completed without reboot${NC}"
    echo -e "${YELLOW}Remember to reboot later for all changes to take effect${NC}"
fi