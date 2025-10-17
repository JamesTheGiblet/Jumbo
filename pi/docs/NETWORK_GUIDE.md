# 🌐 MICRO BOT Network Configuration Guide

## Network Architecture Overview

```
┌─────────────────┐    WiFi     ┌─────────────────┐    Ethernet/WiFi    ┌─────────────────┐
│   ESP32 Bots    │◄──────────►│   MICRO BOT     │◄──────────────────►│      PC MCU     │
│ (WHEELIE/SPEEDIE) │             │ (Raspberry Pi)  │                     │   (Coordinator) │
└─────────────────┘             └─────────────────┘                     └─────────────────┘
  192.168.4.10-50                 192.168.4.1                           External Network
```

## WiFi Access Point Configuration

### Basic WiFi Settings

- **SSID**: `JumboBot_Network`
- **Password**: `JumboBot2025`
- **Security**: WPA2-PSK
- **Channel**: 7 (2.4GHz)
- **Max Clients**: 20

### Advanced WiFi Configuration

```bash
# /etc/hostapd/hostapd.conf
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
```

### Channel Selection Guidelines

- **Channel 1, 6, 11**: Non-overlapping 2.4GHz channels
- **Current**: Channel 7 (compromise for moderate interference)
- **Scan for interference**: `sudo iwlist wlan0 scan | grep -E "Channel|ESSID"`

## IP Address Management

### Network Topology

```
Network: 192.168.4.0/24
Subnet Mask: 255.255.255.0
Gateway: 192.168.4.1 (MICRO BOT)

Address Allocation:
├── 192.168.4.1      - MICRO BOT (Bridge)
├── 192.168.4.2-9    - Reserved for infrastructure
├── 192.168.4.10-29  - ESP32 Bots (DHCP)
├── 192.168.4.30-49  - Static assignments
└── 192.168.4.50-254 - Future expansion
```

### Static IP Assignments (Optional)

```bash
# Add to /etc/dnsmasq.conf for known bots
dhcp-host=AA:BB:CC:DD:EE:01,WHEELIE,192.168.4.10,12h
dhcp-host=AA:BB:CC:DD:EE:02,SPEEDIE,192.168.4.11,12h
```

## DHCP Server Configuration

### Basic DHCP Settings

```bash
# /etc/dnsmasq.conf
interface=wlan0
dhcp-range=192.168.4.10,192.168.4.29,255.255.255.0,12h
dhcp-option=option:router,192.168.4.1
dhcp-option=option:dns-server,8.8.8.8,8.8.4.4
```

### Advanced DHCP Configuration

```bash
# Enhanced /etc/dnsmasq.conf
port=53
domain-needed
bogus-priv
no-resolv
expand-hosts
domain=jumbobot.local

interface=wlan0
listen-address=192.168.4.1
bind-interfaces

# DHCP pool for bots
dhcp-range=192.168.4.10,192.168.4.29,255.255.255.0,12h
dhcp-option=option:router,192.168.4.1
dhcp-option=option:dns-server,8.8.8.8,8.8.4.4
dhcp-option=option:netmask,255.255.255.0

# Local domain resolution
address=/jumbobot.local/192.168.4.1
address=/bridge.jumbobot.local/192.168.4.1
address=/micro-bot.jumbobot.local/192.168.4.1

# Logging
log-dhcp
log-queries
```

## WebSocket Communication

### Port Configuration

```
Bot Communication Port: 8765
├── Protocol: WebSocket (ws://)
├── Endpoint: ws://192.168.4.1:8765
├── Purpose: ESP32 bot ↔ Bridge communication
└── Max Connections: 20 concurrent

PC Communication Port: 8766
├── Protocol: WebSocket (ws://)
├── Endpoint: ws://192.168.4.1:8766  
├── Purpose: PC MCU ↔ Bridge communication
└── Max Connections: 5 concurrent
```

### Message Routing

```python
# Bot → PC Message Flow
ESP32 Bot → ws://192.168.4.1:8765 → Bridge Server → ws://192.168.4.1:8766 → PC MCU

# PC → Bot Message Flow  
PC MCU → ws://192.168.4.1:8766 → Bridge Server → ws://192.168.4.1:8765 → ESP32 Bot
```

## Network Interface Configuration

### Primary Interface (wlan0)

```bash
# /etc/dhcpcd.conf addition
interface wlan0
static ip_address=192.168.4.1/24
nohook wpa_supplicant
```

### Secondary Interface (eth0) - Optional

```bash
# If using Ethernet for PC connection
interface eth0
static ip_address=10.0.0.100/24
static routers=10.0.0.1
static domain_name_servers=8.8.8.8 8.8.4.4
```

## Firewall and NAT Configuration

### iptables Rules

```bash
# NAT for internet access
iptables -t nat -A POSTROUTING -o eth0 -j MASQUERADE

# Allow forwarding between interfaces
iptables -A FORWARD -i eth0 -o wlan0 -m state --state RELATED,ESTABLISHED -j ACCEPT
iptables -A FORWARD -i wlan0 -o eth0 -j ACCEPT

# Allow WebSocket ports
iptables -A INPUT -i wlan0 -p tcp --dport 8765 -j ACCEPT  # Bot port
iptables -A INPUT -i eth0 -p tcp --dport 8766 -j ACCEPT   # PC port
iptables -A INPUT -i wlan0 -p tcp --dport 22 -j ACCEPT    # SSH

# Save rules
iptables-save > /etc/iptables.ipv4.nat
```

### Automatic Rule Restoration

```bash
# /etc/rc.local
#!/bin/sh -e
iptables-restore < /etc/iptables.ipv4.nat
exit 0
```

## Performance Optimization

