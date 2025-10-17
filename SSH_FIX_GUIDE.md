# 🔧 Quick Pi Re-Setup for SSH Access

## Problem: SSH Password Authentication Disabled

Your Pi is configured for SSH key authentication only. Here's the fastest fix:

## Solution: Re-flash with Proper SSH Configuration

### 1. Re-flash Your Pi (5 minutes)

1. **Remove microSD card** from Pi
2. **Insert into PC** card reader
3. **Open Raspberry Pi Imager**
4. **Select "Raspberry Pi OS Lite (64-bit)"**
5. **Click gear icon** for advanced options
6. **Configure properly**:
   - ✅ **Enable SSH**: Check this box
   - ✅ **Use password authentication**: Select this option
   - **Username**: `pi`
   - **Password**: `raspberry` (or your choice)
   - ✅ **Configure WiFi** (optional): Your home network
   - ✅ **Set locale settings**: Your timezone/country
7. **Flash the card**
8. **Insert back into Pi** and power on

### 2. Wait and Connect

1. **Wait 2-3 minutes** for Pi to boot
2. **Find Pi IP** (should still be 192.168.1.207)
3. **SSH with password**:

   ```bash
   ssh pi@192.168.1.207
   # Enter password when prompted
   ```

### 3. Deploy MICRO BOT

Once SSH works:

```bash
# Transfer and extract your package
scp micro_bot_setup.zip pi@192.168.1.207:~/
ssh pi@192.168.1.207
unzip micro_bot_setup.zip -d jumbo_setup
cd jumbo_setup
chmod +x *.sh scripts/*.sh
./setup_micro_bot.sh
```

## Alternative: Manual File Transfer

If you prefer not to re-flash:

### 1. Use USB Drive Method

1. **Copy `micro_bot_setup.zip`** to USB drive
2. **Insert USB into Pi**
3. **Connect keyboard/monitor to Pi**
4. **Log in directly**: username `pi`, password `raspberry`
5. **Mount and copy**:

   ```bash
   # Find USB device
   lsblk
   
   # Mount USB (replace sda1 with your device)
   sudo mkdir /mnt/usb
   sudo mount /dev/sda1 /mnt/usb
   
   # Copy file
   cp /mnt/usb/micro_bot_setup.zip ~/
   
   # Unmount
   sudo umount /mnt/usb
   ```

6. **Extract and setup**:

   ```bash
   unzip micro_bot_setup.zip -d jumbo_setup
   cd jumbo_setup
   chmod +x *.sh scripts/*.sh
   ./setup_micro_bot.sh
   ```

### 2. Enable SSH for Future Use

While logged in directly:

```bash
sudo nano /etc/ssh/sshd_config
# Change: PasswordAuthentication yes
sudo systemctl restart ssh
```

## Recommendation

**Option 1 (Re-flash)** is fastest and ensures clean setup. The Pi Imager method takes only 5 minutes and gives you a properly configured system.

Once SSH works, you can proceed with the MICRO BOT installation! 🤖
