#!/usr/bin/env python3
"""
🤖 MICRO BOT: Communication Bridge Server
Raspberry Pi 3 Model B v1.2 Bridge System

Hardware Platform: Raspberry Pi 3 Model B v1.2
- BCM2837 quad-core ARM Cortex-A53 @ 1.2GHz
- 1GB LPDDR2 SDRAM
- 802.11n Wireless LAN
- Bluetooth 4.1 & BLE
- 40-pin GPIO header
- Ethernet port

MICRO BOT Functions:
- WiFi bridge between ESP32 bots (WHEELIE/SPEEDIE) and PC
- Signal routing and filtering
- Real-time communication hub
- Data logging and analysis
- Bot status monitoring
- Multi-bot coordination
"""

import asyncio
import websockets
import json
import time
import logging
import socket
import threading
from datetime import datetime
from typing import Dict, List, Optional, Any
from dataclasses import dataclass, asdict
from collections import defaultdict, deque
import signal
import sys

# Configure logging
logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s - %(name)s - %(levelname)s - %(message)s',
    handlers=[
        logging.FileHandler('/home/pi/micro_bot.log'),
        logging.StreamHandler()
    ]
)
logger = logging.getLogger('MicroBot')

@dataclass
class BotMessage:
    """Structure for bot communication messages"""
    bot_id: str
    message_type: str  # 'status', 'evolution', 'obstacle', 'strategy', 'signal'
    timestamp: float
    data: Dict[str, Any]
    filtered: bool = False
    priority: int = 1  # 1=low, 2=medium, 3=high

@dataclass
class BotStatus:
    """Bot status tracking"""
    bot_id: str
    last_seen: float
    ip_address: str
    generation: int
    fitness_score: float
    is_online: bool
    message_count: int
    error_count: int

class SignalFilter:
    """Advanced signal filtering and processing"""
    
    def __init__(self):
        self.message_history = defaultdict(lambda: deque(maxlen=100))
        self.filter_rules = {
            'rate_limit': 10,  # Max messages per second per bot
            'duplicate_window': 2.0,  # Seconds to check for duplicates
            'priority_threshold': 2,  # Minimum priority for immediate forwarding
        }
        
    def should_filter(self, message: BotMessage) -> bool:
        """Determine if message should be filtered"""
        bot_history = self.message_history[message.bot_id]
        current_time = time.time()
        
        # Rate limiting
        recent_messages = [msg for msg in bot_history 
                          if current_time - msg.timestamp < 1.0]
        if len(recent_messages) >= self.filter_rules['rate_limit']:
            logger.warning(f"Rate limiting {message.bot_id}")
            return True
            
        # Duplicate detection
        for past_msg in bot_history:
            if (current_time - past_msg.timestamp < self.filter_rules['duplicate_window'] and
                past_msg.message_type == message.message_type and
                past_msg.data == message.data):
                logger.debug(f"Duplicate message filtered from {message.bot_id}")
                return True
                
        # Priority filtering
        if message.priority < self.filter_rules['priority_threshold']:
            if len(recent_messages) > 5:  # Filter low priority if busy
                return True
                
        return False
        
    def process_message(self, message: BotMessage) -> BotMessage:
        """Process and potentially modify message"""
        # Add to history
        self.message_history[message.bot_id].append(message)
        
        # Apply filtering
        message.filtered = self.should_filter(message)
        
        # Add processing metadata
        message.data['processed_at'] = time.time()
        message.data['micro_bot_id'] = 'RPI3-BRIDGE-001'
        
        return message

