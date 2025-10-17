# 🤖 MICRO BOT Pi Setup - Quick Start

## ✅ What You Have Now

You've successfully created a **deployment package** (`micro_bot_setup.zip`) containing everything needed to set up your Raspberry Pi as the **MICRO BOT communication bridge**.

## 🚀 Next Steps (5 Minutes)

### 1. Transfer Package to Pi

- **Method 1**: Use **WinSCP** or **FileZilla** to copy `micro_bot_setup.zip` to `/home/pi/`
- **Method 2**: Copy to USB drive, then to Pi
- **Method 3**: Use network file sharing

### 2. SSH to Your Pi

```bash
ssh pi@<your-pi-ip-address>
```

### 3. Extract and Setup

```bash
# Extract the package
unzip micro_bot_setup.zip -d jumbo_setup

# Navigate to setup directory
cd jumbo_setup

# Make scripts executable
chmod +x *.sh scripts/*.sh

# Run the installation
./setup_micro_bot.sh
```

### 4. Reboot Pi

```bash
sudo reboot
```

### 5. Verify Setup

```bash
# After reboot, check status
cd /home/pi/micro_bot
./check_status.sh
```

## 🌐 Your MICRO BOT Network

Once setup is complete, your Pi will create:

- **WiFi Network**: `JumboBot_Network`
- **WiFi Password**: `JumboBot2025`
- **Bridge IP**: `192.168.4.1`
- **Bot WebSocket**: `ws://192.168.4.1:8765` (for ESP32 bots)
- **PC WebSocket**: `ws://192.168.4.1:8766` (for your PC)

## 🤖 Bot Integration

Your **WHEELIE** and **SPEEDIE** bots can now connect to the `JumboBot_Network` WiFi and communicate through the bridge at `192.168.4.1:8765`.

Your **PC MCU** connects to `192.168.4.1:8766` to coordinate the swarm.

## 🛠️ Troubleshooting

**Bridge not working?**

```bash
sudo systemctl status micro-bot.service
sudo journalctl -u micro-bot.service -f
```

**WiFi issues?**

```bash
sudo systemctl restart hostapd dnsmasq
iwconfig wlan0
```

**Need help?** Check the documentation in the `docs/` folder on your Pi.

---

🎯 **Ready to deploy your swarm!** Your MICRO BOT bridge will handle all communication between your ESP32 bots and PC coordination system.
