# 🍓 Raspberry Pi Initial Setup Guide for MICRO BOT

## What You'll Need

- **Raspberry Pi 3 Model B v1.2** (or newer)
- **32GB microSD card** (Class 10 or better)
- **microSD card reader**
- **Power supply** (5V 2.5A minimum)
- **Ethernet cable** (for initial setup)
- **Computer** (Windows/Mac/Linux)

## Step 1: Download Raspberry Pi Imager

### Windows/Mac/Linux

1. Go to [https://www.raspberrypi.org/software/](https://www.raspberrypi.org/software/)
2. Download **Raspberry Pi Imager**
3. Install the application

## Step 2: Flash Raspberry Pi OS

### Recommended OS Selection

1. **Launch Raspberry Pi Imager**
2. Click **"CHOOSE OS"**
3. Select **"Raspberry Pi OS Lite (64-bit)"**
   - *This is the recommended lightweight version*
   - *No desktop GUI needed for bridge operation*
4. Click **"CHOOSE STORAGE"**
5. Select your **32GB microSD card**

### Configure Advanced Options

1. Click the **⚙️ gear icon** (Advanced options)
2. **Enable SSH**: ✅ Check "Enable SSH"
   - Select "Use password authentication"
   - Username: `pi`
   - Password: `raspberry` (change later for security)
3. **Configure WiFi** (optional for initial setup):
   - SSID: Your home WiFi network
   - Password: Your WiFi password
   - Country: US (or your country)
4. **Configure locale settings**:
   - Timezone: Your timezone
   - Keyboard layout: US (or your layout)
5. Click **"SAVE"**

### Flash the OS

1. Click **"WRITE"**
2. Wait for the process to complete (5-10 minutes)
3. **Safely eject** the microSD card

## Step 3: Initial Pi Boot

### Physical Setup

1. Insert the **microSD card** into your Pi
2. Connect **Ethernet cable** to Pi and router
3. Connect **power supply** to Pi
4. Wait **2-3 minutes** for first boot

### Find Pi IP Address

Check your router admin panel:

- Router IP: Usually `192.168.1.1` or `192.168.0.1`
- Look for device named "raspberrypi" or "ProjectJumbo"
- Note the IP address (e.g., `192.168.1.207`)

## Step 4: First SSH Connection

### From Windows PowerShell

```powershell
ssh pi@192.168.1.207
# Default password: raspberry
```

### Initial Configuration

```bash
# Run initial configuration
sudo raspi-config

# Navigate through these options:
# 1. Change User Password (recommended)
# 5. Localisation Options > I2 Change Timezone
# 6. Advanced Options > A1 Expand Filesystem
# Finish and reboot
```

## Step 5: Basic System Setup

### Update System

```bash
# Update package lists
sudo apt update

# Upgrade all packages
sudo apt upgrade -y

# Install essential tools
sudo apt install -y git curl wget unzip
```

### Set Hostname (Optional)

```bash
# Change hostname to ProjectJumbo
sudo hostnamectl set-hostname ProjectJumbo
sudo reboot
```

## Step 6: Verify Ready for MICRO BOT

### Test SSH Access

```bash
# SSH should work with new hostname
ssh pi@ProjectJumbo.local
# or
ssh pi@192.168.1.207
```

### Check System Status

```bash
# Check system info
uname -a
free -h
df -h

# Check network
ip addr show
iwconfig
```

## Step 7: Deploy MICRO BOT Software

Now your Pi is ready for the MICRO BOT deployment:

### Transfer Setup Package

1. Copy `micro_bot_setup.zip` to your Pi via:
   - **WinSCP/FileZilla**: Transfer to `/home/pi/`
   - **SCP command**: `scp micro_bot_setup.zip pi@192.168.1.207:~/`
   - **USB drive**: Copy manually

### Install MICRO BOT

```bash
# SSH to Pi
ssh pi@192.168.1.207

# Extract package
unzip micro_bot_setup.zip -d jumbo_setup

# Navigate and setup
cd jumbo_setup
chmod +x *.sh scripts/*.sh

# Run installation
./setup_micro_bot.sh

# Reboot when prompted
sudo reboot
```

## Step 8: Verify MICRO BOT Installation

### After Reboot

```bash
# SSH back to Pi
ssh pi@192.168.1.207

# Check MICRO BOT status
cd /home/pi/micro_bot
./check_status.sh
```

### Expected Results

- ✅ Bridge Service: Running
- ✅ WiFi AP: Running (JumboBot_Network)
- ✅ DHCP Server: Running
- ✅ Bot Port (8765): Listening
- ✅ PC Port (8766): Listening

## 🔧 Troubleshooting Initial Setup

### SSH Connection Issues

```bash
# Check if SSH is enabled
sudo systemctl status ssh

# Enable SSH if needed
sudo systemctl enable ssh
sudo systemctl start ssh
```

### WiFi Issues During Setup

```bash
# Check WiFi status
iwconfig
sudo iwlist wlan0 scan | grep ESSID

# Reconfigure WiFi if needed
sudo raspi-config
# 1. System Options > S1 Wireless LAN
```

### SD Card Issues

- Use **Class 10** or better microSD card
- Ensure **32GB minimum** capacity
- Try different card if boot issues persist

## 📋 Quick Reference

### Default Credentials

- **Username**: `pi`
- **Default Password**: `raspberry`
- **SSH**: Enabled by default

### Network After MICRO BOT Setup

- **Pi WiFi AP**: `JumboBot_Network`
- **AP Password**: `JumboBot2025`
- **Bridge IP**: `192.168.4.1`
- **Bot Port**: `8765`
- **PC Port**: `8766`

## 🎯 Next Steps

Once your Pi is set up and MICRO BOT is installed:

1. **Connect ESP32 bots** to `JumboBot_Network`
2. **Configure PC MCU** to connect to `ws://192.168.4.1:8766`
3. **Test swarm communication** through the bridge

Your **MICRO BOT** is now ready to serve as the communication bridge for your WHEELIE/SPEEDIE swarm! 🤖
