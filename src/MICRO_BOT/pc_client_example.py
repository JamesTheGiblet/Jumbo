#!/usr/bin/env python3
"""
🖥️ PC Client Example for MICRO BOT Bridge
Test client to demonstrate PC-to-MicroBot communication

Usage:
    python pc_client_example.py

This client connects to the MICRO BOT bridge and demonstrates:
- Status monitoring
- Bot list retrieval
- Command sending
- Real-time message reception
"""

import asyncio
import websockets
import json
import time
from datetime import datetime

class MicroBotClient:
    """Example PC client for MICRO BOT bridge"""
    
    def __init__(self, host='192.168.4.1', port=8766):
        self.host = host
        self.port = port
        self.websocket = None
        self.running = False
        
    async def connect(self):
        """Connect to MICRO BOT bridge"""
        try:
            uri = f"ws://{self.host}:{self.port}"
            print(f"🔗 Connecting to MICRO BOT at {uri}")
            
            self.websocket = await websockets.connect(uri)
            self.running = True
            
            print("✅ Connected to MICRO BOT bridge!")
            return True
            
        except Exception as e:
            print(f"❌ Connection failed: {e}")
            return False
            
    async def disconnect(self):
        """Disconnect from bridge"""
        self.running = False
        if self.websocket:
            await self.websocket.close()
            print("🔌 Disconnected from MICRO BOT")
            
    async def send_command(self, command, **kwargs):
        """Send command to MICRO BOT"""
        if not self.websocket:
            print("❌ Not connected")
            return
            
        message = {
            'type': 'command',
            'command': command,
            'timestamp': time.time(),
            **kwargs
        }
        
        try:
            await self.websocket.send(json.dumps(message))
            print(f"📤 Sent command: {command}")
        except Exception as e:
            print(f"❌ Failed to send command: {e}")
            
    async def listen_for_messages(self):
        """Listen for messages from MICRO BOT"""
        try:
            async for message in self.websocket:
                data = json.loads(message)
                await self.handle_message(data)
                
        except websockets.exceptions.ConnectionClosed:
            print("🔌 Connection closed by server")
        except Exception as e:
            print(f"❌ Message reception error: {e}")
            
    async def handle_message(self, data):
        """Handle incoming messages"""
        msg_type = data.get('type', 'unknown')
        timestamp = datetime.fromtimestamp(data.get('timestamp', 0))
        
        if msg_type == 'status_update':
            print(f"\n📊 MICRO BOT Status Update ({timestamp.strftime('%H:%M:%S')})")
            print(f"   Uptime: {data.get('uptime', 0):.1f}s")
            print(f"   Connected Bots: {data.get('connected_bots', 0)}")
            print(f"   PC Clients: {data.get('connected_pc_clients', 0)}")
            
            stats = data.get('stats', {})
            print(f"   Messages Processed: {stats.get('messages_processed', 0)}")
            print(f"   Messages Filtered: {stats.get('messages_filtered', 0)}")
            print(f"   Messages Forwarded: {stats.get('messages_forwarded', 0)}")
            
        elif msg_type == 'bot_list':
            print(f"\n🤖 Bot List ({timestamp.strftime('%H:%M:%S')})")
            bots = data.get('bots', [])
            if bots:
                for bot in bots:
                    status_icon = "🟢" if bot.get('is_online') else "🔴"
                    print(f"   {status_icon} {bot.get('bot_id', 'Unknown')}")
                    print(f"      IP: {bot.get('ip_address', 'N/A')}")
                    print(f"      Generation: {bot.get('generation', 0)}")
                    print(f"      Fitness: {bot.get('fitness_score', 0.0):.3f}")
                    print(f"      Messages: {bot.get('message_count', 0)}")
            else:
                print("   No bots connected")
                
        elif msg_type == 'bot_message':
            bot_id = data.get('bot_id', 'Unknown')
            msg_type_bot = data.get('message_type', 'unknown')
            priority = data.get('priority', 1)
            
            priority_icon = "🔴" if priority == 3 else "🟡" if priority == 2 else "🟢"
            print(f"\n📨 {priority_icon} Bot Message from {bot_id} ({timestamp.strftime('%H:%M:%S')})")
            print(f"   Type: {msg_type_bot}")
            
            bot_data = data.get('data', {})
            if 'generation' in bot_data:
                print(f"   Generation: {bot_data['generation']}")
            if 'fitness_score' in bot_data:
                print(f"   Fitness: {bot_data['fitness_score']:.3f}")
            if 'emotional_state' in bot_data:
                emotions = bot_data['emotional_state']
                print(f"   Emotions: Frustration={emotions.get('frustration', 0)}, "
                      f"Confidence={emotions.get('confidence', 0)}")
                      
        elif msg_type == 'bot_offline':
            bot_id = data.get('bot_id', 'Unknown')
            print(f"\n🔴 Bot {bot_id} went offline ({timestamp.strftime('%H:%M:%S')})")
            
        elif msg_type == 'response':
            status = data.get('status', 'unknown')
            message = data.get('message', '')
            icon = "✅" if status == 'success' else "❌"
            print(f"\n{icon} Server Response: {message}")
            
        elif msg_type == 'error':
            message = data.get('message', 'Unknown error')
            print(f"\n❌ Error: {message}")
            
        else:
            print(f"\n❓ Unknown message type: {msg_type}")
            
    async def interactive_mode(self):
        """Interactive command mode"""
        print("\n🎮 Interactive Mode - Available Commands:")
        print("   status - Get system status")
        print("   bots - List connected bots")
        print("   restart <bot_id> - Restart specific bot")
        print("   filter <rate_limit> - Set rate limit (messages/sec)")
        print("   quit - Exit")
        print("\nType commands below:")
        
        while self.running:
            try:
                cmd = input("\n> ").strip().lower()
                
                if cmd == 'quit':
                    break
                elif cmd == 'status':
                    await self.send_command('get_status')
                elif cmd == 'bots':
                    await self.send_command('get_bots')
                elif cmd.startswith('restart '):
                    bot_id = cmd.split(' ', 1)[1]
                    await self.send_command('restart_bot', bot_id=bot_id)
                elif cmd.startswith('filter '):
                    try:
                        rate_limit = int(cmd.split(' ', 1)[1])
                        await self.send_command('set_filter_rules', 
                                               rules={'rate_limit': rate_limit})
                    except ValueError:
                        print("❌ Invalid rate limit value")
                elif cmd == 'help':
                    print("📋 Available commands: status, bots, restart <bot_id>, filter <rate>, quit")
                elif cmd:
                    print("❓ Unknown command. Type 'help' for available commands.")
                    
            except KeyboardInterrupt:
                break
            except EOFError:
                break
                
    async def run_demo(self):
        """Run demonstration sequence"""
        if not await self.connect():
            return
            
        try:
            # Start message listener
            listen_task = asyncio.create_task(self.listen_for_messages())
            
            # Wait a moment for connection
            await asyncio.sleep(1)
            
            # Request initial status
            await self.send_command('get_status')
            await asyncio.sleep(2)
            
            # Request bot list
            await self.send_command('get_bots')
            await asyncio.sleep(2)
            
            # Start interactive mode
            interactive_task = asyncio.create_task(self.interactive_mode())
            
            # Wait for tasks
            await asyncio.gather(listen_task, interactive_task, 
                               return_exceptions=True)
                               
        except KeyboardInterrupt:
            print("\n🛑 Interrupted by user")
        finally:
            await self.disconnect()

def main():
    """Main function"""
    print("🖥️ PC Client for MICRO BOT Bridge")
    print("=================================")
    print("This client demonstrates communication with the MICRO BOT bridge")
    print("Make sure MICRO BOT is running and accessible")
    
    # Get connection details
    host = input("\nMICRO BOT IP (default: 192.168.4.1): ").strip()
    if not host:
        host = '192.168.4.1'
        
    try:
        port = input("Port (default: 8766): ").strip()
        port = int(port) if port else 8766
    except ValueError:
        port = 8766
        
    print(f"\n🎯 Target: {host}:{port}")
    
    # Create and run client
    client = MicroBotClient(host, port)
    
    try:
        asyncio.run(client.run_demo())
    except KeyboardInterrupt:
        print("\n👋 Goodbye!")

if __name__ == "__main__":
    main()