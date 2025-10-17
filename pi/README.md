# 🤖 MICRO BOT (Raspberry Pi) Setup

The **MICRO BOT** is a **Raspberry Pi 3 Model B v1.2** that serves as a **WiFi communication bridge** between the ESP32 swarm bots (WHEELIE/SPEEDIE) and the PC MCU.

## 🏗️ Architecture Role

```
ESP32 Bots (WHEELIE/SPEEDIE) ↔ WiFi ↔ MICRO BOT (Pi 3) ↔ WiFi/Ethernet ↔ PC MCU
```

The Pi creates a **WiFi Access Point** that the ESP32 bots connect to, and routes their messages to/from the PC via **WebSocket bridges**.

## 🔧 Hardware Specifications

- **Platform**: Raspberry Pi 3 Model B v1.2
- **CPU**: BCM2837 quad-core ARM Cortex-A53 @ 1.2GHz
- **RAM**: 1GB LPDDR2
- **WiFi**: 802.11n wireless LAN
- **Ethernet**: 10/100 Ethernet port
- **GPIO**: 40-pin header (for future expansions)

## 🌐 Network Configuration

- **WiFi SSID**: `JumboBot_Network`
- **WiFi Password**: `JumboBot2025`
- **Bridge IP**: `192.168.4.1`
- **DHCP Range**: `192.168.4.2` - `192.168.4.20`
- **Bot WebSocket Port**: `8765`
- **PC WebSocket Port**: `8766`

## 🚀 Quick Setup

### 1. Flash Raspberry Pi OS

```bash
# Use Raspberry Pi Imager to flash Raspberry Pi OS Lite
# Enable SSH and configure initial WiFi if needed
```

### 2. Run Installation Script

```bash
# Copy files to Pi and run installation
scp -r pi/ pi@<pi-ip-address>:/home/pi/jumbo_setup/
ssh pi@<pi-ip-address>
cd /home/pi/jumbo_setup
sudo ./setup_micro_bot.sh
```

### 3. Verify Installation

```bash
# Check system status
./check_status.sh

# View bridge logs
sudo journalctl -u micro-bot.service -f
```

## 🔌 WebSocket Communication

### Bot Connection (Port 8765)

```python
# ESP32 bots connect to:
ws://192.168.4.1:8765
```

### PC Connection (Port 8766)

```python
# PC MCU connects to:
ws://192.168.4.1:8766
```

## 📋 Message Routing

The bridge server handles message routing between bots and PC:

1. **Bot → PC**: Messages from ESP32 bots are forwarded to PC
2. **PC → Bot**: Commands from PC are routed to specific bots
3. **Bot ↔ Bot**: Inter-bot communication via PC coordination
4. **Monitoring**: Connection status and message logging

## 🔧 Management Commands

```bash
# Service Control
sudo systemctl start micro-bot.service    # Start bridge
sudo systemctl stop micro-bot.service     # Stop bridge
sudo systemctl restart micro-bot.service  # Restart bridge
sudo systemctl status micro-bot.service   # Check status

# Network Management
sudo systemctl start hostapd              # Start WiFi AP
sudo systemctl start dnsmasq              # Start DHCP server

# Monitoring
./check_status.sh                         # System status
sudo journalctl -u micro-bot.service -f   # Live logs
sudo arp-scan --local --interface=wlan0   # Connected devices
```

## 📁 File Structure

```
pi/
├── README.md                    # This file
├── setup_micro_bot.sh          # Main installation script
├── bridge_server.py            # WebSocket bridge server
├── config/
│   ├── hostapd.conf            # WiFi AP configuration
│   ├── dnsmasq.conf            # DHCP server config
│   └── micro-bot.service       # Systemd service
├── scripts/
│   ├── start_bridge.sh         # Manual start script
│   ├── check_status.sh         # Status monitoring
│   └── update_system.sh        # System updates
└── docs/
    ├── TROUBLESHOOTING.md      # Common issues
    └── NETWORK_GUIDE.md        # Network setup guide
```

## 🐛 Troubleshooting

### Bridge Not Starting

```bash
# Check service logs
sudo journalctl -u micro-bot.service --no-pager

# Test WebSocket ports
netstat -ln | grep ":876"

# Restart services
sudo systemctl restart micro-bot.service
```

### WiFi AP Not Working

```bash
# Check hostapd status
sudo systemctl status hostapd

# Test WiFi interface
sudo iwconfig wlan0

# Restart WiFi services
sudo systemctl restart hostapd dnsmasq
```

### Bots Can't Connect

```bash
# Check DHCP assignments
sudo cat /var/lib/dhcp/dhcpcd.leases

# Scan for connected devices
sudo arp-scan --local --interface=wlan0

# Check firewall rules
sudo iptables -L -n
```

## 🔗 Integration

The MICRO BOT integrates with:

- **WHEELIE Bot**: Physical manipulation and movement
- **SPEEDIE Bot**: High-speed data processing and analysis
- **PC MCU**: Central coordination and user interface
- **Swarm Ecosystem Manager**: Layer 3 intelligence coordination

## 📊 Monitoring

The bridge provides real-time monitoring of:

- Connected bot count and status
- Message throughput and latency
- Network performance metrics
- Error rates and connection stability

## ⚡ Performance

- **Latency**: < 5ms for local bot communication
- **Throughput**: Up to 1000 messages/second
- **Connections**: Supports up to 20 concurrent bots
- **Reliability**: Auto-reconnection and error recovery
