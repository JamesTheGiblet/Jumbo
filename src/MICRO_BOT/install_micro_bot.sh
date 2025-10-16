#!/bin/bash
# 🤖 MICRO BOT Installation Script
# Raspberry Pi 3 Model B v1.2 Bridge Setup

set -e

echo "🤖 MICRO BOT Installation Starting..."
echo "Platform: Raspberry Pi 3 Model B v1.2"
echo "Function: WiFi Bridge for WHEELIE/SPEEDIE bots"

# Check if running as root
if [ "$EUID" -eq 0 ]; then
    echo "❌ Please run this script as pi user, not root"
    exit 1
fi

# Update system
echo "📦 Updating system packages..."
sudo apt update && sudo apt upgrade -y

# Install required packages
echo "🔧 Installing dependencies..."
sudo apt install -y python3-pip python3-venv hostapd dnsmasq iptables-persistent

# Create project directory
echo "📁 Creating project structure..."
mkdir -p /home/pi/micro_bot
cd /home/pi/micro_bot

# Create Python virtual environment
echo "🐍 Setting up Python environment..."
python3 -m venv venv
source venv/bin/activate

# Install Python packages
echo "📚 Installing Python dependencies..."
pip install --upgrade pip
pip install websockets asyncio

# Create hostapd configuration
echo "📡 Configuring WiFi Access Point..."
sudo tee /etc/hostapd/hostapd.conf > /dev/null <<EOF
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
EOF

# Configure hostapd daemon
sudo sed -i 's|#DAEMON_CONF=""|DAEMON_CONF="/etc/hostapd/hostapd.conf"|' /etc/default/hostapd

# Configure dnsmasq for DHCP
echo "🌐 Configuring DHCP server..."
sudo cp /etc/dnsmasq.conf /etc/dnsmasq.conf.backup
sudo tee -a /etc/dnsmasq.conf > /dev/null <<EOF

# MICRO BOT Configuration
interface=wlan0
dhcp-range=192.168.4.2,192.168.4.20,255.255.255.0,24h
EOF

# Configure static IP for wlan0
echo "🔗 Configuring network interface..."
sudo tee -a /etc/dhcpcd.conf > /dev/null <<EOF

# MICRO BOT Static IP
interface wlan0
static ip_address=192.168.4.1/24
nohook wpa_supplicant
EOF

# Enable IP forwarding
echo "🔄 Enabling IP forwarding..."
sudo sed -i 's/#net.ipv4.ip_forward=1/net.ipv4.ip_forward=1/' /etc/sysctl.conf

# Configure iptables for NAT
echo "🛡️ Configuring firewall rules..."
sudo iptables -t nat -A POSTROUTING -o eth0 -j MASQUERADE
sudo iptables -A FORWARD -i eth0 -o wlan0 -m state --state RELATED,ESTABLISHED -j ACCEPT
sudo iptables -A FORWARD -i wlan0 -o eth0 -j ACCEPT

# Save iptables rules
sudo sh -c "iptables-save > /etc/iptables.ipv4.nat"

# Create restore script for iptables
sudo tee /etc/rc.local > /dev/null <<EOF
#!/bin/sh -e
iptables-restore < /etc/iptables.ipv4.nat
exit 0
EOF
sudo chmod +x /etc/rc.local

# Create systemd service
echo "⚙️ Creating system service..."
sudo tee /etc/systemd/system/micro-bot.service > /dev/null <<EOF
[Unit]
Description=Micro Bot Bridge Server
After=network.target
Wants=hostapd.service dnsmasq.service

[Service]
Type=simple
User=pi
WorkingDirectory=/home/pi/micro_bot
Environment=PATH=/home/pi/micro_bot/venv/bin
ExecStart=/home/pi/micro_bot/venv/bin/python bridge_server.py
Restart=always
RestartSec=10
StandardOutput=journal
StandardError=journal

[Install]
WantedBy=multi-user.target
EOF

# Copy bridge server to project directory
if [ -f "bridge_server.py" ]; then
    echo "📋 Bridge server already exists"
else
    echo "❌ bridge_server.py not found - please copy manually"
fi

# Create startup script
echo "🚀 Creating startup script..."
tee /home/pi/micro_bot/start_micro_bot.sh > /dev/null <<EOF
#!/bin/bash
# MICRO BOT Startup Script

echo "🤖 Starting MICRO BOT Bridge..."

# Activate virtual environment
source /home/pi/micro_bot/venv/bin/activate

# Start bridge server
python bridge_server.py
EOF
chmod +x /home/pi/micro_bot/start_micro_bot.sh

# Create status check script
tee /home/pi/micro_bot/check_status.sh > /dev/null <<EOF
#!/bin/bash
# MICRO BOT Status Check

echo "🤖 MICRO BOT Status Check"
echo "========================="

# Check service status
echo "Service Status:"
sudo systemctl is-active micro-bot.service
echo ""

# Check network interfaces
echo "Network Interfaces:"
ip addr show wlan0 | grep inet
echo ""

# Check connected devices
echo "Connected Devices:"
sudo arp-scan --local --interface=wlan0 2>/dev/null | grep -E "192\.168\.4\." || echo "No devices found"
echo ""

# Check ports
echo "WebSocket Ports:"
netstat -ln | grep ":876" || echo "Ports not listening"
echo ""

# Check logs
echo "Recent Logs:"
sudo journalctl -u micro-bot.service --lines=5 --no-pager
EOF
chmod +x /home/pi/micro_bot/check_status.sh

# Enable services
echo "🔧 Enabling services..."
sudo systemctl enable hostapd
sudo systemctl enable dnsmasq
sudo systemctl enable micro-bot.service

# Create info file
tee /home/pi/micro_bot/INFO.txt > /dev/null <<EOF
🤖 MICRO BOT Bridge System
===========================

Platform: Raspberry Pi 3 Model B v1.2
WiFi Network: JumboBot_Network
Password: JumboBot2025
Bridge IP: 192.168.4.1

WebSocket Ports:
- Bots: 8765
- PC: 8766

Commands:
- Start: sudo systemctl start micro-bot.service
- Stop: sudo systemctl stop micro-bot.service
- Status: ./check_status.sh
- Logs: sudo journalctl -u micro-bot.service -f

Installation completed on: $(date)
EOF

echo ""
echo "✅ MICRO BOT Installation Complete!"
echo ""
echo "📋 Next Steps:"
echo "1. Copy bridge_server.py to /home/pi/micro_bot/"
echo "2. Reboot the Raspberry Pi: sudo reboot"
echo "3. Check status: ./check_status.sh"
echo "4. Connect bots to WiFi network: JumboBot_Network"
echo "5. Connect PC to same network or via Ethernet"
echo ""
echo "🌐 Network Details:"
echo "- SSID: JumboBot_Network"
echo "- Password: JumboBot2025"
echo "- Bridge IP: 192.168.4.1"
echo "- Bot WebSocket: ws://192.168.4.1:8765"
echo "- PC WebSocket: ws://192.168.4.1:8766"
echo ""
echo "📝 Check /home/pi/micro_bot/INFO.txt for reference"