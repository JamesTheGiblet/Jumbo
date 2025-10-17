#!/bin/bash
# 🤖 MICRO BOT Enhanced Setup Script
# Raspberry Pi 3 Model B v1.2 Bridge Setup with Swarm Integration

set -e

# Color codes for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
PURPLE='\033[0;35m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

echo -e "${PURPLE}🤖 MICRO BOT Enhanced Setup${NC}"
echo -e "${BLUE}================================${NC}"
echo -e "${CYAN}Platform: Raspberry Pi 3 Model B v1.2${NC}"
echo -e "${CYAN}Function: Swarm Communication Bridge${NC}"
echo -e "${CYAN}Integration: WHEELIE/SPEEDIE Ecosystem${NC}"
echo ""

# Check if running as root
if [ "$EUID" -eq 0 ]; then
    echo -e "${RED}❌ Please run this script as pi user, not root${NC}"
    exit 1
fi

# Check if bridge_server.py exists
if [ ! -f "bridge_server.py" ]; then
    echo -e "${RED}❌ bridge_server.py not found in current directory${NC}"
    echo -e "${YELLOW}Please ensure you're running this from the pi/ directory${NC}"
    exit 1
fi

# Update system
echo -e "${YELLOW}📦 Updating system packages...${NC}"
sudo apt update && sudo apt upgrade -y

# Install required packages
echo -e "${YELLOW}🔧 Installing dependencies...${NC}"
sudo apt install -y python3-pip python3-venv hostapd dnsmasq iptables-persistent \
    git curl wget htop iftop iotop tree jq

# Create project directory
echo -e "${YELLOW}📁 Creating project structure...${NC}"
mkdir -p /home/pi/micro_bot/{logs,backups,scripts,config}
cd /home/pi/micro_bot

# Create Python virtual environment
echo -e "${YELLOW}🐍 Setting up Python environment...${NC}"
python3 -m venv venv
source venv/bin/activate

# Install Python packages
echo -e "${YELLOW}📚 Installing Python dependencies...${NC}"
pip install --upgrade pip
pip install websockets asyncio aiofiles python-dateutil psutil