class MicroBotServer:
    """Main Micro Bot bridge server"""
    
    def __init__(self, host='0.0.0.0', bot_port=8765, pc_port=8766):
        self.host = host
        self.bot_port = bot_port  # Port for ESP32 bots
        self.pc_port = pc_port    # Port for PC application
        
        self.bot_clients = {}     # Connected ESP32 bots
        self.pc_clients = set()   # Connected PC applications
        self.bot_status = {}      # Bot status tracking
        self.signal_filter = SignalFilter()
        
        self.running = False
        self.stats = {
            'messages_processed': 0,
            'messages_filtered': 0,
            'messages_forwarded': 0,
            'uptime_start': time.time()
        }
        
    async def handle_bot_connection(self, websocket, path):
        """Handle connections from ESP32 bots"""
        bot_ip = websocket.remote_address[0]
        bot_id = None
        
        try:
            logger.info(f"Bot connected from {bot_ip}")
            
            async for message in websocket:
                try:
                    data = json.loads(message)
                    
                    # Extract bot ID from first message
                    if bot_id is None:
                        bot_id = data.get('bot_id', f'Bot-{bot_ip}')
                        self.bot_clients[bot_id] = websocket
                        self.bot_status[bot_id] = BotStatus(
                            bot_id=bot_id,
                            last_seen=time.time(),
                            ip_address=bot_ip,
                            generation=0,
                            fitness_score=0.0,
                            is_online=True,
                            message_count=0,
                            error_count=0
                        )
                        logger.info(f"Bot {bot_id} registered from {bot_ip}")
                    
                    # Process message
                    bot_message = BotMessage(
                        bot_id=bot_id,
                        message_type=data.get('type', 'unknown'),
                        timestamp=time.time(),
                        data=data,
                        priority=data.get('priority', 1)
                    )
                    
                    # Apply filtering
                    processed_message = self.signal_filter.process_message(bot_message)
                    
                    # Update statistics
                    self.stats['messages_processed'] += 1
                    self.bot_status[bot_id].message_count += 1
                    self.bot_status[bot_id].last_seen = time.time()
                    
                    if processed_message.filtered:
                        self.stats['messages_filtered'] += 1
                        continue
                    
                    # Update bot status from message
                    if 'generation' in data:
                        self.bot_status[bot_id].generation = data['generation']
                    if 'fitness_score' in data:
                        self.bot_status[bot_id].fitness_score = data['fitness_score']
                    
                    # Forward to PC clients
                    await self.forward_to_pc(processed_message)
                    self.stats['messages_forwarded'] += 1
                    
                except json.JSONDecodeError:
                    logger.error(f"Invalid JSON from {bot_id or bot_ip}")
                    if bot_id:
                        self.bot_status[bot_id].error_count += 1
                except Exception as e:
                    logger.error(f"Error processing message from {bot_id or bot_ip}: {e}")
                    if bot_id:
                        self.bot_status[bot_id].error_count += 1
                        
        except websockets.exceptions.ConnectionClosed:
            logger.info(f"Bot {bot_id or bot_ip} disconnected")
        except Exception as e:
            logger.error(f"Bot connection error: {e}")
        finally:
            # Cleanup
            if bot_id:
                if bot_id in self.bot_clients:
                    del self.bot_clients[bot_id]
                if bot_id in self.bot_status:
                    self.bot_status[bot_id].is_online = False
                    
    async def handle_pc_connection(self, websocket, path):
        """Handle connections from PC applications"""
        pc_ip = websocket.remote_address[0]
        
        try:
            logger.info(f"PC client connected from {pc_ip}")
            self.pc_clients.add(websocket)
            
            # Send initial status
            await self.send_status_update(websocket)
            
            async for message in websocket:
                try:
                    data = json.loads(message)
                    
                    # Handle PC commands
                    if data.get('type') == 'command':
                        await self.handle_pc_command(data, websocket)
                    elif data.get('type') == 'bot_command':
                        await self.forward_to_bot(data)
                        
                except json.JSONDecodeError:
                    logger.error(f"Invalid JSON from PC {pc_ip}")
                except Exception as e:
                    logger.error(f"Error processing PC message: {e}")
                    
        except websockets.exceptions.ConnectionClosed:
            logger.info(f"PC client {pc_ip} disconnected")
        except Exception as e:
            logger.error(f"PC connection error: {e}")
        finally:
            self.pc_clients.discard(websocket)
            
    async def handle_pc_command(self, data: Dict, websocket):
        """Handle commands from PC"""
        command = data.get('command')
        
        if command == 'get_status':
            await self.send_status_update(websocket)
        elif command == 'get_bots':
            await self.send_bot_list(websocket)
        elif command == 'set_filter_rules':
            self.update_filter_rules(data.get('rules', {}))
            await websocket.send(json.dumps({
                'type': 'response',
                'status': 'success',
                'message': 'Filter rules updated'
            }))
        elif command == 'restart_bot':
            bot_id = data.get('bot_id')
            await self.send_bot_command(bot_id, {'command': 'restart'})
        else:
            await websocket.send(json.dumps({
                'type': 'error',
                'message': f'Unknown command: {command}'
            }))
            
    async def forward_to_pc(self, message: BotMessage):
        """Forward processed message to all PC clients"""
        if not self.pc_clients:
            return
            
        message_json = json.dumps({
            'type': 'bot_message',
            'bot_id': message.bot_id,
            'message_type': message.message_type,
            'timestamp': message.timestamp,
            'data': message.data,
            'priority': message.priority
        })
        
        # Send to all connected PC clients
        disconnected = set()
        for pc_client in self.pc_clients:
            try:
                await pc_client.send(message_json)
            except websockets.exceptions.ConnectionClosed:
                disconnected.add(pc_client)
            except Exception as e:
                logger.error(f"Error forwarding to PC: {e}")
                disconnected.add(pc_client)
                
        # Cleanup disconnected clients
        self.pc_clients -= disconnected
        
    async def forward_to_bot(self, data: Dict):
        """Forward command to specific bot"""
        bot_id = data.get('target_bot')
        if bot_id and bot_id in self.bot_clients:
            try:
                await self.bot_clients[bot_id].send(json.dumps(data))
                logger.info(f"Command forwarded to {bot_id}")
            except Exception as e:
                logger.error(f"Error forwarding to bot {bot_id}: {e}")
        else:
            logger.warning(f"Bot {bot_id} not found for command forwarding")
            
    async def send_status_update(self, websocket):
        """Send system status to PC client"""
        status = {
            'type': 'status_update',
            'timestamp': time.time(),
            'uptime': time.time() - self.stats['uptime_start'],
            'connected_bots': len([b for b in self.bot_status.values() if b.is_online]),
            'total_bots': len(self.bot_status),
            'connected_pc_clients': len(self.pc_clients),
            'stats': self.stats,
            'system_info': self.get_system_info()
        }
        
        try:
            await websocket.send(json.dumps(status))
        except Exception as e:
            logger.error(f"Error sending status: {e}")
            
    async def send_bot_list(self, websocket):
        """Send list of known bots to PC client"""
        bot_list = {
            'type': 'bot_list',
            'timestamp': time.time(),
            'bots': [asdict(status) for status in self.bot_status.values()]
        }
        
        try:
            await websocket.send(json.dumps(bot_list))
        except Exception as e:
            logger.error(f"Error sending bot list: {e}")
            
    def get_system_info(self) -> Dict:
        """Get Raspberry Pi system information"""
        try:
            with open('/proc/cpuinfo', 'r') as f:
                cpuinfo = f.read()
            
            # Extract relevant info
            cpu_model = "BCM2837 ARM Cortex-A53"
            for line in cpuinfo.split('\n'):
                if 'model name' in line:
                    cpu_model = line.split(':')[1].strip()
                    break
                    
            return {
                'hostname': socket.gethostname(),
                'cpu_model': cpu_model,
                'platform': 'Raspberry Pi 3 Model B v1.2',
                'python_version': sys.version.split()[0],
                'local_ip': self.get_local_ip()
            }
        except Exception as e:
            logger.error(f"Error getting system info: {e}")
            return {'error': str(e)}
            
    def get_local_ip(self) -> str:
        """Get local IP address"""
        try:
            # Connect to Google DNS to determine local IP
            with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as s:
                s.connect(("8.8.8.8", 80))
                return s.getsockname()[0]
        except Exception:
            return "127.0.0.1"
            
    def update_filter_rules(self, new_rules: Dict):
        """Update signal filtering rules"""
        self.signal_filter.filter_rules.update(new_rules)
        logger.info(f"Filter rules updated: {self.signal_filter.filter_rules}")
        
    async def status_monitor(self):
        """Background task to monitor bot status and cleanup"""
        while self.running:
            try:
                current_time = time.time()
                timeout = 30.0  # 30 seconds timeout
                
                for bot_id, status in self.bot_status.items():
                    if current_time - status.last_seen > timeout:
                        if status.is_online:
                            logger.warning(f"Bot {bot_id} appears offline")
                            status.is_online = False
                            
                            # Notify PC clients
                            await self.broadcast_to_pc({
                                'type': 'bot_offline',
                                'bot_id': bot_id,
                                'timestamp': current_time
                            })
                            
                # Log periodic stats
                if int(current_time) % 300 == 0:  # Every 5 minutes
                    logger.info(f"Stats: {self.stats}")
                    
            except Exception as e:
                logger.error(f"Status monitor error: {e}")
                
            await asyncio.sleep(5)  # Check every 5 seconds
            
    async def broadcast_to_pc(self, message: Dict):
        """Broadcast message to all PC clients"""
        if not self.pc_clients:
            return
            
        message_json = json.dumps(message)
        disconnected = set()
        
        for pc_client in self.pc_clients:
            try:
                await pc_client.send(message_json)
            except Exception:
                disconnected.add(pc_client)
                
        self.pc_clients -= disconnected
        
    def signal_handler(self, signum, frame):
        """Handle shutdown signals"""
        logger.info("Shutdown signal received")
        self.running = False
        
    async def start_server(self):
        """Start the Micro Bot bridge server"""
        self.running = True
        
        # Setup signal handlers
        signal.signal(signal.SIGINT, self.signal_handler)
        signal.signal(signal.SIGTERM, self.signal_handler)
        
        logger.info("🤖 MICRO BOT Bridge Server Starting...")
        logger.info(f"Bot WebSocket server: {self.host}:{self.bot_port}")
        logger.info(f"PC WebSocket server: {self.host}:{self.pc_port}")
        
        # Start WebSocket servers
        bot_server = websockets.serve(
            self.handle_bot_connection, 
            self.host, 
            self.bot_port,
            ping_interval=20,
            ping_timeout=10
        )
        
        pc_server = websockets.serve(
            self.handle_pc_connection, 
            self.host, 
            self.pc_port,
            ping_interval=20,
            ping_timeout=10
        )
        
        # Start status monitor
        monitor_task = asyncio.create_task(self.status_monitor())
        
        logger.info("🚀 MICRO BOT Bridge Server online!")
        
        try:
            await asyncio.gather(
                bot_server,
                pc_server,
                monitor_task
            )
        except Exception as e:
            logger.error(f"Server error: {e}")
        finally:
            logger.info("🛑 MICRO BOT Bridge Server shutdown")

if __name__ == "__main__":
    # Create and run the Micro Bot server
    micro_bot = MicroBotServer()
    
    try:
        asyncio.run(micro_bot.start_server())
    except KeyboardInterrupt:
        logger.info("Server stopped by user")
    except Exception as e:
        logger.error(f"Fatal error: {e}")