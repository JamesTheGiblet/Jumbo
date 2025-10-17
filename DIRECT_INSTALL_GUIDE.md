# 🔧 BYPASS SSH - Direct Pi Installation

## If SSH keeps failing, install MICRO BOT directly on the Pi

### 1. Transfer via USB

1. **Copy `micro_bot_setup.zip`** to USB drive
2. **Insert USB into Pi**
3. **Connect keyboard/monitor to Pi**  
4. **Log in directly**: pi/raspberry

### 2. Install MICRO BOT Directly

```bash
# Find and mount USB
lsblk
sudo mkdir /mnt/usb
sudo mount /dev/sda1 /mnt/usb  # Adjust device name as needed

# Copy setup file
cp /mnt/usb/micro_bot_setup.zip ~/

# Unmount USB
sudo umount /mnt/usb

# Extract and install
unzip micro_bot_setup.zip -d jumbo_setup
cd jumbo_setup
chmod +x *.sh scripts/*.sh

# Run installation
./setup_micro_bot.sh

# Enable SSH for future use
sudo sed -i 's/PasswordAuthentication no/PasswordAuthentication yes/' /etc/ssh/sshd_config
sudo systemctl restart ssh

# Reboot when setup completes
sudo reboot
```

### 3. Verify Installation

After reboot, check status:

```bash
cd /home/pi/micro_bot
./check_status.sh
```

You should see:

- ✅ Bridge Service: Running
- ✅ WiFi AP: Running (JumboBot_Network visible)
- ✅ DHCP Server: Running
- ✅ WebSocket ports listening

### 4. SSH Should Work Now

After setup:

```bash
ssh pi@192.168.1.207
# Password should work now
```

## Result

Your Pi becomes MICRO BOT bridge with:

- WiFi: JumboBot_Network (password: JumboBot2025)  
- Bridge IP: 192.168.4.1
- Bot port: 8765, PC port: 8766
