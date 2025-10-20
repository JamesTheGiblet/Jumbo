#!/usr/bin/env python3
"""
Clean Bridge Server - No Simulated Bots
Real ESP32 WHEELIE/SPEEDIE bot communication bridge
"""
import asyncio
import websockets
import json
import time
import logging
from datetime import datetime
from typing import Dict, List, Optional, Any
from dataclasses import dataclass, asdict
from collections import defaultdict, deque

# Set up logging
logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s - %(name)s - %(levelname)s - %(message)s'
)
logger = logging.getLogger('MicroBot')

@dataclass
class BotStatus:
    """Status information for a single bot"""
    bot_id: str
    is_online: bool = False
    ip_address: str = ""
    generation: int = 0
    fitness_score: float = 0.0
    last_seen: Optional[datetime] = None
    message_count: int = 0
    emotional_state: Dict = None
    sensor_data: Dict = None
    capabilities: List = None
    
    def __post_init__(self):
        if self.emotional_state is None:
            self.emotional_state = {}
        if self.sensor_data is None:
            self.sensor_data = {}
        if self.capabilities is None:
            self.capabilities = []

class ESPNowBridge:
    """ESP-NOW Communication Bridge - Real Bots Only"""
    
    def __init__(self):
        self.pc_clients = set()
        self.bot_clients = set()
        self.bot_status = {}  # No pre-populated bots
        self.running = True
        
        # Statistics
        self.stats = {
            'messages_processed': 0,
            'messages_filtered': 0, 
            'messages_forwarded': 0,
            'uptime_start': time.time(),
            'swarm_messages_analyzed': 0,
            'swarm_behaviors_detected': 0
        }
        
        logger.info("🤖 Bridge initialized - Ready for real ESP32 bots")
    
    async def handle_bot_connection(self, websocket):
        """Handle connections from ESP32 bots (WHEELIE/SPEEDIE)"""
        bot_ip = websocket.remote_address[0]
        logger.info(f"🤖 ESP32 bot connected from {bot_ip}")
        
        try:
            self.bot_clients.add(websocket)
            
            async for message in websocket:
                try:
                    # Parse ESP32 bot message
                    if message.startswith('{'):
                        # JSON message
                        data = json.loads(message)
                        await self.process_bot_message(data, bot_ip, websocket)
                    else:
                        # Raw ESP-NOW message
                        await self.process_raw_message(message, bot_ip)
                        
                    self.stats['messages_processed'] += 1
                    
                except json.JSONDecodeError:
                    logger.warning(f"Invalid JSON from bot {bot_ip}: {message[:100]}")
                except Exception as e:
                    logger.error(f"Error processing bot message from {bot_ip}: {e}")
                    
        except websockets.exceptions.ConnectionClosed:
            logger.info(f"🤖 ESP32 bot {bot_ip} disconnected")
        except Exception as e:
            logger.error(f"Bot connection error from {bot_ip}: {e}")
        finally:
            self.bot_clients.discard(websocket)
            # Mark bot as offline
            for bot_id, status in self.bot_status.items():
                if status.ip_address == bot_ip:
                    status.is_online = False
                    await self.notify_pc_clients('bot_offline', {'bot_id': bot_id})
    
    async def process_bot_message(self, data: Dict, bot_ip: str, websocket):
        """Process structured message from ESP32 bot"""
        bot_id = data.get('bot_id', f"BOT_{bot_ip.replace('.', '_')}")
        
        # Update or create bot status
        if bot_id not in self.bot_status:
            self.bot_status[bot_id] = BotStatus(
                bot_id=bot_id,
                ip_address=bot_ip,
                is_online=True
            )
            logger.info(f"🆕 New bot registered: {bot_id}")
            
        bot = self.bot_status[bot_id]
        bot.is_online = True
        bot.last_seen = datetime.now()
        bot.message_count += 1
        
        # Update bot data from message
        if 'generation' in data:
            bot.generation = data['generation']
        if 'fitness_score' in data:
            bot.fitness_score = data['fitness_score']
        if 'emotional_state' in data:
            bot.emotional_state.update(data['emotional_state'])
        if 'sensor_data' in data:
            bot.sensor_data.update(data['sensor_data'])
            
        # Forward to PC clients
        await self.notify_pc_clients('bot_message', {
            'bot_id': bot_id,
            'data': data,
            'timestamp': time.time()
        })
        
    async def process_raw_message(self, message: str, bot_ip: str):
        """Process raw ESP-NOW message"""
        logger.info(f"📡 Raw ESP-NOW from {bot_ip}: {message[:50]}...")
        
        # Forward raw message to PC clients for analysis
        await self.notify_pc_clients('raw_message', {
            'source_ip': bot_ip,
            'message': message,
            'timestamp': time.time()
        })
    
    async def handle_pc_connection(self, websocket):
        """Handle connections from PC applications"""
        pc_ip = websocket.remote_address[0]
        
        try:
            logger.info(f"💻 PC client connected from {pc_ip}")
            self.pc_clients.add(websocket)
            
            # Send initial status (empty if no bots connected yet)
            await self.send_initial_status(websocket)
            
            async for message in websocket:
                try:
                    data = json.loads(message)
                    logger.info(f"📥 PC command: {data.get('command', 'unknown')}")
                    
                    # Handle PC commands
                    if data.get('type') == 'command':
                        await self.handle_pc_command(data, websocket)
                    elif data.get('type') == 'bot_command':
                        await self.forward_to_bot(data)
                        
                except json.JSONDecodeError as e:
                    logger.error(f"Invalid JSON from PC {pc_ip}: {e}")
                except Exception as e:
                    logger.error(f"Error processing PC message: {e}")
                    
        except websockets.exceptions.ConnectionClosed:
            logger.info(f"💻 PC client {pc_ip} disconnected")
        except Exception as e:
            logger.error(f"PC connection error: {e}")
        finally:
            self.pc_clients.discard(websocket)
    
    async def send_initial_status(self, websocket):
        """Send initial status to newly connected PC"""
        try:
            bot_list = []
            for bot_status in self.bot_status.values():
                bot_dict = {
                    'bot_id': bot_status.bot_id,
                    'is_online': bot_status.is_online,
                    'ip_address': bot_status.ip_address,
                    'generation': bot_status.generation,
                    'fitness_score': bot_status.fitness_score,
                    'last_seen': bot_status.last_seen.isoformat() if bot_status.last_seen else None,
                    'message_count': bot_status.message_count,
                    'emotional_state': bot_status.emotional_state,
                    'sensor_data': bot_status.sensor_data,
                    'capabilities': bot_status.capabilities
                }
                bot_list.append(bot_dict)
            
            status = {
                'type': 'status_update',
                'timestamp': time.time(),
                'uptime': time.time() - self.stats['uptime_start'],
                'connected_bots': len([b for b in self.bot_status.values() if b.is_online]),
                'total_bots': len(self.bot_status),
                'connected_pc_clients': len(self.pc_clients),
                'stats': self.stats,
                'bots': bot_list
            }
            
            await websocket.send(json.dumps(status))
            logger.info(f"📤 Sent initial status: {len(bot_list)} bots")
            
        except Exception as e:
            logger.error(f"Error sending initial status: {e}")
    
    async def handle_pc_command(self, data: Dict, websocket):
        """Handle commands from PC"""
        command = data.get('command')
        
        if command == 'get_status':
            await self.send_initial_status(websocket)
        elif command == 'get_bots':
            await self.send_bot_list(websocket)
        elif command == 'ping':
            await websocket.send(json.dumps({
                'type': 'pong',
                'timestamp': time.time()
            }))
    
    async def send_bot_list(self, websocket):
        """Send list of connected bots to PC client"""
        try:
            bot_list = []
            for bot_status in self.bot_status.values():
                bot_dict = {
                    'bot_id': bot_status.bot_id,
                    'is_online': bot_status.is_online,
                    'ip_address': bot_status.ip_address,
                    'generation': bot_status.generation,
                    'fitness_score': bot_status.fitness_score,
                    'last_seen': bot_status.last_seen.isoformat() if bot_status.last_seen else None,
                    'message_count': bot_status.message_count,
                    'emotional_state': bot_status.emotional_state,
                    'sensor_data': bot_status.sensor_data,
                    'capabilities': bot_status.capabilities
                }
                bot_list.append(bot_dict)
            
            response = {
                'type': 'bot_list',
                'timestamp': time.time(),
                'bots': bot_list
            }
            
            await websocket.send(json.dumps(response))
            logger.info(f"📤 Sent bot list: {len(bot_list)} bots")
            
        except Exception as e:
            logger.error(f"Error sending bot list: {e}")
            
    async def forward_to_bot(self, data: Dict):
        """Forward command to specific bot"""
        bot_id = data.get('bot_id')
        command = data.get('command')
        
        if bot_id in self.bot_status:
            bot_ip = self.bot_status[bot_id].ip_address
            
            # Find bot websocket by IP
            for bot_ws in self.bot_clients:
                if bot_ws.remote_address[0] == bot_ip:
                    try:
                        await bot_ws.send(json.dumps({
                            'type': 'command',
                            'command': command,
                            'parameters': data.get('parameters', {})
                        }))
                        logger.info(f"📤 Forwarded command '{command}' to {bot_id}")
                        break
                    except Exception as e:
                        logger.error(f"Error forwarding to {bot_id}: {e}")
        else:
            logger.warning(f"Bot {bot_id} not found for command forwarding")
            
    async def notify_pc_clients(self, msg_type: str, data: Dict):
        """Notify all connected PC clients"""
        if not self.pc_clients:
            return
            
        message = {
            'type': msg_type,
            'timestamp': time.time(),
            **data
        }
        
        disconnected = set()
        for pc_ws in self.pc_clients:
            try:
                await pc_ws.send(json.dumps(message))
            except websockets.exceptions.ConnectionClosed:
                disconnected.add(pc_ws)
            except Exception as e:
                logger.error(f"Error notifying PC client: {e}")
                disconnected.add(pc_ws)
                
        # Clean up disconnected clients
        self.pc_clients -= disconnected

    async def start_servers(self):
        """Start both WebSocket servers"""
        logger.info("🤖 MICRO BOT Bridge Server Starting...")
        logger.info("📡 ESP32 Bot server: 0.0.0.0:8765")
        logger.info("💻 PC Client server: 0.0.0.0:8766")
        logger.info("🎯 Ready for real WHEELIE/SPEEDIE bots")
        
        # Start bot server (ESP32 connections)
        bot_server = websockets.serve(
            self.handle_bot_connection,
            "0.0.0.0", 
            8765
        )
        
        # Start PC server
        pc_server = websockets.serve(
            self.handle_pc_connection,
            "0.0.0.0",
            8766
        )
        
        logger.info("🚀 MICRO BOT Bridge Server online!")
        
        # Run both servers
        await asyncio.gather(
            bot_server,
            pc_server,
            self.stats_loop()
        )
    
    async def stats_loop(self):
        """Periodic statistics update"""
        while self.running:
            await asyncio.sleep(30)
            online_bots = len([b for b in self.bot_status.values() if b.is_online])
            logger.info(f"📊 Stats: {online_bots} bots online, {len(self.pc_clients)} PC clients, {self.stats['messages_processed']} messages processed")

async def main():
    """Main entry point"""
    bridge = ESPNowBridge()
    
    try:
        await bridge.start_servers()
    except KeyboardInterrupt:
        logger.info("🛑 Shutting down bridge server...")
        bridge.running = False
    except Exception as e:
        logger.error(f"Bridge server error: {e}")

if __name__ == "__main__":
    asyncio.run(main())