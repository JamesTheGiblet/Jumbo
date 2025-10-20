import asyncio
import websockets
import json
import time
import threading
import logging
import random
from datetime import datetime
from typing import Dict, Set

# Configure logging
logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s - %(levelname)s - %(message)s',
    datefmt='%Y-%m-%d %H:%M:%S'
)
logger = logging.getLogger(__name__)

class ESPNowBridge:
    def __init__(self, host='0.0.0.0', port=8766):
        self.host = host
        self.port = port
        self.connected_clients: Set[websockets.WebSocketServerProtocol] = set()
        self.bot_data: Dict[str, dict] = {}
        self.start_time = time.time()
        
    async def start_websocket_server(self):
        """Start WebSocket server for PC MCU connections"""
        logger.info(f"Starting WebSocket server on {self.host}:{self.port}")
        
        async def handler(websocket, path):
            # Register new client
            self.connected_clients.add(websocket)
            client_ip = websocket.remote_address[0]
            logger.info(f"PC MCU connected: {client_ip}")
            
            try:
                # Send welcome message
                await websocket.send(json.dumps({
                    'type': 'system_message',
                    'message': 'Connected to Raspberry Pi Bridge',
                    'timestamp': time.time()
                }))
                
                # Send initial bot data
                if self.bot_data:
                    await websocket.send(json.dumps({
                        'type': 'bot_list',
                        'bots': list(self.bot_data.values())
                    }))
                
                # Send status update
                await websocket.send(json.dumps({
                    'type': 'status_update',
                    'connected_bots': len([b for b in self.bot_data.values() if b['is_online']]),
                    'uptime': time.time() - self.start_time,
                    'bridge_version': '1.0.0'
                }))
                
                # Handle incoming messages from PC MCU
                async for message in websocket:
                    await self.handle_pc_message(websocket, message)
                    
            except websockets.exceptions.ConnectionClosed:
                logger.info(f"PC MCU disconnected: {client_ip}")
            except Exception as e:
                logger.error(f"Error with client {client_ip}: {e}")
            finally:
                self.connected_clients.remove(websocket)
        
        return await websockets.serve(handler, self.host, self.port)
    
    async def handle_pc_message(self, websocket, message_str: str):
        """Handle messages from PC MCU"""
        try:
            message = json.loads(message_str)
            msg_type = message.get('type')
            
            logger.info(f"Received from PC MCU: {msg_type}")
            
            if msg_type == 'command':
                command = message.get('command')
                if command == 'get_bots':
                    await self.send_bot_list(websocket)
                elif command == 'get_status':
                    await self.send_status_update(websocket)
                elif command == 'ping':
                    await websocket.send(json.dumps({
                        'type': 'pong',
                        'timestamp': time.time()
                    }))
                    
            elif msg_type == 'mission':
                target_bot = message.get('target_bot')
                mission_data = message.get('mission_data')
                logger.info(f"Mission for {target_bot}: {mission_data}")
                
                # Simulate mission acceptance
                await websocket.send(json.dumps({
                    'type': 'mission_status',
                    'mission_id': mission_data.get('id'),
                    'status': 'accepted',
                    'bot_id': target_bot,
                    'timestamp': time.time()
                }))
                
        except json.JSONDecodeError as e:
            logger.error(f"Invalid JSON from PC MCU: {e}")
        except Exception as e:
            logger.error(f"Error handling PC message: {e}")
    
    async def send_bot_list(self, websocket=None):
        """Send bot list to specific client or broadcast"""
        message = {
            'type': 'bot_list',
            'bots': list(self.bot_data.values()),
            'timestamp': time.time()
        }
        
        if websocket:
            await websocket.send(json.dumps(message))
        else:
            await self.broadcast_message(message)
    
    async def send_status_update(self, websocket=None):
        """Send status update"""
        message = {
            'type': 'status_update',
            'connected_bots': len([b for b in self.bot_data.values() if b['is_online']]),
            'total_bots': len(self.bot_data),
            'uptime': time.time() - self.start_time,
            'bridge_clients': len(self.connected_clients),
            'timestamp': time.time()
        }
        
        if websocket:
            await websocket.send(json.dumps(message))
        else:
            await self.broadcast_message(message)
    
    async def broadcast_message(self, message):
        """Broadcast message to all connected WebSocket clients"""
        if self.connected_clients:
            message_str = json.dumps(message)
            disconnected_clients = []
            
            for client in self.connected_clients:
                try:
                    await client.send(message_str)
                except:
                    disconnected_clients.append(client)
            
            # Remove disconnected clients
            for client in disconnected_clients:
                self.connected_clients.remove(client)
    
    def simulate_espnow_bots(self):
        """Simulate ESP-NOW bots for testing"""
        bot_templates = [
            {
                'id': 'WHEELIE_001',
                'type': 'wheelie',
                'base_fitness': 0.6,
                'capabilities': ['mobility', 'exploration', 'object_avoidance']
            },
            {
                'id': 'SPEEDIE_001', 
                'type': 'speedie',
                'base_fitness': 0.8,
                'capabilities': ['high_speed', 'patrol', 'rapid_response']
            },
            {
                'id': 'MICRO_001',
                'type': 'micro',
                'base_fitness': 0.4,
                'capabilities': ['sensing', 'data_collection', 'low_power']
            }
        ]
        
        while True:
            for template in bot_templates:
                # Simulate bot data updates
                fitness_variation = random.uniform(-0.2, 0.2)
                current_fitness = max(0.1, min(1.0, template['base_fitness'] + fitness_variation))
                
                bot_message = {
                    'bot_id': template['id'],
                    'type': 'bot_message',
                    'data': {
                        'generation': random.randint(1, 5),
                        'fitness_score': current_fitness,
                        'emotional_state': {
                            'curiosity': random.uniform(0.3, 0.9),
                            'confidence': current_fitness,
                            'social_drive': random.uniform(0.4, 0.8),
                            'aggression': random.uniform(0.1, 0.5)
                        },
                        'sensor_data': {
                            'battery': random.uniform(3.5, 4.2),
                            'temperature': random.uniform(20, 35),
                            'humidity': random.uniform(30, 80),
                            'signal_strength': random.uniform(-70, -40)
                        },
                        'position': {
                            'x': random.uniform(0, 10),
                            'y': random.uniform(0, 10),
                            'heading': random.uniform(0, 360)
                        }
                    },
                    'timestamp': time.time(),
                    'rssi': random.randint(-80, -40)
                }
                
                # Update bot data store
                self.bot_data[template['id']] = {
                    'bot_id': template['id'],
                    'type': template['type'],
                    'is_online': True,
                    'generation': bot_message['data']['generation'],
                    'fitness_score': current_fitness,
                    'last_seen': datetime.now().isoformat(),
                    'message_count': self.bot_data.get(template['id'], {}).get('message_count', 0) + 1,
                    'capabilities': template['capabilities'],
                    'ip_address': f"192.168.1.{random.randint(100, 200)}",
                    'emotional_state': bot_message['data']['emotional_state'],
                    'sensor_data': bot_message['data']['sensor_data']
                }
                
                # Broadcast to all connected PC MCU clients
                asyncio.run_coroutine_threadsafe(
                    self.broadcast_message(bot_message),
                    asyncio.get_event_loop()
                )
            
            # Occasionally send bot list updates
            if random.random() < 0.3:  # 30% chance
                asyncio.run_coroutine_threadsafe(
                    self.send_bot_list(),
                    asyncio.get_event_loop()
                )
            
            # Occasionally send status updates  
            if random.random() < 0.2:  # 20% chance
                asyncio.run_coroutine_threadsafe(
                    self.send_status_update(),
                    asyncio.get_event_loop()
                )
            
            time.sleep(3)  # Simulate message interval
    
    async def run(self):
        """Main bridge operation"""
        logger.info("🔄 Starting Project Jumbo Raspberry Pi Bridge")
        
        # Start bot simulation in background thread
        bot_thread = threading.Thread(target=self.simulate_espnow_bots, daemon=True)
        bot_thread.start()
        
        # Start WebSocket server
        server = await self.start_websocket_server()
        
        logger.info("✅ Bridge initialized successfully!")
        logger.info(f"🌐 WebSocket server listening on {self.host}:{self.port}")
        logger.info("🤖 Bot simulation started")
        logger.info("🚀 Ready for PC MCU connections...")
        
        # Periodic status logging
        async def status_logger():
            while True:
                await asyncio.sleep(30)
                online_bots = len([b for b in self.bot_data.values() if b['is_online']])
                logger.info(f"📊 Status: {online_bots} bots online, {len(self.connected_clients)} MCU clients connected")
        
        asyncio.create_task(status_logger())
        
        # Keep the server running
        try:
            await asyncio.Future()  # Run forever
        except KeyboardInterrupt:
            logger.info("Bridge shutdown requested")

def main():
    """Main function"""
    print("🔄 Project Jumbo - Raspberry Pi Bridge")
    print("================================================")
    print("Starting bridge with virtual environment...")
    
    # Check if we're in a virtual environment
    import sys
    if hasattr(sys, 'real_prefix') or (hasattr(sys, 'base_prefix') and sys.base_prefix != sys.prefix):
        print("✅ Running in virtual environment")
    else:
        print("⚠️  Not running in virtual environment - activate with: source jumbo_bridge/bin/activate")
    
    bridge = ESPNowBridge()
    
    try:
        asyncio.run(bridge.run())
    except KeyboardInterrupt:
        print("\n👋 Bridge stopped gracefully")
    except Exception as e:
        print(f"❌ Bridge error: {e}")
        logger.exception("Fatal error")

if __name__ == "__main__":
    main()
    