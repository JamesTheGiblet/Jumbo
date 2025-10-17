# ✅ Complete MICRO BOT Setup Checklist

## 🎯 Overview

Transform your Raspberry Pi into a **MICRO BOT communication bridge** for your ESP32 swarm.

## 📋 Pre-Setup Checklist

### Hardware Required

- [ ] **Raspberry Pi 3 Model B v1.2** (or newer)
- [ ] **32GB microSD card** (Class 10+)
- [ ] **Power supply** (5V 2.5A minimum)
- [ ] **Ethernet cable** (for initial setup)
- [ ] **Computer** with microSD card reader

### Software Required

- [ ] **Raspberry Pi Imager** downloaded
- [ ] **SSH client** (Windows: built-in, or PuTTY)
- [ ] **SCP client** (WinSCP, FileZilla, or command line)

## 🛠️ Step-by-Step Setup

### Phase 1: Prepare Raspberry Pi

- [ ] **1.1** Download and install Raspberry Pi Imager
- [ ] **1.2** Flash **Raspberry Pi OS Lite (64-bit)** to 32GB microSD card
- [ ] **1.3** Configure advanced options in imager:
  - [ ] Enable SSH with password authentication
  - [ ] Set username: `pi`, password: `raspberry`
  - [ ] Configure WiFi (optional)
  - [ ] Set timezone and locale
- [ ] **1.4** Insert microSD card into Pi and power on
- [ ] **1.5** Find Pi IP address via router admin (e.g., 192.168.1.207)
- [ ] **1.6** SSH to Pi: `ssh pi@192.168.1.207`
- [ ] **1.7** Run initial configuration: `sudo raspi-config`
  - [ ] Change password (recommended)
  - [ ] Expand filesystem
  - [ ] Set timezone
- [ ] **1.8** Update system: `sudo apt update && sudo apt upgrade -y`
- [ ] **1.9** Install essentials: `sudo apt install -y git curl wget unzip`

### Phase 2: Deploy MICRO BOT Software

- [ ] **2.1** Create deployment package on PC: `.\create_pi_package.ps1`
- [ ] **2.2** Transfer `micro_bot_setup.zip` to Pi via:
  - [ ] WinSCP/FileZilla to `/home/pi/`
  - [ ] SCP: `scp micro_bot_setup.zip pi@192.168.1.207:~/`
  - [ ] USB drive (manual copy)
- [ ] **2.3** SSH to Pi: `ssh pi@192.168.1.207`
- [ ] **2.4** Extract package: `unzip micro_bot_setup.zip -d jumbo_setup`
- [ ] **2.5** Navigate: `cd jumbo_setup`
- [ ] **2.6** Set permissions: `chmod +x *.sh scripts/*.sh`
- [ ] **2.7** Run setup: `./setup_micro_bot.sh`
- [ ] **2.8** Wait for installation (5-10 minutes)
- [ ] **2.9** Reboot: `sudo reboot`

### Phase 3: Verify Installation

- [ ] **3.1** Wait 2-3 minutes after reboot
- [ ] **3.2** SSH back to Pi: `ssh pi@192.168.1.207`
- [ ] **3.3** Check status: `cd /home/pi/micro_bot && ./check_status.sh`
- [ ] **3.4** Verify services running:
  - [ ] ✅ Bridge Service: Running
  - [ ] ✅ WiFi AP: Running
  - [ ] ✅ DHCP Server: Running
  - [ ] ✅ Bot Port (8765): Listening
  - [ ] ✅ PC Port (8766): Listening
- [ ] **3.5** Verify WiFi network visible: `JumboBot_Network`

## 🌐 Network Configuration

### MICRO BOT Network Details

- **WiFi SSID**: `JumboBot_Network`
- **WiFi Password**: `JumboBot2025`
- **Bridge IP**: `192.168.4.1`
- **Bot WebSocket**: `ws://192.168.4.1:8765`
- **PC WebSocket**: `ws://192.168.4.1:8766`
- **DHCP Range**: `192.168.4.10-50`

## 🤖 Integration Testing

### Test Bot Connection

- [ ] **4.1** Connect device to `JumboBot_Network` WiFi
- [ ] **4.2** Verify IP assignment (192.168.4.x range)
- [ ] **4.3** Test WebSocket: `ws://192.168.4.1:8765`

### Test PC Connection

- [ ] **4.4** Connect PC to same network or via Ethernet to Pi
- [ ] **4.5** Test WebSocket: `ws://192.168.4.1:8766`

## 🔧 Troubleshooting Checklist

### If SSH fails

- [ ] Check Pi power and network connection
- [ ] Verify IP address in router admin
- [ ] Try: `ssh pi@ProjectJumbo.local`
- [ ] Check SSH service: `sudo systemctl status ssh`

### If MICRO BOT services fail

- [ ] Check logs: `sudo journalctl -u micro-bot.service -f`
- [ ] Restart services: `sudo systemctl restart micro-bot.service hostapd dnsmasq`
- [ ] Check network interface: `ip addr show wlan0`

### If WiFi AP doesn't appear

- [ ] Check hostapd: `sudo systemctl status hostapd`
- [ ] Restart WiFi: `sudo systemctl restart hostapd dnsmasq`
- [ ] Check interface: `iwconfig wlan0`

## 🎯 Success Criteria

**Your MICRO BOT is ready when:**

- [ ] Pi boots and SSH is accessible
- [ ] All MICRO BOT services are running
- [ ] `JumboBot_Network` WiFi is visible and connectable
- [ ] WebSocket ports 8765 and 8766 are listening
- [ ] Bot devices can connect and get IP addresses
- [ ] PC can connect and communicate via WebSocket

## 📊 Performance Verification

- [ ] **CPU Usage**: < 25% under normal load
- [ ] **Memory Usage**: < 60% of available RAM  
- [ ] **Temperature**: < 70°C under load
- [ ] **WiFi Signal**: Strong signal strength for bot range
- [ ] **Network Latency**: < 10ms for local communication

## 🚀 Ready for Swarm Operations

Once all checkboxes are complete:
✅ **WHEELIE/SPEEDIE bots** can connect to `JumboBot_Network`  
✅ **PC MCU** can coordinate swarm via `ws://192.168.4.1:8766`  
✅ **Bridge** routes all communication between bots and PC  
✅ **Monitoring** and auto-recovery systems active  

**Your MICRO BOT communication bridge is operational!** 🤖
