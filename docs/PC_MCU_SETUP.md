# 🖥️ PC MCU Setup Guide

## Project Jumbo Control Center

### Overview

The PC MCU (Micro Control Unit) is the central command and control application for the Project Jumbo swarm ecosystem. It provides a comprehensive GUI for monitoring, controlling, and analyzing the WHEELIE/SPEEDIE bot swarm through the MICRO BOT bridge.

### System Requirements

- **OS**: Windows 10/11, Linux, or macOS
- **Python**: 3.9+
- **RAM**: 4GB minimum, 8GB recommended
- **Network**: Connection to same network as MICRO BOT Pi

### Dependencies

- **tkinter**: GUI framework (usually included with Python)
- **websockets**: WebSocket client library
- **matplotlib**: Analytics charts and graphs
- **asyncio**: Asynchronous networking

---

## 🚀 Quick Setup

### Step 1: Install Python Dependencies

```bash
# Navigate to PC_MCU directory
cd src/PC_MCU

# Install required packages
pip install websockets matplotlib

# Or use requirements file
pip install -r requirements.txt
```

### Step 2: Configure Connection

Edit `pc_mcu_main.py` if needed to set your MICRO BOT Pi IP:

```python
# Connection settings (around line 141)
self.micro_bot_ip = "192.168.1.207"  # Your Pi's IP address
self.micro_bot_port = 8766
```

### Step 3: Launch PC MCU

```bash
# Start the application
python pc_mcu_main.py

# Or use the batch file (Windows)
setup_pc_mcu.bat
```

---

## 🎛️ GUI Interface

### Connection Tab

- **IP Address**: MICRO BOT Pi IP address
- **Port**: 8766 (PC client port)
- **Connect/Disconnect**: Establish bridge connection
- **Status Display**: Real-time connection messages

### Monitoring Tab

- **Bot List**: Live display of connected bots
- **Bot Status**: Online/offline indicators
- **Bot Details**: Generation, fitness, messages, last seen
- **Real-time Updates**: Automatic refresh every 10 seconds

### Missions Tab

- **Mission Types**: exploration, formation, patrol, gather, emergency
- **Create Mission**: Assign missions to selected bots
- **Mission List**: Active missions with progress tracking
- **Bot Assignment**: Manual or automatic bot selection

### Analytics Tab

- **Fitness Trends**: Historical performance graphs
- **Generation Progress**: Bot evolution tracking
- **Message Activity**: Communication statistics
- **Status Distribution**: Online/offline pie chart

### Logs Tab

- **System Messages**: Application and bridge communications
- **Export Logs**: Save logs to file for analysis
- **Clear Logs**: Reset log display

---

## 🔧 Configuration Options

### Connection Settings

```python
# Default MICRO BOT connection
micro_bot_ip = "192.168.1.207"    # Pi IP address
micro_bot_port = 8766              # PC client port

# Update intervals
periodic_update_interval = 10000   # Bot list updates (ms)
analytics_update_interval = 5000   # Chart refresh (ms)
```

### Display Settings

```python
# Bot performance history
performance_history_length = 100   # Data points to keep

# Message log size
message_log_max = 1000             # Maximum log entries

# GUI dimensions
window_size = "1200x800"           # Default window size
```

---

## 🤖 Bot Communication Protocol

### Expected Bot Message Format

The PC MCU expects bots to send JSON messages through the MICRO BOT bridge:

```json
{
  "bot_id": "WHEELIE_001",
  "generation": 1,
  "fitness_score": 85.5,
  "emotional_state": {
    "curiosity": 0.8,
    "confidence": 0.7,
    "cooperation": 0.9,
    "energy_level": 0.85
  },
  "sensor_data": {
    "temperature": 23.5,
    "humidity": 65.2,
    "battery_voltage": 3.7,
    "signal_strength": -45
  },
  "capabilities": ["movement", "sensing", "communication"]
}
```

### Command Messages to Bots

PC MCU can send commands to bots:

```json
{
  "type": "bot_command",
  "bot_id": "WHEELIE_001",
  "command": "start_mission",
  "parameters": {
    "mission_type": "exploration",
    "duration": 300
  }
}
```

---

## 🛠️ Troubleshooting

### Common Issues

**Application won't start - Import errors**

```bash
# Install missing dependencies
pip install --upgrade websockets matplotlib tkinter

# Check Python version
python --version  # Should be 3.9+
```

**Connection fails - Can't reach MICRO BOT**

1. Verify MICRO BOT Pi is running and accessible
2. Check IP address is correct in PC MCU settings
3. Ensure both devices on same network
4. Test connection: `ping [PI_IP]`

**GUI display issues**

```bash
# Update display libraries
pip install --upgrade matplotlib pillow

# Check tkinter availability
python -c "import tkinter; print('tkinter available')"
```

**Deprecation warnings**

- These are normal and don't affect functionality
- Warnings about event loops are cosmetic

### Debug Mode

Enable detailed logging by setting log level:

```python
logging.basicConfig(level=logging.DEBUG)
```

---

## 📊 Analytics and Monitoring

### Performance Metrics

- **Fitness Score Trends**: Track bot evolution over time  
- **Generation Progress**: Monitor genetic algorithm advancement
- **Message Activity**: Communication frequency analysis
- **System Uptime**: Bridge and connection reliability

### Data Export

- **Log Files**: Automatic timestamped log exports
- **Performance Data**: Bot history and statistics
- **Mission Reports**: Completed mission analytics

### Real-time Monitoring

- **Live Updates**: 10-second bot status refresh
- **Connection Health**: Bridge communication monitoring  
- **System Statistics**: Message counts and processing stats

---

## 🚀 Advanced Usage

### Mission Programming

Create custom mission types by extending the `MissionPlanner` class:

```python
def create_custom_mission(self, bot_ids, custom_params):
    mission_id = self.create_mission(
        mission_type="custom",
        bot_ids=bot_ids,
        parameters=custom_params
    )
    return mission_id
```

### Analytics Extensions  

Add custom charts by extending the analytics update method:

```python
def update_custom_analytics(self):
    # Add new subplot for custom metrics
    self.ax5 = self.fig.add_subplot(235)
    # Custom plotting logic here
```

---

## ✅ Verification Checklist

PC MCU is working correctly when:

- [x] **Application starts** without import errors
- [x] **Connection established** to MICRO BOT bridge  
- [x] **Status updates** received every few seconds
- [x] **Bot monitoring** shows empty list (ready for real bots)
- [x] **All tabs functional** and displaying correctly
- [x] **No connection drops** or stability issues

---

## 📡 System Integration

```
┌─────────────────┐    WebSocket     ┌──────────────────┐    WiFi/ESP-NOW    ┌─────────────────┐
│   PC MCU GUI    │ ◄──────────────► │ MICRO BOT Bridge │ ◄─────────────────► │  ESP32 Swarm    │
│  Control Center │  (Port 8766)     │   (Raspberry Pi) │   (Port 8765)      │ WHEELIE/SPEEDIE │
└─────────────────┘                  └──────────────────┘                    └─────────────────┘
```

The PC MCU serves as the human interface to the entire Project Jumbo autonomous swarm ecosystem.