# Copy bridge server and configurations
echo -e "${YELLOW}📋 Installing bridge server...${NC}"
cp ~/jumbo_setup/bridge_server.py .
cp ~/jumbo_setup/config/* config/ 2>/dev/null || true
cp ~/jumbo_setup/scripts/* scripts/ 2>/dev/null || true

# Create enhanced hostapd configuration
echo -e "${YELLOW}📡 Configuring WiFi Access Point...${NC}"
sudo tee /etc/hostapd/hostapd.conf > /dev/null <<EOF
# MICRO BOT WiFi Access Point Configuration
interface=wlan0
driver=nl80211
ssid=JumboBot_Network
hw_mode=g
channel=7
wmm_enabled=0
macaddr_acl=0
auth_algs=1
ignore_broadcast_ssid=0
wpa=2
wpa_passphrase=JumboBot2025
wpa_key_mgmt=WPA-PSK
wpa_pairwise=TKIP
rsn_pairwise=CCMP

# Performance optimizations
beacon_int=100
dtim_period=2
max_num_sta=20
rts_threshold=2347
fragm_threshold=2346
EOF

# Configure hostapd daemon
sudo sed -i 's|#DAEMON_CONF=""|DAEMON_CONF="/etc/hostapd/hostapd.conf"|' /etc/default/hostapd

# Create enhanced dnsmasq configuration
echo -e "${YELLOW}🌐 Configuring DHCP server...${NC}"
sudo cp /etc/dnsmasq.conf /etc/dnsmasq.conf.backup
sudo tee /etc/dnsmasq.conf > /dev/null <<EOF
# MICRO BOT Enhanced DHCP Configuration

# Basic configuration
port=53
domain-needed
bogus-priv
no-resolv

# Interface configuration
interface=wlan0
listen-address=192.168.4.1
bind-interfaces

# DHCP configuration
dhcp-range=192.168.4.10,192.168.4.50,255.255.255.0,12h
dhcp-option=option:router,192.168.4.1
dhcp-option=option:dns-server,8.8.8.8,8.8.4.4

# Static leases for known bots (optional)
# dhcp-host=<mac-address>,WHEELIE,192.168.4.10
# dhcp-host=<mac-address>,SPEEDIE,192.168.4.11

# Logging
log-dhcp
log-queries=extra
EOF

# Configure network interface
echo -e "${YELLOW}🔗 Configuring network interface...${NC}"
sudo tee -a /etc/dhcpcd.conf > /dev/null <<EOF

# MICRO BOT Static IP Configuration
interface wlan0
static ip_address=192.168.4.1/24
nohook wpa_supplicant
EOF

# Enable IP forwarding
echo -e "${YELLOW}🔄 Enabling IP forwarding...${NC}"
sudo sed -i 's/#net.ipv4.ip_forward=1/net.ipv4.ip_forward=1/' /etc/sysctl.conf
echo 'net.ipv4.ip_forward=1' | sudo tee -a /etc/sysctl.conf

# Configure iptables for NAT and security
echo -e "${YELLOW}🛡️ Configuring firewall rules...${NC}"
sudo iptables -F
sudo iptables -t nat -F

# NAT rules
sudo iptables -t nat -A POSTROUTING -o eth0 -j MASQUERADE
sudo iptables -t nat -A POSTROUTING -o wlan1 -j MASQUERADE

# Forwarding rules
sudo iptables -A FORWARD -i eth0 -o wlan0 -m state --state RELATED,ESTABLISHED -j ACCEPT
sudo iptables -A FORWARD -i wlan0 -o eth0 -j ACCEPT
sudo iptables -A FORWARD -i wlan1 -o wlan0 -m state --state RELATED,ESTABLISHED -j ACCEPT
sudo iptables -A FORWARD -i wlan0 -o wlan1 -j ACCEPT

# Security rules
sudo iptables -A INPUT -i lo -j ACCEPT
sudo iptables -A INPUT -i wlan0 -p tcp --dport 8765 -j ACCEPT  # Bot port
sudo iptables -A INPUT -i eth0 -p tcp --dport 8766 -j ACCEPT   # PC port
sudo iptables -A INPUT -i wlan1 -p tcp --dport 8766 -j ACCEPT  # PC port via WiFi
sudo iptables -A INPUT -i wlan0 -p tcp --dport 22 -j ACCEPT    # SSH
sudo iptables -A INPUT -i eth0 -p tcp --dport 22 -j ACCEPT     # SSH
sudo iptables -A INPUT -m state --state RELATED,ESTABLISHED -j ACCEPT

# Save iptables rules
sudo sh -c "iptables-save > /etc/iptables.ipv4.nat"

# Create enhanced systemd service
echo -e "${YELLOW}⚙️ Creating system service...${NC}"
sudo tee /etc/systemd/system/micro-bot.service > /dev/null <<EOF
[Unit]
Description=MICRO BOT Bridge Server - Swarm Communication Hub
After=network.target hostapd.service dnsmasq.service
Wants=hostapd.service dnsmasq.service
Requires=network.target

[Service]
Type=simple
User=pi
Group=pi
WorkingDirectory=/home/pi/micro_bot
Environment=PATH=/home/pi/micro_bot/venv/bin:/usr/local/bin:/usr/bin:/bin
Environment=PYTHONPATH=/home/pi/micro_bot
ExecStart=/home/pi/micro_bot/venv/bin/python bridge_server.py
ExecReload=/bin/kill -HUP \$MAINPID
KillMode=mixed
Restart=always
RestartSec=10
TimeoutStopSec=30

# Logging
StandardOutput=journal
StandardError=journal
SyslogIdentifier=micro-bot

# Security
NoNewPrivileges=true
PrivateTmp=true

# Resource limits
LimitNOFILE=65536
LimitNPROC=4096

[Install]
WantedBy=multi-user.target
EOF

# Create startup script
echo -e "${YELLOW}🚀 Creating startup script...${NC}"
tee /home/pi/micro_bot/start_bridge.sh > /dev/null <<'EOF'
#!/bin/bash
# MICRO BOT Bridge Startup Script

SCRIPT_DIR="/home/pi/micro_bot"
LOG_FILE="$SCRIPT_DIR/logs/startup.log"

# Create logs directory
mkdir -p "$SCRIPT_DIR/logs"

echo "$(date): 🤖 Starting MICRO BOT Bridge..." | tee -a "$LOG_FILE"

# Check network interface
if ! ip addr show wlan0 | grep -q "192.168.4.1"; then
    echo "$(date): ⚠️ Warning: wlan0 not configured correctly" | tee -a "$LOG_FILE"
fi

# Activate virtual environment
source "$SCRIPT_DIR/venv/bin/activate"

# Start bridge server with logging
cd "$SCRIPT_DIR"
python bridge_server.py 2>&1 | tee -a "$LOG_FILE"
EOF
chmod +x /home/pi/micro_bot/start_bridge.sh

# Create enhanced status check script
echo -e "${YELLOW}📊 Creating status monitoring...${NC}"
tee /home/pi/micro_bot/check_status.sh > /dev/null <<'EOF'
#!/bin/bash
# MICRO BOT Enhanced Status Check

# Color codes
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
PURPLE='\033[0;35m'
CYAN='\033[0;36m'
NC='\033[0m'

echo -e "${PURPLE}🤖 MICRO BOT Status Dashboard${NC}"
echo -e "${BLUE}==============================${NC}"
echo ""

# Service Status
echo -e "${CYAN}📋 Service Status:${NC}"
if sudo systemctl is-active micro-bot.service >/dev/null 2>&1; then
    echo -e "  Bridge Service: ${GREEN}✅ Running${NC}"
else
    echo -e "  Bridge Service: ${RED}❌ Stopped${NC}"
fi

if sudo systemctl is-active hostapd.service >/dev/null 2>&1; then
    echo -e "  WiFi AP: ${GREEN}✅ Running${NC}"
else
    echo -e "  WiFi AP: ${RED}❌ Stopped${NC}"
fi

if sudo systemctl is-active dnsmasq.service >/dev/null 2>&1; then
    echo -e "  DHCP Server: ${GREEN}✅ Running${NC}"
else
    echo -e "  DHCP Server: ${RED}❌ Stopped${NC}"
fi
echo ""

# Network Status
echo -e "${CYAN}🌐 Network Status:${NC}"
if ip addr show wlan0 | grep -q "192.168.4.1"; then
    echo -e "  Bridge IP: ${GREEN}✅ 192.168.4.1${NC}"
else
    echo -e "  Bridge IP: ${RED}❌ Not configured${NC}"
fi

# WiFi Status
WIFI_STATUS=$(iwconfig wlan0 2>/dev/null | grep "Access Point" | awk '{print $4}')
if [ ! -z "$WIFI_STATUS" ]; then
    echo -e "  WiFi Mode: ${GREEN}✅ Access Point${NC}"
else
    echo -e "  WiFi Mode: ${RED}❌ Not in AP mode${NC}"
fi
echo ""

# Connected Devices
echo -e "${CYAN}🔌 Connected Devices:${NC}"
CONNECTED=$(sudo arp-scan --local --interface=wlan0 2>/dev/null | grep -E "192\.168\.4\." | wc -l)
if [ "$CONNECTED" -gt 0 ]; then
    echo -e "  Device Count: ${GREEN}$CONNECTED devices${NC}"
    sudo arp-scan --local --interface=wlan0 2>/dev/null | grep -E "192\.168\.4\." | while read line; do
        IP=$(echo $line | awk '{print $1}')
        MAC=$(echo $line | awk '{print $2}')
        echo -e "    ${YELLOW}$IP${NC} - $MAC"
    done
else
    echo -e "  Device Count: ${YELLOW}0 devices${NC}"
fi
echo ""

# WebSocket Ports
echo -e "${CYAN}🔌 WebSocket Ports:${NC}"
if netstat -ln | grep -q ":8765"; then
    echo -e "  Bot Port (8765): ${GREEN}✅ Listening${NC}"
else
    echo -e "  Bot Port (8765): ${RED}❌ Not listening${NC}"
fi

if netstat -ln | grep -q ":8766"; then
    echo -e "  PC Port (8766): ${GREEN}✅ Listening${NC}"
else
    echo -e "  PC Port (8766): ${RED}❌ Not listening${NC}"
fi
echo ""

# System Resources
echo -e "${CYAN}💻 System Resources:${NC}"
CPU_USAGE=$(top -bn1 | grep "Cpu(s)" | awk '{print $2}' | awk -F'%' '{print $1}')
MEMORY_USAGE=$(free | grep Mem | awk '{printf("%.1f", $3/$2 * 100.0)}')
DISK_USAGE=$(df -h / | awk 'NR==2{printf "%s", $5}')
TEMP=$(vcgencmd measure_temp 2>/dev/null | cut -d= -f2 | cut -d\' -f1 || echo "N/A")

echo -e "  CPU Usage: ${YELLOW}$CPU_USAGE%${NC}"
echo -e "  Memory Usage: ${YELLOW}$MEMORY_USAGE%${NC}"
echo -e "  Disk Usage: ${YELLOW}$DISK_USAGE${NC}"
echo -e "  Temperature: ${YELLOW}$TEMP°C${NC}"
echo ""

# Recent Logs
echo -e "${CYAN}📝 Recent Bridge Logs:${NC}"
sudo journalctl -u micro-bot.service --lines=3 --no-pager | tail -3
echo ""

# Quick Commands
echo -e "${CYAN}🛠️ Quick Commands:${NC}"
echo -e "  Restart Bridge: ${YELLOW}sudo systemctl restart micro-bot.service${NC}"
echo -e "  View Live Logs: ${YELLOW}sudo journalctl -u micro-bot.service -f${NC}"
echo -e "  Restart WiFi: ${YELLOW}sudo systemctl restart hostapd dnsmasq${NC}"
EOF
chmod +x /home/pi/micro_bot/check_status.sh

# Create update script
tee /home/pi/micro_bot/update_system.sh > /dev/null <<'EOF'
#!/bin/bash
# MICRO BOT System Update Script

echo "🤖 MICRO BOT System Update"
echo "========================="

# Backup current configuration
echo "📋 Creating backup..."
mkdir -p /home/pi/micro_bot/backups/$(date +%Y%m%d_%H%M%S)
cp /etc/hostapd/hostapd.conf /home/pi/micro_bot/backups/$(date +%Y%m%d_%H%M%S)/
cp /etc/dnsmasq.conf /home/pi/micro_bot/backups/$(date +%Y%m%d_%H%M%S)/

# Update system
echo "📦 Updating packages..."
sudo apt update && sudo apt upgrade -y

# Update Python packages
echo "🐍 Updating Python packages..."
source /home/pi/micro_bot/venv/bin/activate
pip install --upgrade pip websockets asyncio aiofiles python-dateutil psutil

echo "✅ Update completed!"
echo "🔄 Please reboot to ensure all changes take effect"
EOF
chmod +x /home/pi/micro_bot/update_system.sh

# Create logs directory
mkdir -p /home/pi/micro_bot/logs

# Create info file
tee /home/pi/micro_bot/INFO.txt > /dev/null <<EOF
🤖 MICRO BOT Enhanced Bridge System
====================================

Platform: Raspberry Pi 3 Model B v1.2
Function: Swarm Communication Bridge
Integration: WHEELIE/SPEEDIE Ecosystem

Network Configuration:
- WiFi SSID: JumboBot_Network
- WiFi Password: JumboBot2025
- Bridge IP: 192.168.4.1
- DHCP Range: 192.168.4.10 - 192.168.4.50

WebSocket Ports:
- Bot Communication: 8765
- PC Communication: 8766

File Locations:
- Bridge Server: /home/pi/micro_bot/bridge_server.py
- Configuration: /home/pi/micro_bot/config/
- Logs: /home/pi/micro_bot/logs/
- Scripts: /home/pi/micro_bot/scripts/

Management Commands:
- Status Check: ./check_status.sh
- Start Bridge: sudo systemctl start micro-bot.service
- Stop Bridge: sudo systemctl stop micro-bot.service
- Restart Bridge: sudo systemctl restart micro-bot.service
- View Logs: sudo journalctl -u micro-bot.service -f
- Update System: ./update_system.sh

Installation Date: $(date)
Installation Script Version: 2.0 (Enhanced)
EOF

# Enable services
echo -e "${YELLOW}🔧 Enabling services...${NC}"
sudo systemctl daemon-reload
sudo systemctl enable hostapd
sudo systemctl enable dnsmasq
sudo systemctl enable micro-bot.service

# Create rc.local for iptables restoration
echo -e "${YELLOW}🛡️ Configuring boot scripts...${NC}"
sudo tee /etc/rc.local > /dev/null <<EOF
#!/bin/sh -e
# MICRO BOT Boot Script

# Restore iptables rules
iptables-restore < /etc/iptables.ipv4.nat

# Ensure WiFi interface is up
ip link set wlan0 up

exit 0
EOF
sudo chmod +x /etc/rc.local

# Final status
echo ""
echo -e "${GREEN}✅ MICRO BOT Enhanced Setup Complete!${NC}"
echo ""
echo -e "${BLUE}📋 Next Steps:${NC}"
echo -e "${YELLOW}1. Reboot the Raspberry Pi: ${CYAN}sudo reboot${NC}"
echo -e "${YELLOW}2. After reboot, check status: ${CYAN}./check_status.sh${NC}"
echo -e "${YELLOW}3. Connect bots to WiFi: ${CYAN}JumboBot_Network${NC}"
echo -e "${YELLOW}4. Test WebSocket connections${NC}"
echo ""
echo -e "${BLUE}🌐 Network Information:${NC}"
echo -e "${CYAN}- SSID: JumboBot_Network${NC}"
echo -e "${CYAN}- Password: JumboBot2025${NC}"
echo -e "${CYAN}- Bridge IP: 192.168.4.1${NC}"
echo -e "${CYAN}- Bot WebSocket: ws://192.168.4.1:8765${NC}"
echo -e "${CYAN}- PC WebSocket: ws://192.168.4.1:8766${NC}"
echo ""
echo -e "${PURPLE}🤖 MICRO BOT is ready for swarm operations!${NC}"