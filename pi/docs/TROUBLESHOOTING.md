# 🐛 MICRO BOT Troubleshooting Guide

## Common Issues and Solutions

### 🔧 Bridge Service Issues

#### Bridge Service Won't Start

```bash
# Check service status
sudo systemctl status micro-bot.service

# View detailed logs
sudo journalctl -u micro-bot.service --no-pager

# Common fixes:
sudo systemctl daemon-reload
sudo systemctl restart micro-bot.service
```

#### WebSocket Ports Not Listening

```bash
# Check if ports are in use
netstat -ln | grep ":876"
sudo lsof -i :8765
sudo lsof -i :8766

# Kill conflicting processes
sudo pkill -f "python.*bridge_server.py"

# Restart bridge
sudo systemctl restart micro-bot.service
```

#### Python Environment Issues

```bash
# Recreate virtual environment
cd /home/pi/micro_bot
rm -rf venv
python3 -m venv venv
source venv/bin/activate
pip install websockets asyncio aiofiles python-dateutil psutil
```

### 📡 WiFi Access Point Issues

#### WiFi AP Not Starting

```bash
# Check hostapd status
sudo systemctl status hostapd

# Test configuration
sudo hostapd -d /etc/hostapd/hostapd.conf

# Common fixes:
sudo systemctl stop wpa_supplicant
sudo systemctl restart hostapd
```

#### Can't See JumboBot_Network

```bash
# Check WiFi interface
sudo iwconfig wlan0
ip addr show wlan0

# Restart WiFi services
sudo systemctl restart hostapd dnsmasq

# Check for interference
sudo iwlist wlan0 scan | grep -E "ESSID|Channel"
```

#### WiFi Interface Down

```bash
# Bring interface up
sudo ip link set wlan0 up

# Check interface configuration
cat /etc/dhcpcd.conf | grep -A 3 "interface wlan0"

# Reset network configuration
sudo systemctl restart dhcpcd
```

### 🌐 Network Configuration Issues

#### Bridge IP Not Assigned

```bash
# Manual IP assignment
sudo ip addr add 192.168.4.1/24 dev wlan0

# Check dhcpcd configuration
sudo nano /etc/dhcpcd.conf

# Restart network service
sudo systemctl restart dhcpcd
```

#### DHCP Not Working

```bash
# Check dnsmasq status
sudo systemctl status dnsmasq

# Test dnsmasq configuration
sudo dnsmasq --test

# View DHCP leases
sudo cat /var/lib/dhcp/dhcpcd.leases

# Restart DHCP service
sudo systemctl restart dnsmasq
```

#### No Internet Access for Bots

```bash
# Check IP forwarding
cat /proc/sys/net/ipv4/ip_forward

# Enable IP forwarding
echo 1 | sudo tee /proc/sys/net/ipv4/ip_forward

# Check iptables rules
sudo iptables -L -n -t nat

# Restore iptables rules
sudo iptables-restore < /etc/iptables.ipv4.nat
```

### 🤖 Bot Connection Issues

#### Bots Can't Connect to WiFi

```bash
# Check WiFi signal strength
iwconfig wlan0

# Scan for the network from bot's perspective
iwlist wlan0 scan | grep -A 5 "JumboBot_Network"

# Check DHCP range availability
sudo arp-scan --local --interface=wlan0
```

#### WebSocket Connection Failures

```bash
# Test WebSocket ports locally
curl -i -N -H "Connection: Upgrade" -H "Upgrade: websocket" -H "Sec-WebSocket-Key: test" -H "Sec-WebSocket-Version: 13" http://192.168.4.1:8765/

# Check firewall rules
sudo iptables -L -n | grep 876

# Monitor connection attempts
sudo journalctl -u micro-bot.service -f
```

#### Bot Authentication Issues

```bash
# Verify WiFi password
grep "wpa_passphrase" /etc/hostapd/hostapd.conf

# Check WPA settings
grep -A 5 -B 5 "wpa=" /etc/hostapd/hostapd.conf

# Monitor hostapd for auth attempts
sudo journalctl -u hostapd.service -f
```

### 💻 System Performance Issues

#### High CPU Usage

