# 🖥️ PC MCU - Project Jumbo Control Center

The PC Master Control Unit (MCU) is a comprehensive Python application for managing and monitoring the entire Project Jumbo robot ecosystem.

## Features

### 🔗 Connection Management

- WebSocket connection to MICRO BOT bridge
- Automatic reconnection handling
- Connection status monitoring
- Configurable IP and port settings

### 🤖 Bot Monitoring

- Real-time bot status tracking
- Performance metrics display
- Generation and fitness monitoring
- Emotional state visualization
- Sensor data inspection

### 🎯 Mission Control

- Mission planning and creation
- Exploration missions
- Patrol missions
- Collaborative multi-bot missions
- Mission progress tracking

### 📊 Analytics Dashboard

- Real-time performance charts
- Fitness score trends
- Generation progress tracking
- Message activity monitoring
- Bot status distribution

### 📝 Logging System

- Comprehensive message logging
- Exportable log files
- Real-time status updates
- Connection event tracking

## Installation

### Prerequisites

- Python 3.8 or higher
- MICRO BOT running on Raspberry Pi
- Network connectivity to MICRO BOT

### Quick Setup

1. Run the setup script:

   ```bash
   setup_pc_mcu.bat
   ```

2. This will:
   - Create a Python virtual environment
   - Install all required dependencies
   - Create a desktop shortcut for easy access

### Manual Installation

1. Create virtual environment:

   ```bash
   python -m venv pc_mcu_env
   pc_mcu_env\Scripts\activate
   ```

2. Install dependencies:

   ```bash
   pip install -r requirements.txt
   ```

3. Run the application:

   ```bash
   python pc_mcu_main.py
   ```

## Usage

### First Launch

1. Start the PC MCU application
2. Go to the "🔗 Connection" tab
3. Enter MICRO BOT IP address (default: 192.168.4.1)
4. Enter port (default: 8766)
5. Click "Connect"

### Monitoring Bots

1. Switch to "🤖 Monitoring" tab
2. View connected bots in the list
3. Click on a bot to see detailed information
4. Monitor real-time status updates

### Creating Missions

1. Go to "🎯 Missions" tab
2. Select mission type:
   - **Exploration**: Bot explores a defined area
   - **Patrol**: Bot follows waypoints in cycles
   - **Collaborative**: Multiple bots work together
3. Choose target bot from dropdown
4. Click "Create Mission"

### Analytics

1. Visit "📊 Analytics" tab for:
   - Fitness score trends over time
   - Generation progress comparison
   - Message activity levels
   - Bot online/offline status

### Logs

1. Check "📝 Logs" tab for:
   - Real-time message logs
   - Connection events
   - System status updates
2. Use "Save Logs" to export log files

## Mission Types

### Exploration Mission

- **Purpose**: Systematic area mapping and object detection
- **Parameters**:
  - Area bounds (coordinates)
  - Duration (minutes)
  - Scan resolution
  - Return to start option
- **Best for**: WHEELIE with precision sensors

### Patrol Mission

- **Purpose**: Regular monitoring of defined waypoints
- **Parameters**:
  - Waypoint coordinates
  - Number of cycles
  - Speed mode (adaptive/fixed)
  - Obstacle avoidance
- **Best for**: SPEEDIE with motion detection

### Collaborative Mission

- **Purpose**: Multi-bot coordinated operations
- **Parameters**:
  - Multiple bot selection
  - Coordination strategy
  - Communication protocols
- **Best for**: Combined WHEELIE + SPEEDIE operations

## Communication Protocol

### Bot → PC Messages

```json
{
  "type": "bot_message",
  "bot_id": "WHEELIE",
  "data": {
    "generation": 15,
    "fitness_score": 0.847,
    "emotional_state": {
      "frustration": 2,
      "confidence": 8
    },
    "sensor_data": {
      "distance": 25.4,
      "motion_detected": false
    }
  }
}
```

### PC → Bot Commands

```json
{
  "type": "mission",
  "target_bot": "SPEEDIE",
  "mission_data": {
    "type": "patrol",
    "parameters": {
      "waypoints": [[0,0], [5,0], [5,5], [0,5]],
      "cycles": 3
    }
  }
}
```

## Network Configuration

### Default Settings

- **MICRO BOT IP**: 192.168.4.1
- **PC MCU Port**: 8766
- **Connection Type**: WebSocket
- **Protocol**: JSON messages

### Troubleshooting Connection

1. Verify MICRO BOT is running:

   ```bash
   ping 192.168.4.1
   ```

2. Check MICRO BOT bridge service:

   ```bash
   # On Raspberry Pi
   sudo systemctl status micro-bot-bridge
   ```

3. Verify port accessibility:

   ```bash
   telnet 192.168.4.1 8766
   ```

## Performance Monitoring

### Key Metrics

- **Fitness Score**: Bot performance indicator (0.0-1.0)
- **Generation**: Evolution cycle number
- **Message Count**: Communication activity
- **Response Time**: Connection latency

### Optimization Tips

- Monitor fitness trends for learning progress
- Watch for message count spikes (may indicate issues)
- Check emotional states for bot behavior insights
- Use analytics to identify optimal mission parameters

## File Structure

```txt
PC_MCU/
├── pc_mcu_main.py          # Main application
├── requirements.txt         # Python dependencies
├── setup_pc_mcu.bat       # Windows setup script
├── PC_MCU_GUIDE.md         # This documentation
└── logs/                   # Generated log files
```

## Integration with Project Jumbo

### WHEELIE Integration

- Precision exploration missions
- Detailed sensor data analysis
- Learning behavior monitoring
- Evolutionary progress tracking

### SPEEDIE Integration

- High-speed patrol missions
- Motion detection coordination
- Performance optimization
- Real-time response monitoring

### MICRO BOT Integration

- Centralized communication hub
- Signal filtering and routing
- Multi-bot coordination
- Network management

## Future Enhancements

### Planned Features

- [ ] Voice control integration
- [ ] Advanced mission scripting
- [ ] Machine learning analytics
- [ ] Remote deployment tools
- [ ] 3D visualization
- [ ] Mobile app companion

### API Extensions

- [ ] REST API for external integration
- [ ] Plugin system for custom missions
- [ ] Database integration for history
- [ ] Cloud synchronization
- [ ] Multi-site deployment

## Support

### Getting Help

1. Check the logs tab for error messages
2. Verify network connectivity to MICRO BOT
3. Ensure all bots are powered and connected
4. Review MICRO BOT bridge status

### Common Issues

- **Connection timeout**: Check MICRO BOT IP address
- **No bot data**: Verify bots are connected to MICRO BOT
- **Mission not starting**: Ensure target bot is online
- **Charts not updating**: Check data refresh rate

### Contact

For technical support and questions about Project Jumbo, refer to the main project documentation.
