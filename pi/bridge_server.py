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

@dataclass  
class SwarmState:
    """Swarm intelligence state tracking"""
    leader_bot: Optional[str] = None
    leadership_election_active: bool = False
    active_formations: Dict[str, Any] = None
    emergent_behavior: str = "none"
    swarm_coherence: float = 0.0
    active_tasks: List[Dict] = None
    exploration_zones: List[Dict] = None
    
    def __post_init__(self):
        if self.active_formations is None:
            self.active_formations = {}
        if self.active_tasks is None:
            self.active_tasks = []
        if self.exploration_zones is None:
            self.exploration_zones = []

class SwarmIntelligenceMonitor:
    """Monitor and coordinate ESP-NOW swarm communications"""
    
    def __init__(self):
        self.swarm_state = SwarmState()
        self.swarm_messages = deque(maxlen=500)  # Store recent swarm messages
        self.peer_relationships = {}  # Track peer connections
        self.behavior_patterns = defaultdict(int)  # Track behavior frequency
        self.task_assignments = {}  # Track task assignments
        self.formation_history = deque(maxlen=50)  # Formation changes
        
    def process_swarm_message(self, bot_id: str, message: Dict) -> Dict:
        """Process ESP-NOW swarm communication message"""
        msg_type = message.get('header', {}).get('messageType', 0)
        timestamp = time.time()
        
        # Store message for analysis
        swarm_msg = {
            'bot_id': bot_id,
            'timestamp': timestamp,
            'type': msg_type,
            'payload': message.get('payload', {}),
            'priority': message.get('header', {}).get('priority', 1)
        }
        self.swarm_messages.append(swarm_msg)
        
        # Process specific message types
        if msg_type == 0x01:  # MSG_DISCOVERY
            return self._handle_discovery(bot_id, message)
        elif msg_type == 0x30:  # MSG_LEADER_ELECTION
            return self._handle_leadership_election(bot_id, message)
        elif msg_type == 0x20:  # MSG_TASK_ASSIGNMENT
            return self._handle_task_assignment(bot_id, message)
        elif msg_type == 0x22:  # MSG_FORMATION_REQUEST
            return self._handle_formation_request(bot_id, message)
        elif msg_type == 0x32:  # MSG_AREA_CLAIM
            return self._handle_area_claim(bot_id, message)
        elif msg_type == 0x40:  # MSG_GENOME_SHARE
            return self._handle_genome_share(bot_id, message)
        
        return self._create_analysis_response(msg_type, bot_id)
    
    def _handle_discovery(self, bot_id: str, message: Dict) -> Dict:
        """Handle bot discovery messages"""
        payload = message.get('payload', {})
        
        # Update peer relationships
        if bot_id not in self.peer_relationships:
            self.peer_relationships[bot_id] = {
                'first_seen': time.time(),
                'last_discovery': time.time(),
                'bot_type': payload.get('botType', 0),
                'role': payload.get('currentRole', 0),
                'capabilities': payload.get('capabilities', []),
                'discovery_count': 1
            }
        else:
            self.peer_relationships[bot_id]['last_discovery'] = time.time()
            self.peer_relationships[bot_id]['discovery_count'] += 1
            
        logger.info(f"🔍 Discovery from {bot_id}: Role={payload.get('currentRole', 0)}")
        
        return {
            'analysis': 'discovery_processed',
            'peer_count': len(self.peer_relationships),
            'bot_type': payload.get('botType', 0),
            'swarm_size': len([p for p in self.peer_relationships.values() 
                             if time.time() - p['last_discovery'] < 30])
        }
    
    def _handle_leadership_election(self, bot_id: str, message: Dict) -> Dict:
        """Handle leadership election messages"""
        payload = message.get('payload', {})
        
        self.swarm_state.leadership_election_active = True
        
        # Track leadership bids
        if 'generation' in payload and 'fitnessScore' in payload:
            fitness = payload['fitnessScore']
            generation = payload['generation']
            
            # Simple leader selection based on fitness
            if (self.swarm_state.leader_bot is None or 
                fitness > self._get_leader_fitness()):
                self.swarm_state.leader_bot = bot_id
                logger.info(f"👑 New swarm leader: {bot_id} (Fitness: {fitness:.3f})")
                
        return {
            'analysis': 'leadership_election',
            'current_leader': self.swarm_state.leader_bot,
            'election_active': self.swarm_state.leadership_election_active,
            'candidate_fitness': payload.get('fitnessScore', 0)
        }
    
    def _handle_task_assignment(self, bot_id: str, message: Dict) -> Dict:
        """Handle task assignment messages"""
        payload = message.get('payload', {})
        task_id = payload.get('taskId', 0)
        
        # Track task assignments
        if task_id not in self.task_assignments:
            self.task_assignments[task_id] = {
                'assigned_by': bot_id,
                'task_type': payload.get('taskType', 0),
                'priority': payload.get('taskPriority', 1),
                'created_time': time.time(),
                'status': 'assigned',
                'target_bot': payload.get('targetBot', 'unknown')
            }
            
        # Add to swarm state
        task_summary = {
            'id': task_id,
            'type': payload.get('taskType', 0),
            'priority': payload.get('taskPriority', 1),
            'assigned_by': bot_id,
            'status': 'active'
        }
        
        # Remove duplicates and add new task
        self.swarm_state.active_tasks = [t for t in self.swarm_state.active_tasks 
                                       if t.get('id') != task_id]
        self.swarm_state.active_tasks.append(task_summary)
        
        logger.info(f"📋 Task {task_id} assigned by {bot_id}")
        
        return {
            'analysis': 'task_assignment',
            'task_id': task_id,
            'total_active_tasks': len(self.swarm_state.active_tasks),
            'task_distribution': self._analyze_task_distribution()
        }
    
    def _handle_formation_request(self, bot_id: str, message: Dict) -> Dict:
        """Handle formation change requests"""
        payload = message.get('payload', {})
        formation_type = payload.get('formationType', 0)
        
        # Track formation changes
        formation_change = {
            'timestamp': time.time(),
            'requested_by': bot_id,
            'formation_type': formation_type,
            'scale': payload.get('scale', 1.0)
        }
        self.formation_history.append(formation_change)
        
        # Update current formation
        self.swarm_state.active_formations[bot_id] = {
            'type': formation_type,
            'scale': payload.get('scale', 1.0),
            'updated': time.time()
        }
        
        logger.info(f"🔄 Formation request from {bot_id}: Type={formation_type}")
        
        return {
            'analysis': 'formation_change',
            'formation_type': formation_type,
            'requesting_bot': bot_id,
            'formation_stability': self._calculate_formation_stability()
        }
    
    def _handle_area_claim(self, bot_id: str, message: Dict) -> Dict:
        """Handle exploration area claims"""
        payload = message.get('payload', {})
        
        area_info = {
            'bot_id': bot_id,
            'center_x': payload.get('centerX', 0),
            'center_y': payload.get('centerY', 0),
            'width': payload.get('width', 0),
            'height': payload.get('height', 0),
            'strategy': payload.get('strategy', 0),
            'claimed_time': time.time()
        }
        
        # Add to exploration zones
        self.swarm_state.exploration_zones.append(area_info)
        
        # Keep only recent zones (last 20)
        if len(self.swarm_state.exploration_zones) > 20:
            self.swarm_state.exploration_zones = self.swarm_state.exploration_zones[-20:]
            
        logger.info(f"🗺️ Area claimed by {bot_id}: ({area_info['center_x']}, {area_info['center_y']})")
        
        return {
            'analysis': 'area_claim',
            'total_zones': len(self.swarm_state.exploration_zones),
            'coverage_efficiency': self._calculate_coverage_efficiency()
        }
    
    def _handle_genome_share(self, bot_id: str, message: Dict) -> Dict:
        """Handle genome sharing for collective learning"""
        payload = message.get('payload', {})
        
        fitness = payload.get('fitnessScore', 0)
        generation = payload.get('generation', 0)
        
        # Track learning patterns
        self.behavior_patterns['genome_sharing'] += 1
        
        logger.info(f"🧬 Genome share from {bot_id}: Gen={generation}, Fit={fitness:.3f}")
        
        return {
            'analysis': 'genome_sharing',
            'sharing_bot': bot_id,
            'fitness_score': fitness,
            'generation': generation,
            'learning_activity': self.behavior_patterns['genome_sharing']
        }
    
    def _create_analysis_response(self, msg_type: int, bot_id: str) -> Dict:
        """Create analysis response for unknown message types"""
        return {
            'analysis': 'unknown_message',
            'message_type': msg_type,
            'bot_id': bot_id,
            'timestamp': time.time()
        }
    
    def _get_leader_fitness(self) -> float:
        """Get current leader's fitness score"""
        if not self.swarm_state.leader_bot:
            return 0.0
            
        # Find recent discovery message from leader
        for msg in reversed(self.swarm_messages):
            if (msg['bot_id'] == self.swarm_state.leader_bot and 
                msg['type'] == 0x01):  # Discovery message
                return msg['payload'].get('fitnessScore', 0.0)
        return 0.0
    
    def _analyze_task_distribution(self) -> Dict:
        """Analyze task distribution across swarm"""
        if not self.swarm_state.active_tasks:
            return {'total': 0, 'by_priority': {}}
            
        by_priority = defaultdict(int)
        for task in self.swarm_state.active_tasks:
            priority = task.get('priority', 1)
            by_priority[priority] += 1
            
        return {
            'total': len(self.swarm_state.active_tasks),
            'by_priority': dict(by_priority)
        }
    
    def _calculate_formation_stability(self) -> float:
        """Calculate formation stability score"""
        if len(self.formation_history) < 2:
            return 1.0
            
        recent_changes = len([f for f in self.formation_history 
                            if time.time() - f['timestamp'] < 60])
        
        # More changes = less stability
        stability = max(0.0, 1.0 - (recent_changes / 10.0))
        return stability
    
    def _calculate_coverage_efficiency(self) -> float:
        """Calculate exploration coverage efficiency"""
        if not self.swarm_state.exploration_zones:
            return 0.0
            
        # Simple efficiency based on zone count and time
        recent_zones = [z for z in self.swarm_state.exploration_zones 
                       if time.time() - z['claimed_time'] < 300]  # Last 5 minutes
        
        return min(1.0, len(recent_zones) / 5.0)  # Normalize to 0-1
    
    def get_swarm_summary(self) -> Dict:
        """Get comprehensive swarm intelligence summary"""
        active_peers = len([p for p in self.peer_relationships.values() 
                          if time.time() - p['last_discovery'] < 30])
        
        return {
            'swarm_size': active_peers,
            'leader': self.swarm_state.leader_bot,
            'leadership_election_active': self.swarm_state.leadership_election_active,
            'active_tasks': len(self.swarm_state.active_tasks),
            'exploration_zones': len(self.swarm_state.exploration_zones),
            'formation_stability': self._calculate_formation_stability(),
            'coverage_efficiency': self._calculate_coverage_efficiency(),
            'behavior_patterns': dict(self.behavior_patterns),
            'recent_messages': len(self.swarm_messages),
            'swarm_coherence': self._calculate_swarm_coherence()
        }
    
    def _calculate_swarm_coherence(self) -> float:
        """Calculate overall swarm coherence"""
        if not self.peer_relationships:
            return 0.0
            
        # Factors: recent activity, formation stability, task coordination
        recent_activity = len([msg for msg in self.swarm_messages 
                             if time.time() - msg['timestamp'] < 30])
        
        activity_score = min(1.0, recent_activity / 20.0)
        formation_score = self._calculate_formation_stability()
        task_score = min(1.0, len(self.swarm_state.active_tasks) / 5.0)
        
        coherence = (activity_score + formation_score + task_score) / 3.0
        self.swarm_state.swarm_coherence = coherence
        return coherence

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
        self.swarm_monitor = SwarmIntelligenceMonitor()  # Swarm intelligence monitoring
        
        self.running = False
        self.stats = {
            'messages_processed': 0,
            'messages_filtered': 0,
            'messages_forwarded': 0,
            'uptime_start': time.time(),
            'swarm_messages_analyzed': 0,
            'swarm_behaviors_detected': 0
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
                    
                    # Analyze swarm intelligence patterns
                    swarm_analysis = None
                    if 'header' in data and 'messageType' in data['header']:
                        # This is an ESP-NOW swarm message
                        swarm_analysis = self.swarm_monitor.process_swarm_message(bot_id, data)
                        self.stats['swarm_messages_analyzed'] += 1
                        
                        # Add swarm analysis to message data
                        processed_message.data['swarm_analysis'] = swarm_analysis
                        
                        logger.debug(f"📡 Swarm message from {bot_id}: Type=0x{data['header']['messageType']:02X}")
                    
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
        elif command == 'get_swarm_status':
            await self.send_swarm_status(websocket)
        elif command == 'get_swarm_analysis':
            await self.send_swarm_analysis(websocket)
        elif command == 'trigger_leader_election':
            await self.trigger_swarm_leader_election()
            await websocket.send(json.dumps({
                'type': 'response',
                'status': 'success',
                'message': 'Leader election triggered'
            }))
        elif command == 'set_formation':
            formation_type = data.get('formation_type', 1)
            scale = data.get('scale', 1.0)
            await self.set_swarm_formation(formation_type, scale)
            await websocket.send(json.dumps({
                'type': 'response',
                'status': 'success',
                'message': f'Formation set to type {formation_type}'
            }))
        elif command == 'assign_exploration_zone':
            zone_data = data.get('zone_data', {})
            target_bot = data.get('target_bot')
            await self.assign_exploration_zone(target_bot, zone_data)
            await websocket.send(json.dumps({
                'type': 'response',
                'status': 'success',
                'message': f'Exploration zone assigned to {target_bot}'
            }))
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
    
    # ═══════════════════════════════════════════════════════════
    # 🌊 SWARM INTELLIGENCE COMMAND METHODS
    # ═══════════════════════════════════════════════════════════
    
    async def send_swarm_status(self, websocket):
        """Send comprehensive swarm status to PC"""
        swarm_summary = self.swarm_monitor.get_swarm_summary()
        
        response = {
            'type': 'swarm_status',
            'timestamp': time.time(),
            'swarm_data': swarm_summary,
            'connected_bots': len([bot for bot in self.bot_status.values() if bot.is_online]),
            'total_bots_seen': len(self.bot_status)
        }
        
        await websocket.send(json.dumps(response))
        logger.debug("📊 Swarm status sent to PC")
    
    async def send_swarm_analysis(self, websocket):
        """Send detailed swarm intelligence analysis"""
        recent_messages = list(self.swarm_monitor.swarm_messages)[-50:]  # Last 50 messages
        
        analysis = {
            'type': 'swarm_analysis',
            'timestamp': time.time(),
            'recent_swarm_messages': recent_messages,
            'peer_relationships': dict(self.swarm_monitor.peer_relationships),
            'behavior_patterns': dict(self.swarm_monitor.behavior_patterns),
            'task_assignments': dict(self.swarm_monitor.task_assignments),
            'formation_history': list(self.swarm_monitor.formation_history)[-10:],  # Last 10 formations
            'swarm_state': asdict(self.swarm_monitor.swarm_state)
        }
        
        await websocket.send(json.dumps(analysis))
        logger.debug("🧠 Swarm analysis sent to PC")
    
    async def trigger_swarm_leader_election(self):
        """Trigger leader election across the swarm"""
        election_command = {
            'type': 'swarm_command',
            'command': 'leader_election',
            'timestamp': time.time(),
            'criteria': 'fitness'  # Election based on fitness
        }
        
        # Send to all connected bots
        for bot_id, websocket in self.bot_clients.items():
            try:
                await websocket.send(json.dumps(election_command))
                logger.info(f"🗳️ Leader election command sent to {bot_id}")
            except Exception as e:
                logger.error(f"Failed to send election command to {bot_id}: {e}")
        
        # Update swarm state
        self.swarm_monitor.swarm_state.leadership_election_active = True
        self.swarm_monitor.swarm_state.leader_bot = None
    
    async def set_swarm_formation(self, formation_type: int, scale: float):
        """Set swarm formation across all bots"""
        formation_command = {
            'type': 'swarm_command',
            'command': 'set_formation',
            'timestamp': time.time(),
            'formation_type': formation_type,
            'scale': scale
        }
        
        # Send to all connected bots
        for bot_id, websocket in self.bot_clients.items():
            try:
                await websocket.send(json.dumps(formation_command))
                logger.info(f"🔄 Formation command sent to {bot_id}: Type={formation_type}")
            except Exception as e:
                logger.error(f"Failed to send formation command to {bot_id}: {e}")
    
    async def assign_exploration_zone(self, target_bot: str, zone_data: Dict):
        """Assign exploration zone to specific bot"""
        if target_bot not in self.bot_clients:
            logger.warning(f"Target bot {target_bot} not connected")
            return
        
        assignment_command = {
            'type': 'swarm_command',
            'command': 'assign_zone',
            'timestamp': time.time(),
            'zone_data': zone_data
        }
        
        try:
            await self.bot_clients[target_bot].send(json.dumps(assignment_command))
            logger.info(f"🗺️ Exploration zone assigned to {target_bot}")
            
            # Track in swarm monitor
            zone_info = {
                'bot_id': target_bot,
                'center_x': zone_data.get('center_x', 0),
                'center_y': zone_data.get('center_y', 0),
                'width': zone_data.get('width', 5),
                'height': zone_data.get('height', 5),
                'strategy': zone_data.get('strategy', 1),
                'assigned_time': time.time()
            }
            self.swarm_monitor.swarm_state.exploration_zones.append(zone_info)
            
        except Exception as e:
            logger.error(f"Failed to assign zone to {target_bot}: {e}")
    
    async def broadcast_swarm_emergency(self, emergency_type: int, location: tuple = (0, 0)):
        """Broadcast emergency to entire swarm"""
        emergency_command = {
            'type': 'swarm_command',
            'command': 'emergency',
            'timestamp': time.time(),
            'emergency_type': emergency_type,
            'location': {'x': location[0], 'y': location[1]},
            'priority': 10  # Maximum priority
        }
        
        # Send to all connected bots
        for bot_id, websocket in self.bot_clients.items():
            try:
                await websocket.send(json.dumps(emergency_command))
                logger.critical(f"🚨 Emergency broadcast sent to {bot_id}")
            except Exception as e:
                logger.error(f"Failed to send emergency to {bot_id}: {e}")
        
        # Send to PC clients as well
        emergency_notification = {
            'type': 'emergency_alert',
            'timestamp': time.time(),
            'emergency_type': emergency_type,
            'location': location,
            'affected_bots': list(self.bot_clients.keys())
        }
        
        for pc_client in self.pc_clients:
            try:
                await pc_client.send(json.dumps(emergency_notification))
            except Exception as e:
                logger.error(f"Failed to notify PC of emergency: {e}")
    
    def get_swarm_metrics(self) -> Dict:
        """Get comprehensive swarm performance metrics"""
        current_time = time.time()
        uptime = current_time - self.stats['uptime_start']
        
        # Calculate additional metrics
        active_bots = len([bot for bot in self.bot_status.values() if bot.is_online])
        
        metrics = {
            'uptime_seconds': uptime,
            'total_messages_processed': self.stats['messages_processed'],
            'swarm_messages_analyzed': self.stats['swarm_messages_analyzed'],
            'message_processing_rate': self.stats['messages_processed'] / max(uptime, 1),
            'active_bots': active_bots,
            'swarm_coherence': self.swarm_monitor.swarm_state.swarm_coherence,
            'current_leader': self.swarm_monitor.swarm_state.leader_bot,
            'active_tasks': len(self.swarm_monitor.swarm_state.active_tasks),
            'exploration_zones': len(self.swarm_monitor.swarm_state.exploration_zones),
            'behavior_patterns_detected': len(self.swarm_monitor.behavior_patterns),
            'peer_relationships': len(self.swarm_monitor.peer_relationships)
        }
        
        return metrics

if __name__ == "__main__":
    # Create and run the Micro Bot server
    micro_bot = MicroBotServer()
    
    try:
        asyncio.run(micro_bot.start_server())
    except KeyboardInterrupt:
        logger.info("Server stopped by user")
    except Exception as e:
        logger.error(f"Fatal error: {e}")