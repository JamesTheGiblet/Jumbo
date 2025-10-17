# 🔍 USB Detection Commands for Pi

## You're now logged into the Pi directly! Here's how to find your USB

### 1. Check if USB is detected

```bash
# List all block devices
lsblk

# Check USB devices
lsusb

# Watch for new devices (insert USB now)
sudo dmesg | tail -10
```

### 2. Insert USB drive and check again

```bash
# After inserting USB, run:
lsblk
# Look for new device like sda, sda1, sdb, sdb1, etc.
```

### 3. Mount the USB drive

```bash
# If USB shows as sda1 (most common):
sudo mount /dev/sda1 /mnt/usb

# If USB shows as sdb1:
sudo mount /dev/sdb1 /mnt/usb

# Check if mounted:
ls /mnt/usb
# You should see micro_bot_setup.zip
```

### 4. Copy and install MICRO BOT

```bash
# Copy the setup file
cp /mnt/usb/micro_bot_setup.zip ~/

# Unmount USB
sudo umount /mnt/usb

# Verify file copied
ls -la ~/micro_bot_setup.zip

# Extract and install
unzip micro_bot_setup.zip -d jumbo_setup
cd jumbo_setup
chmod +x *.sh scripts/*.sh

# Run the installation
./setup_micro_bot.sh
```

## Alternative: Skip USB and Use Network

If USB detection is problematic, you can download directly:

```bash
# Download from GitHub or use wget if you have internet
# Or skip USB and re-flash Pi with SSH enabled properly

# If you want to enable SSH right now:
sudo nano /etc/ssh/sshd_config
# Change: PasswordAuthentication yes
# Save with Ctrl+X, Y, Enter

sudo systemctl restart ssh
```

## What to do next

1. **Insert your USB drive** (with micro_bot_setup.zip on it)
2. **Run `lsblk`** to see the new device (probably sda1)
3. **Mount it**: `sudo mount /dev/sda1 /mnt/usb`
4. **Copy file**: `cp /mnt/usb/micro_bot_setup.zip ~/`
5. **Continue with installation**
