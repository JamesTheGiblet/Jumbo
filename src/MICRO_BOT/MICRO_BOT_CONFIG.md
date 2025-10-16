# 🤖 MICRO BOT Configuration Guide

## Bot Identity

- **Name**: MICRO BOT
- **Type**: Communication Bridge & Hub
- **Platform**: Raspberry Pi 3 Model B v1.2
- **Function**: Static WiFi bridge between ESP32 bots and PC

## Hardware Platform

### Raspberry Pi 3 Model B v1.2 Specifications

- **CPU**: BCM2837 quad-core ARM Cortex-A53 @ 1.2GHz
- **RAM**: 1GB LPDDR2 SDRAM
- **WiFi**: 802.11n Wireless LAN
- **Bluetooth**: 4.1 & BLE
- **GPIO**: 40-pin header
- **Ethernet**: 10/100 Ethernet port
- **USB**: 4x USB 2.0 ports
- **Storage**: MicroSD card slot
- **Power**: 5V micro USB (2.5A recommended)

## System Architecture

### Communication Flow

```
ESP32 Bots (WHEELIE/SPEEDIE) 
    ↓ WiFi
MICRO BOT (Raspberry Pi 3)
    ↓ WiFi/Ethernet  
PC MCU Program
```

### Network Configuration

- **Bot WiFi Network**: `JumboBot_Network`
- **Bridge IP**: `192.168.4.1` (Access Point mode)
- **Bot Port**: `8765` (WebSocket for ESP32 connections)
- **PC Port**: `8766` (WebSocket for PC connections)
- **PC Connection**: WiFi or Ethernet to home network

## Key Features

### Signal Processing

- **Real-time Filtering**: Rate limiting, duplicate detection
- **Priority Queuing**: High/medium/low priority message handling
- **Message Routing**: Intelligent forwarding between bots and PC
- **Data Logging**: Persistent storage of bot communications
- **Error Handling**: Connection recovery and error reporting

### Bot Management

- **Status Monitoring**: Track bot health, fitness, generations
- **Command Forwarding**: Send commands from PC to specific bots
- **Connection Management**: Handle bot connections/disconnections
- **Performance Metrics**: Message counts, error rates, uptime

### PC Interface

- **WebSocket API**: Real-time bidirectional communication
- **Status Dashboard**: System health and bot overview
- **Command Interface**: Bot control and configuration
- **Data Export**: Historical data and analytics

## Installation & Setup

### Prerequisites

```bash
# Update Raspberry Pi OS
sudo apt update && sudo apt upgrade -y

# Install Python dependencies
sudo apt install python3-pip python3-venv -y

# Create project directory
mkdir /home/pi/micro_bot
cd /home/pi/micro_bot

# Create virtual environment
python3 -m venv venv
source venv/bin/activate

# Install required packages
pip install websockets asyncio
```

### Network Setup

```bash
# Configure WiFi Access Point
sudo apt install hostapd dnsmasq -y

# Edit hostapd configuration
sudo nano /etc/hostapd/hostapd.conf
```

### Access Point Configuration (`/etc/hostapd/hostapd.conf`)

```
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
```

### DHCP Configuration (`/etc/dnsmasq.conf`)

```
interface=wlan0
dhcp-range=192.168.4.2,192.168.4.20,255.255.255.0,24h
```

### Auto-start Service

Create systemd service file: `/etc/systemd/system/micro-bot.service`

```
[Unit]
Description=Micro Bot Bridge Server
After=network.target

[Service]
Type=simple
User=pi
WorkingDirectory=/home/pi/micro_bot
Environment=PATH=/home/pi/micro_bot/venv/bin
ExecStart=/home/pi/micro_bot/venv/bin/python bridge_server.py
Restart=always
RestartSec=10

[Install]
WantedBy=multi-user.target
```

Enable and start service:

```bash
sudo systemctl enable micro-bot.service
sudo systemctl start micro-bot.service
```

## Message Protocol

### Bot to MICRO BOT Messages

```json
{
    "bot_id": "SPEEDIE-001",
    "type": "status|evolution|obstacle|strategy|signal",
    "timestamp": 1698000000.123,
    "priority": 1-3,
    "data": {
        "generation": 42,
        "fitness_score": 0.85,
        "obstacle_distance": 150,
        "strategy_used": "learned",
        "emotional_state": {
            "frustration": 20,
            "confidence": 80
        }
    }
}
```

### PC to MICRO BOT Commands

```json
{
    "type": "command",
    "command": "get_status|get_bots|restart_bot",
    "bot_id": "SPEEDIE-001",
    "parameters": {}
}
```

### MICRO BOT to PC Status

```json
{
    "type": "status_update",
    "timestamp": 1698000000.123,
    "uptime": 86400.0,
    "connected_bots": 2,
    "stats": {
        "messages_processed": 15420,
        "messages_filtered": 3240,
        "messages_forwarded": 12180
    }
}
```

## Monitoring & Maintenance

### Log Files

- **Main Log**: `/home/pi/micro_bot.log`
- **System Log**: `journalctl -u micro-bot.service`
- **Access Point**: `/var/log/hostapd.log`

### Performance Monitoring

```bash
# Check service status
sudo systemctl status micro-bot.service

# Monitor real-time logs
tail -f /home/pi/micro_bot.log

# Check network connections
netstat -an | grep :876

# Monitor system resources
htop
```

### Troubleshooting

- **WiFi Issues**: Check hostapd and dnsmasq status
- **Connection Problems**: Verify WebSocket ports are open
- **Performance**: Monitor CPU and memory usage
- **Bot Communication**: Check message filtering rules

## Security Considerations

- **Network Isolation**: Bots on separate WiFi network
- **Access Control**: Limited connection attempts
- **Message Validation**: JSON schema validation
- **Log Rotation**: Automatic log cleanup
- **Firewall**: Only necessary ports open

## Expansion Capabilities

- **Multi-Bot Support**: Scalable to 10+ bots
- **Database Integration**: PostgreSQL for data persistence  
- **Web Dashboard**: Real-time monitoring interface
- **API Extensions**: REST API for external integrations
- **Machine Learning**: Pattern recognition in bot behavior
- **Remote Access**: SSH tunneling for remote monitoring

## Expected Behavior

- **Startup**: Initialize WiFi AP, start WebSocket servers
- **Bot Discovery**: Automatic registration of connecting bots
- **Signal Processing**: Real-time filtering and routing
- **PC Communication**: Bidirectional command/status exchange
- **Health Monitoring**: Continuous bot status tracking
- **Graceful Shutdown**: Clean disconnection handling
