# 🤖 MICRO BOT (Pi) Quick Setup Guide

## What is MICRO BOT?

The **MICRO BOT** is a **Raspberry Pi 3 Model B v1.2** running as a **WiFi communication bridge** between your ESP32 swarm bots (WHEELIE/SPEEDIE) and PC. It creates a dedicated WiFi network for the bots and routes messages via WebSocket connections.

```txt
ESP32 Bots ↔ WiFi ↔ MICRO BOT (Pi) ↔ Ethernet/WiFi ↔ PC
```

## � Prerequisites

**Before starting, ensure you have:**

1. **Raspberry Pi OS** installed on your Pi (see `RASPBERRY_PI_SETUP_GUIDE.md`)
2. **SSH access** to your Pi enabled
3. **Network connection** to your Pi (Ethernet or WiFi)
4. **32GB microSD card** with fresh Raspberry Pi OS

## �🚀 Quick Setup (5 Steps)

### 0. Prepare Raspberry Pi (First Time Only)

If you haven't set up your Pi yet, follow the complete guide:
**📖 See: `RASPBERRY_PI_SETUP_GUIDE.md`**

This covers:

- Flashing Raspberry Pi OS to microSD card
- Initial SSH configuration  
- Basic system setup and updates
- Network configuration

### 1. Deploy Files to Pi

**Easy Method (Windows):**

```powershell
# Create deployment package
.\create_pi_package.ps1

# Transfer micro_bot_setup.zip to Pi via:
# - WinSCP/FileZilla
# - USB drive
# - Network share
```

**Advanced Method (if SSH configured):**

```powershell
# Direct deployment via SSH
.\deploy_to_pi_fixed.ps1
```

### 2. SSH to Pi and Run Setup

```bash
# Connect to Pi
ssh pi@<pi-ip-address>

# Navigate to setup
cd /home/pi/jumbo_setup

# Run installation
./setup_micro_bot.sh
```

### 3. Reboot Pi

```bash
sudo reboot
```

### 4. Verify Setup

```bash
# After reboot, check status
cd /home/pi/micro_bot
./check_status.sh
```

### 5. Connect Bots and PC

- **Bots**: Connect to WiFi `JumboBot_Network` (password: `JumboBot2025`)
- **PC**: Connect to same network or use Ethernet to Pi

## 📊 Network Configuration

| Component | IP Address | Port | Purpose |
|-----------|------------|------|---------|
| MICRO BOT | 192.168.4.1 | - | Bridge/Gateway |
| Bot WebSocket | 192.168.4.1 | 8765 | ESP32 ↔ Bridge |
| PC WebSocket | 192.168.4.1 | 8766 | PC ↔ Bridge |
| ESP32 Bots | 192.168.4.10-29 | - | DHCP Range |

## 🛠️ Management Commands

```bash
# Check system status
./check_status.sh

# Restart bridge service  
sudo systemctl restart micro-bot.service

# View live logs
sudo journalctl -u micro-bot.service -f

# Update system
./update_system.sh
```

## 🔧 WebSocket Connections

**ESP32 Bots connect to:**

```txt
ws://192.168.4.1:8765
```

**PC MCU connects to:**

```txt
ws://192.168.4.1:8766
```

## 🐛 Common Issues

**Bridge not starting?**

```bash
sudo systemctl status micro-bot.service
sudo journalctl -u micro-bot.service --no-pager
```

**WiFi not working?**

```bash
sudo systemctl restart hostapd dnsmasq
iwconfig wlan0
```

**Can't see JumboBot_Network?**

```bash
sudo iwlist wlan0 scan | grep "JumboBot"
sudo systemctl status hostapd
```

## 📁 File Locations

- **Bridge Server**: `/home/pi/micro_bot/bridge_server.py`
- **Configuration**: `/home/pi/micro_bot/config/`
- **Logs**: `/home/pi/micro_bot/logs/`
- **Status Check**: `/home/pi/micro_bot/check_status.sh`

## 🎯 Testing Connection

**Test from PC:**

```python
import asyncio
import websockets
import json

async def test():
    async with websockets.connect("ws://192.168.4.1:8766") as ws:
        await ws.send(json.dumps({"test": "pc_connection"}))
        response = await ws.recv()
        print(f"Bridge response: {response}")

asyncio.run(test())
```

## ✅ Success Indicators

When setup is complete, you should see:

- ✅ Bridge Service: Running
- ✅ WiFi AP: Running  
- ✅ DHCP Server: Running
- ✅ Bot Port (8765): Listening
- ✅ PC Port (8766): Listening
- ✅ WiFi Network: JumboBot_Network visible

## 🔗 Integration

The MICRO BOT bridge integrates with:

- **WHEELIE Bot** (Physical manipulation ESP32)
- **SPEEDIE Bot** (High-speed processing ESP32)  
- **PC MCU** (Central coordination program)
- **Swarm Ecosystem Manager** (Layer 3 intelligence)

## 📞 Need Help?

1. **Check Status**: Run `./check_status.sh` for complete diagnostic
2. **View Logs**: `sudo journalctl -u micro-bot.service -f`
3. **Restart Services**: `sudo systemctl restart micro-bot.service hostapd dnsmasq`
4. **Troubleshooting**: See `docs/TROUBLESHOOTING.md`

---

🤖 **MICRO BOT** - Your swarm's communication bridge is ready!