### WiFi Performance Tuning

```bash
# Optimize WiFi for IoT devices
echo 'options cfg80211 ieee80211_regdom=US' >> /etc/modprobe.d/cfg80211.conf

# Power management (disable for stability)
iwconfig wlan0 power off

# Transmission power (adjust as needed)
iwconfig wlan0 txpower 20dBm
```

### Network Buffer Optimization

```bash
# /etc/sysctl.conf additions for better networking
net.core.rmem_max = 16777216
net.core.wmem_max = 16777216
net.ipv4.tcp_rmem = 4096 65536 16777216
net.ipv4.tcp_wmem = 4096 65536 16777216
```

## Monitoring and Diagnostics

### Network Monitoring Commands

```bash
# Real-time connection monitoring
watch -n 1 'sudo arp-scan --local --interface=wlan0'

# Traffic monitoring
sudo iftop -i wlan0

# Connection statistics
netstat -an | grep :876

# WiFi signal quality
watch -n 1 iwconfig
```

### DHCP Lease Monitoring

```bash
# View active DHCP leases
tail -f /var/log/dnsmasq.log

# Check lease database
cat /var/lib/dhcp/dhcpcd.leases
```

### WebSocket Connection Testing

```bash
# Test bot port
curl -i -N \
  -H "Connection: Upgrade" \
  -H "Upgrade: websocket" \
  -H "Sec-WebSocket-Key: test" \
  -H "Sec-WebSocket-Version: 13" \
  http://192.168.4.1:8765/

# Test PC port
curl -i -N \
  -H "Connection: Upgrade" \
  -H "Upgrade: websocket" \
  -H "Sec-WebSocket-Key: test" \
  -H "Sec-WebSocket-Version: 13" \
  http://192.168.4.1:8766/
```

## Security Considerations

### Access Control

```bash
# MAC address filtering (optional)
# Add to /etc/hostapd/hostapd.conf
macaddr_acl=1
accept_mac_file=/etc/hostapd/hostapd.accept

# Create MAC whitelist
echo "AA:BB:CC:DD:EE:01" >> /etc/hostapd/hostapd.accept
echo "AA:BB:CC:DD:EE:02" >> /etc/hostapd/hostapd.accept
```

### Network Isolation

```bash
# Prevent bot-to-bot direct communication
iptables -I FORWARD -i wlan0 -o wlan0 -j DROP

# Allow only bridge communication
iptables -I FORWARD -s 192.168.4.1 -j ACCEPT
iptables -I FORWARD -d 192.168.4.1 -j ACCEPT
```

## Backup and Recovery

### Configuration Backup

```bash
#!/bin/bash
# backup_network_config.sh

BACKUP_DIR="/home/pi/micro_bot/backups/network_$(date +%Y%m%d)"
mkdir -p "$BACKUP_DIR"

cp /etc/hostapd/hostapd.conf "$BACKUP_DIR/"
cp /etc/dnsmasq.conf "$BACKUP_DIR/"
cp /etc/dhcpcd.conf "$BACKUP_DIR/"
iptables-save > "$BACKUP_DIR/iptables.rules"

echo "Network configuration backed up to: $BACKUP_DIR"
```

### Configuration Restoration

```bash
#!/bin/bash
# restore_network_config.sh

BACKUP_DIR="$1"
if [ -z "$BACKUP_DIR" ]; then
    echo "Usage: $0 <backup_directory>"
    exit 1
fi

sudo cp "$BACKUP_DIR/hostapd.conf" /etc/hostapd/
sudo cp "$BACKUP_DIR/dnsmasq.conf" /etc/
sudo cp "$BACKUP_DIR/dhcpcd.conf" /etc/
sudo iptables-restore < "$BACKUP_DIR/iptables.rules"

sudo systemctl restart hostapd dnsmasq dhcpcd micro-bot.service
```

## Integration Testing

### End-to-End Network Test

```python
#!/usr/bin/env python3
# network_test.py

import asyncio
import websockets
import json

async def test_bot_connection():
    """Test bot WebSocket connection"""
    try:
        async with websockets.connect("ws://192.168.4.1:8765") as websocket:
            test_message = {"type": "test", "bot_id": "test_bot"}
            await websocket.send(json.dumps(test_message))
            response = await websocket.recv()
            print(f"Bot port test: SUCCESS - {response}")
    except Exception as e:
        print(f"Bot port test: FAILED - {e}")

async def test_pc_connection():
    """Test PC WebSocket connection"""
    try:
        async with websockets.connect("ws://192.168.4.1:8766") as websocket:
            test_message = {"type": "test", "client": "pc_test"}
            await websocket.send(json.dumps(test_message))
            response = await websocket.recv()
            print(f"PC port test: SUCCESS - {response}")
    except Exception as e:
        print(f"PC port test: FAILED - {e}")

async def main():
    await asyncio.gather(
        test_bot_connection(),
        test_pc_connection()
    )

if __name__ == "__main__":
    asyncio.run(main())
```

## Best Practices

### 1. Regular Monitoring

- Monitor DHCP lease usage
- Check WiFi signal quality
- Monitor WebSocket connection counts
- Review system resource usage

### 2. Capacity Planning

- Maximum 20 concurrent bot connections
- Reserve IP addresses for known bots
- Monitor bandwidth usage during swarm operations
- Plan for network expansion

### 3. Maintenance Schedule

- Weekly: Check service status and logs
- Monthly: Update system and clean logs
- Quarterly: Review and optimize network settings
- As needed: Backup configurations before changes

### 4. Performance Optimization

- Use static IP assignments for critical bots
- Optimize WiFi channel based on environment
- Monitor and adjust firewall rules
- Regular network performance testing
