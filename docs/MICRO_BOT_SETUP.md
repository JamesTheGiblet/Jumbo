# 🤖 MICRO BOT Setup Guide

## Raspberry Pi 3 Model B v1.2 Bridge Server

### Overview

The MICRO BOT serves as a WiFi communication bridge between ESP32 bots (WHEELIE/SPEEDIE) and the PC MCU Control Center. It runs on a Raspberry Pi and provides WebSocket servers for both bot and PC connections.

### Hardware Requirements

- **Raspberry Pi 3 Model B v1.2**
- **SD Card**: 32GB+ (Class 10 recommended)
- **Power Supply**: 5V 2.5A micro USB
- **Network**: WiFi or Ethernet connection

### Software Requirements

- **OS**: Raspberry Pi OS (Latest)
- **Python**: 3.9+ (included with Pi OS)
- **Packages**: websockets, asyncio (installed automatically)

---

## 🚀 Quick Setup

### Step 1: Prepare Raspberry Pi

1. **Flash Raspberry Pi OS** to SD card using Raspberry Pi Imager
2. **Enable SSH** in Pi configuration before first boot
3. **Boot Pi** and connect to your network
4. **Find Pi IP address**: `nmap -sn 192.168.1.0/24` or check router

### Step 2: Install MICRO BOT

**Easy Method (Windows):**

```bash
# Run the deployment script from project root
deploy_microbot.bat
# or
powershell deploy_microbot.ps1
```

**Manual Method:**

```bash
# SSH into your Pi
ssh pi@[PI_IP_ADDRESS]

# Create micro_bot directory
mkdir -p /home/pi/micro_bot
cd /home/pi/micro_bot

# Copy bridge server from your PC
scp src/MICRO_BOT/real_bridge.py pi@[PI_IP]:/home/pi/micro_bot/
```

### Step 3: Run MICRO BOT Bridge

```bash
# Start the bridge server
cd /home/pi/micro_bot
python3 real_bridge.py

# Or run in background
nohup python3 real_bridge.py > bridge.log 2>&1 &
```

---

## 🔧 Configuration

### Network Ports

- **Port 8765**: ESP32 Bot connections (WHEELIE/SPEEDIE)
- **Port 8766**: PC MCU connections

### Bridge Server Features

- **Real-time bot detection**: Automatically discovers connecting ESP32 bots
- **Message routing**: Routes messages between bots and PC
- **Status monitoring**: Tracks bot online/offline status
- **Statistics**: Logs message counts and system uptime

### ESP32 Bot Connection Format

Your ESP32 bots should connect to: `ws://[PI_IP]:8765`

**Message Format** (JSON):

```json
{
  "bot_id": "WHEELIE_001",
  "generation": 1,
  "fitness_score": 85.5,
  "emotional_state": {
    "curiosity": 0.8,
    "confidence": 0.7
  },
  "sensor_data": {
    "temperature": 23.5,
    "battery_voltage": 3.7
  }
}
```

---

## 🛠️ Troubleshooting

### Common Issues

**Bridge won't start - Port in use**

```bash
# Kill existing processes
sudo pkill -f real_bridge.py
# Wait 2 seconds, then restart
python3 real_bridge.py
```

**Pi not accessible**

```bash
# Check Pi is on network
ping [PI_IP]
# Check SSH service
ssh pi@[PI_IP] "echo 'Pi accessible'"
```

**Connection drops immediately**

- Ensure no simulated bot data conflicts
- Check websockets library version compatibility
- Verify no firewall blocking ports 8765/8766

### Log Monitoring

```bash
# View real-time bridge logs
tail -f /home/pi/micro_bot/bridge.log

# Check bridge process
ps aux | grep real_bridge
```

---

## 🔄 Service Setup (Optional)

To run MICRO BOT as a system service:

```bash
# Create service file
sudo nano /etc/systemd/system/microbot.service
```

**Service content:**

```ini
[Unit]
Description=MICRO BOT Bridge Server
After=network.target

[Service]
Type=simple
User=pi
WorkingDirectory=/home/pi/micro_bot
ExecStart=/usr/bin/python3 real_bridge.py
Restart=always
RestartSec=5

[Install]
WantedBy=multi-user.target
```

**Enable service:**

```bash
sudo systemctl enable microbot.service
sudo systemctl start microbot.service
sudo systemctl status microbot.service
```

---

## ✅ Verification

Bridge is working correctly when:

1. **Logs show**: "🚀 MICRO BOT Bridge Server online!"
2. **Ports listening**: `netstat -tlnp | grep -E '8765|8766'`
3. **PC can connect**: Connection stays stable from PC MCU
4. **Ready for bots**: "Ready for real WHEELIE/SPEEDIE bots" message

---

## 📡 Network Architecture

```
ESP32 Bots ─┐
            ├─ WiFi ─ MICRO BOT (Pi) ─ Network ─ PC MCU
ESP32 Bots ─┘         Port 8765        Port 8766
```

The MICRO BOT Pi serves as the central communication hub for the entire Project Jumbo swarm ecosystem.
