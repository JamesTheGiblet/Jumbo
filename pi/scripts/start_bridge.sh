#!/bin/bash
# MICRO BOT Manual Bridge Startup Script

SCRIPT_DIR="/home/pi/micro_bot"
LOG_FILE="$SCRIPT_DIR/logs/manual_startup.log"
PID_FILE="$SCRIPT_DIR/bridge.pid"

# Color codes
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

echo -e "${BLUE}🤖 MICRO BOT Manual Bridge Startup${NC}"
echo -e "${BLUE}===================================${NC}"

# Create logs directory
mkdir -p "$SCRIPT_DIR/logs"

# Check if bridge is already running
if [ -f "$PID_FILE" ] && kill -0 $(cat "$PID_FILE") 2>/dev/null; then
    echo -e "${YELLOW}⚠️ Bridge is already running (PID: $(cat $PID_FILE))${NC}"
    echo -e "${YELLOW}Use 'sudo systemctl stop micro-bot.service' to stop it first${NC}"
    exit 1
fi

# Check network configuration
echo -e "${YELLOW}🔍 Checking network configuration...${NC}"
if ! ip addr show wlan0 | grep -q "192.168.4.1"; then
    echo -e "${RED}❌ Error: wlan0 not configured with IP 192.168.4.1${NC}"
    echo -e "${YELLOW}Please run setup script or configure manually${NC}"
    exit 1
fi

# Check WiFi AP
if ! iwconfig wlan0 2>/dev/null | grep -q "Mode:Master"; then
    echo -e "${YELLOW}⚠️ Warning: wlan0 not in Access Point mode${NC}"
    echo -e "${YELLOW}Starting hostapd service...${NC}"
    sudo systemctl start hostapd
    sleep 2
fi

# Check DHCP server
if ! sudo systemctl is-active dnsmasq >/dev/null 2>&1; then
    echo -e "${YELLOW}⚠️ Warning: dnsmasq not running${NC}"
    echo -e "${YELLOW}Starting dnsmasq service...${NC}"
    sudo systemctl start dnsmasq
    sleep 2
fi

echo -e "${GREEN}✅ Network services ready${NC}"

# Start bridge server
echo -e "${YELLOW}🚀 Starting bridge server...${NC}"
echo "$(date): Manual startup initiated" >> "$LOG_FILE"

# Change to bridge directory
cd "$SCRIPT_DIR"

# Activate virtual environment
if [ -f "venv/bin/activate" ]; then
    source venv/bin/activate
    echo -e "${GREEN}✅ Virtual environment activated${NC}"
else
    echo -e "${RED}❌ Virtual environment not found${NC}"
    exit 1
fi

# Check if bridge_server.py exists
if [ ! -f "bridge_server.py" ]; then
    echo -e "${RED}❌ bridge_server.py not found${NC}"
    exit 1
fi

# Start the bridge server in background
echo -e "${GREEN}🎯 Launching bridge server...${NC}"
python bridge_server.py > "$LOG_FILE" 2>&1 &
BRIDGE_PID=$!

# Save PID
echo $BRIDGE_PID > "$PID_FILE"

# Wait a moment and check if it's still running
sleep 3
if kill -0 $BRIDGE_PID 2>/dev/null; then
    echo -e "${GREEN}✅ Bridge server started successfully (PID: $BRIDGE_PID)${NC}"
    echo -e "${BLUE}📋 Bridge Status:${NC}"
    echo -e "  Bot WebSocket: ${YELLOW}ws://192.168.4.1:8765${NC}"
    echo -e "  PC WebSocket: ${YELLOW}ws://192.168.4.1:8766${NC}"
    echo -e "  Log File: ${YELLOW}$LOG_FILE${NC}"
    echo -e "  PID File: ${YELLOW}$PID_FILE${NC}"
    echo ""
    echo -e "${BLUE}🛠️ Management Commands:${NC}"
    echo -e "  Stop Bridge: ${YELLOW}kill $BRIDGE_PID${NC}"
    echo -e "  View Logs: ${YELLOW}tail -f $LOG_FILE${NC}"
    echo -e "  Check Status: ${YELLOW}./check_status.sh${NC}"
    echo ""
    echo -e "${GREEN}🤖 MICRO BOT Bridge is now running manually${NC}"
else
    echo -e "${RED}❌ Bridge server failed to start${NC}"
    echo -e "${YELLOW}Check the log file: $LOG_FILE${NC}"
    rm -f "$PID_FILE"
    exit 1
fi