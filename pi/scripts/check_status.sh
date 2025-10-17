#!/bin/bash
# MICRO BOT Enhanced Status Check Script

# Color codes
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
PURPLE='\033[0;35m'
CYAN='\033[0;36m'
NC='\033[0m'

# Function to check if command exists
command_exists() {
    command -v "$1" >/dev/null 2>&1
}

# Function to get service status with color
get_service_status() {
    local service=$1
    if sudo systemctl is-active "$service" >/dev/null 2>&1; then
        echo -e "${GREEN}✅ Running${NC}"
    else
        echo -e "${RED}❌ Stopped${NC}"
    fi
}

# Function to check port status
check_port() {
    local port=$1
    if netstat -ln | grep -q ":$port"; then
        echo -e "${GREEN}✅ Listening${NC}"
    else
        echo -e "${RED}❌ Not listening${NC}"
    fi
}

# Header
clear
echo -e "${PURPLE}🤖 MICRO BOT Status Dashboard${NC}"
echo -e "${BLUE}==============================${NC}"
echo -e "${CYAN}$(date)${NC}"
echo ""

# System Information
echo -e "${CYAN}💻 System Information:${NC}"
echo -e "  Hostname: ${YELLOW}$(hostname)${NC}"
echo -e "  Uptime: ${YELLOW}$(uptime -p)${NC}"
if command_exists vcgencmd; then
    TEMP=$(vcgencmd measure_temp 2>/dev/null | cut -d= -f2 | cut -d\' -f1 || echo "N/A")
    echo -e "  CPU Temperature: ${YELLOW}$TEMP°C${NC}"
fi
echo ""

# Service Status
echo -e "${CYAN}📋 Service Status:${NC}"
echo -e "  Bridge Service: $(get_service_status micro-bot.service)"
echo -e "  WiFi AP (hostapd): $(get_service_status hostapd.service)"
echo -e "  DHCP Server (dnsmasq): $(get_service_status dnsmasq.service)"
echo -e "  SSH Server: $(get_service_status ssh.service)"
echo ""

# Network Status
echo -e "${CYAN}🌐 Network Configuration:${NC}"
if ip addr show wlan0 | grep -q "192.168.4.1"; then
    echo -e "  Bridge IP (wlan0): ${GREEN}✅ 192.168.4.1/24${NC}"
else
    echo -e "  Bridge IP (wlan0): ${RED}❌ Not configured${NC}"
fi

# WiFi Access Point Status
if command_exists iwconfig; then
    WIFI_MODE=$(iwconfig wlan0 2>/dev/null | grep "Mode:" | awk -F'Mode:' '{print $2}' | awk '{print $1}')
    if [ "$WIFI_MODE" = "Master" ]; then
        echo -e "  WiFi Mode: ${GREEN}✅ Access Point${NC}"
        ESSID=$(iwconfig wlan0 2>/dev/null | grep "ESSID:" | awk -F'ESSID:' '{print $2}' | tr -d '"')
        echo -e "  SSID: ${YELLOW}$ESSID${NC}"
    else
        echo -e "  WiFi Mode: ${RED}❌ Not in AP mode${NC}"
    fi
fi

# Ethernet status
if ip addr show eth0 >/dev/null 2>&1; then
    ETH_IP=$(ip addr show eth0 | grep "inet " | awk '{print $2}' | head -1)
    if [ ! -z "$ETH_IP" ]; then
        echo -e "  Ethernet IP: ${GREEN}✅ $ETH_IP${NC}"
    else
        echo -e "  Ethernet IP: ${YELLOW}⚠️ No IP assigned${NC}"
    fi
fi
echo ""

# WebSocket Ports
echo -e "${CYAN}🔌 WebSocket Ports:${NC}"
echo -e "  Bot Port (8765): $(check_port 8765)"
echo -e "  PC Port (8766): $(check_port 8766)"
echo ""

# Connected Devices
echo -e "${CYAN}🤖 Connected Bot Devices:${NC}"
if command_exists arp-scan; then
    DEVICES=$(sudo arp-scan --local --interface=wlan0 2>/dev/null | grep -E "192\.168\.4\." | grep -v "192.168.4.1")
    DEVICE_COUNT=$(echo "$DEVICES" | wc -l)
    if [ ! -z "$DEVICES" ] && [ "$DEVICE_COUNT" -gt 0 ]; then
        echo -e "  Device Count: ${GREEN}$DEVICE_COUNT connected${NC}"
        echo "$DEVICES" | while read line; do
            IP=$(echo $line | awk '{print $1}')
            MAC=$(echo $line | awk '{print $2}')
            VENDOR=$(echo $line | awk '{print $3}' | cut -c1-20)
            echo -e "    ${YELLOW}$IP${NC} - $MAC ${CYAN}($VENDOR)${NC}"
        done
    else
        echo -e "  Device Count: ${YELLOW}0 devices${NC}"
    fi
else
    # Alternative method using ARP table
    ARP_DEVICES=$(arp -a | grep "192.168.4." | grep -v "192.168.4.1")
    if [ ! -z "$ARP_DEVICES" ]; then
        DEVICE_COUNT=$(echo "$ARP_DEVICES" | wc -l)
        echo -e "  Device Count: ${GREEN}$DEVICE_COUNT in ARP table${NC}"
        echo "$ARP_DEVICES" | while read line; do
            echo -e "    ${YELLOW}$line${NC}"
        done
    else
        echo -e "  Device Count: ${YELLOW}0 devices${NC}"
    fi
fi
echo ""

# DHCP Leases
echo -e "${CYAN}📋 DHCP Leases:${NC}"
if [ -f "/var/lib/dhcp/dhcpcd.leases" ]; then
    LEASE_COUNT=$(grep -c "lease " /var/lib/dhcp/dhcpcd.leases 2>/dev/null || echo "0")
    echo -e "  Active Leases: ${YELLOW}$LEASE_COUNT${NC}"
elif [ -f "/var/lib/dhcpcd5/dhcpcd.leases" ]; then
    LEASE_COUNT=$(grep -c "lease " /var/lib/dhcpcd5/dhcpcd.leases 2>/dev/null || echo "0")
    echo -e "  Active Leases: ${YELLOW}$LEASE_COUNT${NC}"
else
    echo -e "  DHCP Leases: ${YELLOW}File not found${NC}"
fi
echo ""

# System Resources
echo -e "${CYAN}📊 System Resources:${NC}"
if command_exists top; then
    CPU_USAGE=$(top -bn1 | grep "Cpu(s)" | awk '{print $2}' | awk -F'%' '{print $1}' 2>/dev/null || echo "N/A")
    echo -e "  CPU Usage: ${YELLOW}$CPU_USAGE%${NC}"
fi

MEMORY_INFO=$(free | grep Mem)
MEMORY_TOTAL=$(echo $MEMORY_INFO | awk '{print $2}')
MEMORY_USED=$(echo $MEMORY_INFO | awk '{print $3}')
MEMORY_USAGE=$(awk "BEGIN {printf \"%.1f\", $MEMORY_USED/$MEMORY_TOTAL * 100}")
echo -e "  Memory Usage: ${YELLOW}$MEMORY_USAGE%${NC} ($(($MEMORY_USED/1024))MB / $(($MEMORY_TOTAL/1024))MB)"

DISK_USAGE=$(df -h / | awk 'NR==2{print $5}' 2>/dev/null || echo "N/A")
echo -e "  Disk Usage (/): ${YELLOW}$DISK_USAGE${NC}"

if [ -f "/sys/class/thermal/thermal_zone0/temp" ]; then
    TEMP_RAW=$(cat /sys/class/thermal/thermal_zone0/temp 2>/dev/null || echo "0")
    TEMP_C=$((TEMP_RAW / 1000))
    echo -e "  CPU Temperature: ${YELLOW}$TEMP_C°C${NC}"
fi
echo ""

# Bridge Server Process
echo -e "${CYAN}🔧 Bridge Server Process:${NC}"
BRIDGE_PID=$(pgrep -f "python.*bridge_server.py" 2>/dev/null)
if [ ! -z "$BRIDGE_PID" ]; then
    echo -e "  Process ID: ${GREEN}$BRIDGE_PID${NC}"
    if command_exists ps; then
        BRIDGE_CPU=$(ps -p $BRIDGE_PID -o %cpu= 2>/dev/null | tr -d ' ')
        BRIDGE_MEM=$(ps -p $BRIDGE_PID -o %mem= 2>/dev/null | tr -d ' ')
        echo -e "  CPU Usage: ${YELLOW}$BRIDGE_CPU%${NC}"
        echo -e "  Memory Usage: ${YELLOW}$BRIDGE_MEM%${NC}"
    fi
else
    echo -e "  Process: ${RED}❌ Not running${NC}"
fi
echo ""

# Recent Logs
echo -e "${CYAN}📝 Recent Bridge Logs:${NC}"
if sudo journalctl -u micro-bot.service --lines=3 --no-pager >/dev/null 2>&1; then
    sudo journalctl -u micro-bot.service --lines=3 --no-pager | tail -3 | while read line; do
        echo -e "  ${YELLOW}$line${NC}"
    done
else
    echo -e "  ${YELLOW}No service logs available${NC}"
fi
echo ""

# Network Connectivity Test
echo -e "${CYAN}🌐 Network Connectivity:${NC}"
if ping -c 1 8.8.8.8 >/dev/null 2>&1; then
    echo -e "  Internet: ${GREEN}✅ Connected${NC}"
else
    echo -e "  Internet: ${RED}❌ No connection${NC}"
fi

if ping -c 1 192.168.4.1 >/dev/null 2>&1; then
    echo -e "  Local Bridge: ${GREEN}✅ Reachable${NC}"
else
    echo -e "  Local Bridge: ${RED}❌ Unreachable${NC}"
fi
echo ""

# Quick Commands Reference
echo -e "${CYAN}🛠️ Quick Commands:${NC}"
echo -e "  Restart Bridge: ${YELLOW}sudo systemctl restart micro-bot.service${NC}"
echo -e "  View Live Logs: ${YELLOW}sudo journalctl -u micro-bot.service -f${NC}"
echo -e "  Restart WiFi: ${YELLOW}sudo systemctl restart hostapd dnsmasq${NC}"
echo -e "  Manual Start: ${YELLOW}cd /home/pi/micro_bot && ./scripts/start_bridge.sh${NC}"
echo -e "  Update System: ${YELLOW}cd /home/pi/micro_bot && ./update_system.sh${NC}"
echo ""

echo -e "${PURPLE}🤖 MICRO BOT Status Check Complete${NC}"