```bash
# Check process usage
top -bn1 | head -20

# Check bridge server performance
ps aux | grep python
sudo strace -p $(pgrep -f bridge_server.py)

# Restart services if needed
sudo systemctl restart micro-bot.service
```

#### Memory Issues

```bash
# Check memory usage
free -h
cat /proc/meminfo

# Clear cache if needed
sudo sh -c 'echo 3 > /proc/sys/vm/drop_caches'

# Check for memory leaks
sudo journalctl | grep -i "killed process"
```

#### High Temperature

```bash
# Check CPU temperature
vcgencmd measure_temp

# Check for throttling
vcgencmd get_throttled

# Monitor system load
iostat 1 5
```

## 📊 Diagnostic Commands

### System Status Check

```bash
# Complete system overview
./check_status.sh

# Service status
sudo systemctl status micro-bot.service hostapd.service dnsmasq.service

# Network interfaces
ip addr show
iwconfig
```

### Network Diagnostics

```bash
# Connected devices
sudo arp-scan --local --interface=wlan0

# Network traffic
sudo iftop -i wlan0
sudo tcpdump -i wlan0 -n

# DNS resolution
nslookup google.com 192.168.4.1
```

### Log Analysis

```bash
# Bridge logs
sudo journalctl -u micro-bot.service --since "1 hour ago"

# System logs
sudo journalctl --since "1 hour ago" | grep -i error

# Network logs
sudo journalctl -u hostapd.service -u dnsmasq.service
```

## 🔧 Emergency Recovery

### Complete Service Reset

```bash
#!/bin/bash
# Emergency recovery script

sudo systemctl stop micro-bot.service hostapd.service dnsmasq.service
sudo systemctl daemon-reload
sudo systemctl enable micro-bot.service hostapd.service dnsmasq.service
sudo systemctl restart hostapd dnsmasq
sleep 5
sudo systemctl restart micro-bot.service
```

### Network Interface Reset

```bash
#!/bin/bash
# Reset network interface

sudo ip link set wlan0 down
sudo ip addr flush dev wlan0
sudo ip link set wlan0 up
sudo systemctl restart dhcpcd
sudo systemctl restart hostapd dnsmasq
```

### Factory Reset

```bash
#!/bin/bash
# Factory reset (use with caution)

# Stop all services
sudo systemctl stop micro-bot.service hostapd.service dnsmasq.service

# Reset configurations
sudo cp /etc/hostapd/hostapd.conf.backup /etc/hostapd/hostapd.conf 2>/dev/null
sudo cp /etc/dnsmasq.conf.backup /etc/dnsmasq.conf 2>/dev/null

# Clear DHCP leases
sudo rm -f /var/lib/dhcp/dhcpcd.leases

# Restart everything
sudo reboot
```

## 📞 Getting Help

### Log Collection for Support

```bash
#!/bin/bash
# Collect diagnostic information

mkdir -p ~/micro_bot_diagnostics
cd ~/micro_bot_diagnostics

# System information
uname -a > system_info.txt
lsb_release -a >> system_info.txt
free -h >> system_info.txt
df -h >> system_info.txt

# Service status
sudo systemctl status micro-bot.service > service_status.txt
sudo systemctl status hostapd.service >> service_status.txt
sudo systemctl status dnsmasq.service >> service_status.txt

# Network configuration
ip addr show > network_config.txt
iwconfig >> network_config.txt
cat /etc/hostapd/hostapd.conf >> network_config.txt
cat /etc/dnsmasq.conf >> network_config.txt

# Recent logs
sudo journalctl -u micro-bot.service --since "24 hours ago" > bridge_logs.txt
sudo journalctl -u hostapd.service --since "24 hours ago" > hostapd_logs.txt
sudo journalctl -u dnsmasq.service --since "24 hours ago" > dnsmasq_logs.txt

# Create archive
tar -czf micro_bot_diagnostics_$(date +%Y%m%d_%H%M%S).tar.gz *.txt

echo "Diagnostic archive created: micro_bot_diagnostics_$(date +%Y%m%d_%H%M%S).tar.gz"
```

### Contact Information

- **Project Repository**: Check the main project documentation
- **Issue Tracker**: Report bugs and issues in the project repository
- **Documentation**: Refer to the main project README and architecture docs
