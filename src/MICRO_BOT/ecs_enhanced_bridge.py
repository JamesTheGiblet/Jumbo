#!/usr/bin/env python3
"""
🧬 Project Jumbo: Enhanced MICRO_BOT Bridge with ECS v2.0 Integration
Real ESP32 WHEELIE/SPEEDIE bot communication bridge with Evolutionary Code System

This enhanced bridge integrates the Evolutionary Code System (ECS) to enable:
- Real-time bot evolution and parameter optimization
- Performance monitoring and fitness evaluation
- Intelligent mutation selection and application
- Swarm behavior analysis and optimization
"""

import asyncio
import websockets
import json
import time
import logging
import sys
import os
from datetime import datetime
from typing import Dict, List, Optional, Any
from dataclasses import dataclass, asdict
from collections import defaultdict, deque

# Add the src directory to the path to import ECS
sys.path.append(os.path.join(os.path.dirname(__file__), '..'))
from evolutionary_code_system import EvolutionaryCodeSystem, MutationType

# Set up logging
logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s - %(name)s - %(levelname)s - %(message)s'
)
logger = logging.getLogger('ECSBridge')

@dataclass
class EnhancedBotStatus:
    """Enhanced status information for a bot with ECS integration"""
    bot_id: str
    is_online: bool = False
    ip_address: str = ""
    
    # Evolution data
    generation: int = 0
    fitness_score: float = 0.0
    parameters: Dict = None
    mutation_history: List = None
    
    # Performance metrics
    performance_metrics: Dict = None
    error_log: List = None
    
    # Standard tracking
    last_seen: Optional[datetime] = None
    message_count: int = 0
    emotional_state: Dict = None
    sensor_data: Dict = None
    capabilities: List = None
    
    # ECS-specific data
    evolution_requests: int = 0
    successful_mutations: int = 0
    fitness_trend: List = None
    last_evolution: Optional[datetime] = None
    
    def __post_init__(self):
        if self.parameters is None:
            self.parameters = {}
        if self.mutation_history is None:
            self.mutation_history = []
        if self.performance_metrics is None:
            self.performance_metrics = {}
        if self.error_log is None:
            self.error_log = []
        if self.emotional_state is None:
            self.emotional_state = {}
        if self.sensor_data is None:
            self.sensor_data = {}
        if self.capabilities is None:
            self.capabilities = []
        if self.fitness_trend is None:
            self.fitness_trend = []

class ECSEnhancedBridge:
    """ESP-NOW Bridge with Evolutionary Code System Integration"""
    
    def __init__(self):
        self.pc_clients = set()
        self.bot_clients = set()
        self.bot_status: Dict[str, EnhancedBotStatus] = {}
        self.running = True
        
        # Initialize ECS
        self.ecs = EvolutionaryCodeSystem("bridge_evolution_corpus.json")
        logger.info("🧬 ECS v2.0 initialized")
        
        # Evolution tracking
        self.evolution_sessions: Dict[str, Dict] = {}
        self.swarm_metrics = {
            'coordination_efficiency': 0.0,
            'collective_task_completion': 0.0,
            'communication_clarity': 0.0,
            'formation_stability': 0.0
        }
        
        # Statistics
        self.stats = {
            'messages_processed': 0,
            'messages_filtered': 0, 
            'messages_forwarded': 0,
            'uptime_start': time.time(),
            'evolution_requests': 0,
            'successful_evolutions': 0,
            'mutations_applied': 0,
            'fitness_improvements': 0,
            'swarm_behaviors_detected': 0
        }
        
        logger.info("🤖 ECS Bridge initialized - Ready for evolutionary swarm robotics")
    
    async def handle_bot_connection(self, websocket):
        """Handle connections from ESP32 bots with ECS support"""
        bot_ip = websocket.remote_address[0]
        logger.info(f"🤖 ESP32 bot connected from {bot_ip}")
        
        try:
            self.bot_clients.add(websocket)
            
            async for message in websocket:
                try:
                    if message.startswith('{'):
                        # JSON message (includes ECS messages)
                        data = json.loads(message)
                        await self.process_enhanced_bot_message(data, bot_ip, websocket)
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
            await self.mark_bot_offline(bot_ip)
    
    async def process_enhanced_bot_message(self, data: Dict, bot_ip: str, websocket):
        """Process enhanced message from ESP32 bot with ECS support"""
        bot_id = data.get('bot_id', f"BOT_{bot_ip.replace('.', '_')}")
        message_type = data.get('type', 'unknown')
        
        # Update or create bot status
        if bot_id not in self.bot_status:
            self.bot_status[bot_id] = EnhancedBotStatus(
                bot_id=bot_id,
                ip_address=bot_ip,
                is_online=True
            )
            logger.info(f"🆕 New ECS-enabled bot registered: {bot_id}")
            
        bot = self.bot_status[bot_id]
        bot.is_online = True
        bot.last_seen = datetime.now()
        bot.message_count += 1
        
        # Handle different message types
        if message_type == 'performance_report':
            await self.handle_performance_report(bot, data)
        elif message_type == 'error_report':
            await self.handle_error_report(bot, data)
        elif message_type == 'evolution_request':
            await self.handle_evolution_request(bot, data, websocket)
        elif message_type == 'evolution_result':
            await self.handle_evolution_result(bot, data)
        elif message_type == 'heartbeat':
            await self.handle_heartbeat(bot, data)
        elif message_type == 'parameter_update':
            await self.handle_parameter_update(bot, data)
        else:
            # Standard bot message
            await self.handle_standard_message(bot, data)
            
        # Forward to PC clients
        await self.notify_pc_clients('bot_message', {
            'bot_id': bot_id,
            'type': message_type,
            'data': data,
            'timestamp': time.time()
        })
    
    async def handle_performance_report(self, bot: EnhancedBotStatus, data: Dict):
        """Handle performance metrics from bot"""
        if 'metrics' in data:
            bot.performance_metrics.update(data['metrics'])
            
        if 'fitness' in data:
            old_fitness = bot.fitness_score
            bot.fitness_score = data['fitness']
            bot.fitness_trend.append({
                'timestamp': time.time(),
                'fitness': bot.fitness_score
            })
            
            # Keep only last 50 fitness readings
            if len(bot.fitness_trend) > 50:
                bot.fitness_trend.pop(0)
                
            # Check for fitness improvement
            if bot.fitness_score > old_fitness:
                self.stats['fitness_improvements'] += 1
        
        if 'generation' in data:
            bot.generation = data['generation']
            
        if 'parameters' in data:
            bot.parameters.update(data['parameters'])
            
        logger.info(f"📊 Performance report from {bot.bot_id}: fitness={bot.fitness_score:.3f}, gen={bot.generation}")
    
    async def handle_error_report(self, bot: EnhancedBotStatus, data: Dict):
        """Handle error reports from bot"""
        if 'errors' in data:
            for error in data['errors']:
                bot.error_log.append({
                    'timestamp': time.time(),
                    'severity': error.get('severity', 'unknown'),
                    'code': error.get('code', 0),
                    'description': error.get('description', ''),
                    'function': error.get('function', '')
                })
            
            # Keep only last 100 errors
            if len(bot.error_log) > 100:
                bot.error_log = bot.error_log[-100:]
                
        logger.warning(f"🚨 Error report from {bot.bot_id}: {len(data.get('errors', []))} errors")
    
    async def handle_evolution_request(self, bot: EnhancedBotStatus, data: Dict, websocket):
        """Handle evolution request from bot"""
        bot.evolution_requests += 1
        self.stats['evolution_requests'] += 1
        
        trigger_reason = data.get('trigger', 'Unknown trigger')
        current_fitness = data.get('fitness', bot.fitness_score)
        
        logger.info(f"🧬 Evolution request from {bot.bot_id}: {trigger_reason} (fitness: {current_fitness:.3f})")
        
        # Extract performance metrics for ECS
        performance_metrics = {
            'task_completion_rate': bot.performance_metrics.get('task_completion', 0.5),
            'energy_efficiency': bot.performance_metrics.get('energy_efficiency', 0.5),
            'communication_effectiveness': bot.performance_metrics.get('communication_success', 0.5),
            'collision_avoidance': bot.performance_metrics.get('collision_avoidance', 0.5),
            'cooperation_score': bot.performance_metrics.get('cooperation_score', 0.5),
            'adaptation_speed': bot.performance_metrics.get('adaptation_speed', 0.5)
        }
        
        # Create error log for ECS
        error_log = [error['description'] for error in bot.error_log[-10:]]  # Last 10 errors
        
        # Request evolution from ECS
        evolution_result = self.ecs.evolve_bot_behavior(
            bot_id=bot.bot_id,
            error_log=error_log,
            performance_metrics=performance_metrics
        )
        
        # Apply successful mutations
        if evolution_result['mutation_results']:
            successful_mutations = [m for m in evolution_result['mutation_results'] if m.success]
            
            if successful_mutations:
                # Send parameter updates to bot
                for mutation in successful_mutations:
                    await self.apply_mutation_to_bot(bot, mutation, websocket)
                
                bot.successful_mutations += len(successful_mutations)
                bot.last_evolution = datetime.now()
                self.stats['successful_evolutions'] += 1
                
                logger.info(f"✅ Applied {len(successful_mutations)} mutations to {bot.bot_id}")
            else:
                logger.info(f"❌ No successful mutations for {bot.bot_id}")
        
        # Store evolution session
        self.evolution_sessions[f"{bot.bot_id}_{time.time()}"] = evolution_result
    
    async def apply_mutation_to_bot(self, bot: EnhancedBotStatus, mutation, websocket):
        """Apply a successful mutation to the bot"""
        # Extract parameter changes from the mutation
        # This would need to be customized based on your specific mutation format
        
        try:
            # Parse mutated code/parameters (simplified example)
            if hasattr(mutation, 'mutated_code'):
                # For now, we'll send a parameter update command
                # In a real implementation, you'd parse the mutated code for parameter changes
                
                # Example: send motor speed adjustment
                parameter_update = {
                    'type': 'parameter_update',
                    'parameter': 'motorSpeed',  # This would be extracted from mutation
                    'value': 180,  # This would be calculated from mutation
                    'mutation_type': mutation.mutation_type.value,
                    'generation': bot.generation + 1
                }
                
                await websocket.send(json.dumps(parameter_update))
                self.stats['mutations_applied'] += 1
                
                # Track mutation in bot history
                bot.mutation_history.append({
                    'timestamp': time.time(),
                    'type': mutation.mutation_type.value,
                    'fitness_before': mutation.fitness_before,
                    'fitness_after': mutation.fitness_after,
                    'success': mutation.success
                })
                
                logger.info(f"🔬 Mutation applied to {bot.bot_id}: {mutation.mutation_type.value}")
                
        except Exception as e:
            logger.error(f"Error applying mutation to {bot.bot_id}: {e}")
    
    async def handle_evolution_result(self, bot: EnhancedBotStatus, data: Dict):
        """Handle evolution result feedback from bot"""
        success = data.get('success', False)
        fitness_delta = data.get('fitness_delta', 0.0)
        
        if success:
            logger.info(f"✅ Evolution confirmed successful for {bot.bot_id}: Δ{fitness_delta:.3f}")
        else:
            logger.info(f"❌ Evolution failed for {bot.bot_id}: Δ{fitness_delta:.3f}")
    
    async def handle_heartbeat(self, bot: EnhancedBotStatus, data: Dict):
        """Handle heartbeat message from bot"""
        bot.is_online = True
        bot.last_seen = datetime.now()
        
        # Update basic stats
        if 'generation' in data:
            bot.generation = data['generation']
        if 'fitness' in data:
            bot.fitness_score = data['fitness']
        if 'free_heap' in data:
            bot.sensor_data['free_heap'] = data['free_heap']
    
    async def handle_parameter_update(self, bot: EnhancedBotStatus, data: Dict):
        """Handle parameter update notification from bot"""
        parameter_name = data.get('parameter')
        new_value = data.get('value')
        
        if parameter_name and new_value is not None:
            bot.parameters[parameter_name] = new_value
            logger.info(f"🔧 Parameter updated on {bot.bot_id}: {parameter_name} = {new_value}")
    
    async def handle_standard_message(self, bot: EnhancedBotStatus, data: Dict):
        """Handle standard bot messages (non-ECS)"""
        # Update standard bot data
        if 'generation' in data:
            bot.generation = data['generation']
        if 'fitness_score' in data:
            bot.fitness_score = data['fitness_score']
        if 'emotional_state' in data:
            bot.emotional_state.update(data['emotional_state'])
        if 'sensor_data' in data:
            bot.sensor_data.update(data['sensor_data'])
    
    async def mark_bot_offline(self, bot_ip: str):
        """Mark bot as offline and notify PC clients"""
        for bot_id, status in self.bot_status.items():
            if status.ip_address == bot_ip:
                status.is_online = False
                await self.notify_pc_clients('bot_offline', {'bot_id': bot_id})
                logger.info(f"📴 Bot {bot_id} marked offline")
                break
    
    async def handle_pc_connection(self, websocket):
        """Handle connections from PC applications with ECS features"""
        pc_ip = websocket.remote_address[0]
        
        try:
            logger.info(f"💻 PC client connected from {pc_ip}")
            self.pc_clients.add(websocket)
            
            # Send enhanced initial status
            await self.send_enhanced_status(websocket)
            
            async for message in websocket:
                try:
                    data = json.loads(message)
                    command = data.get('command', 'unknown')
                    logger.info(f"📥 PC command: {command}")
                    
                    # Handle PC commands (including ECS commands)
                    if data.get('type') == 'command':
                        await self.handle_enhanced_pc_command(data, websocket)
                    elif data.get('type') == 'bot_command':
                        await self.forward_to_bot(data)
                    elif data.get('type') == 'evolution_command':
                        await self.handle_evolution_command(data, websocket)
                        
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
    
    async def handle_enhanced_pc_command(self, data: Dict, websocket):
        """Handle enhanced PC commands including ECS functions"""
        command = data.get('command')
        
        if command == 'get_status':
            await self.send_enhanced_status(websocket)
        elif command == 'get_bots':
            await self.send_enhanced_bot_list(websocket)
        elif command == 'get_ecs_stats':
            await self.send_ecs_statistics(websocket)
        elif command == 'trigger_evolution':
            bot_id = data.get('bot_id')
            await self.trigger_manual_evolution(bot_id, websocket)
        elif command == 'reset_parameters':
            bot_id = data.get('bot_id')
            await self.reset_bot_parameters(bot_id, websocket)
        elif command == 'get_swarm_metrics':
            await self.send_swarm_metrics(websocket)
        elif command == 'ping':
            await websocket.send(json.dumps({
                'type': 'pong',
                'timestamp': time.time()
            }))
    
    async def handle_evolution_command(self, data: Dict, websocket):
        """Handle evolution-specific commands from PC"""
        command = data.get('command')
        bot_id = data.get('bot_id')
        
        if command == 'force_mutation':
            mutation_type = data.get('mutation_type')
            parameter = data.get('parameter')
            value = data.get('value')
            await self.force_bot_mutation(bot_id, mutation_type, parameter, value, websocket)
        elif command == 'get_evolution_history':
            await self.send_evolution_history(bot_id, websocket)
        elif command == 'analyze_fitness_trend':
            await self.analyze_fitness_trends(bot_id, websocket)
    
    async def send_enhanced_status(self, websocket):
        """Send enhanced status including ECS data"""
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
                    'evolution_requests': bot_status.evolution_requests,
                    'successful_mutations': bot_status.successful_mutations,
                    'last_evolution': bot_status.last_evolution.isoformat() if bot_status.last_evolution else None,
                    'parameters': bot_status.parameters,
                    'performance_metrics': bot_status.performance_metrics,
                    'error_count': len(bot_status.error_log),
                    'fitness_trend': bot_status.fitness_trend[-10:],  # Last 10 readings
                    'emotional_state': bot_status.emotional_state,
                    'sensor_data': bot_status.sensor_data,
                    'capabilities': bot_status.capabilities
                }
                bot_list.append(bot_dict)
            
            # Get ECS statistics
            ecs_stats = self.ecs.get_evolution_stats()
            
            status = {
                'type': 'enhanced_status',
                'timestamp': time.time(),
                'uptime': time.time() - self.stats['uptime_start'],
                'connected_bots': len([b for b in self.bot_status.values() if b.is_online]),
                'total_bots': len(self.bot_status),
                'connected_pc_clients': len(self.pc_clients),
                'bridge_stats': self.stats,
                'ecs_stats': ecs_stats,
                'swarm_metrics': self.swarm_metrics,
                'bots': bot_list
            }
            
            await websocket.send(json.dumps(status))
            logger.info(f"📤 Sent enhanced status: {len(bot_list)} bots, ECS gen {ecs_stats['generation']}")
            
        except Exception as e:
            logger.error(f"Error sending enhanced status: {e}")
    
    async def send_ecs_statistics(self, websocket):
        """Send detailed ECS statistics"""
        try:
            ecs_stats = self.ecs.get_evolution_stats()
            
            response = {
                'type': 'ecs_statistics',
                'timestamp': time.time(),
                'stats': ecs_stats,
                'evolution_sessions': len(self.evolution_sessions),
                'active_bots': len([b for b in self.bot_status.values() if b.is_online]),
                'total_fitness': sum(b.fitness_score for b in self.bot_status.values()),
                'average_fitness': sum(b.fitness_score for b in self.bot_status.values()) / max(1, len(self.bot_status))
            }
            
            await websocket.send(json.dumps(response))
            
        except Exception as e:
            logger.error(f"Error sending ECS statistics: {e}")
    
    async def trigger_manual_evolution(self, bot_id: str, websocket):
        """Trigger manual evolution for a specific bot"""
        if bot_id not in self.bot_status:
            await websocket.send(json.dumps({
                'type': 'error',
                'message': f'Bot {bot_id} not found'
            }))
            return
        
        bot = self.bot_status[bot_id]
        if not bot.is_online:
            await websocket.send(json.dumps({
                'type': 'error',
                'message': f'Bot {bot_id} is offline'
            }))
            return
        
        # Find bot websocket and send evolution trigger
        for bot_ws in self.bot_clients:
            if bot_ws.remote_address[0] == bot.ip_address:
                try:
                    await bot_ws.send(json.dumps({
                        'type': 'evolution_trigger',
                        'reason': 'Manual trigger from PC'
                    }))
                    
                    await websocket.send(json.dumps({
                        'type': 'evolution_triggered',
                        'bot_id': bot_id,
                        'timestamp': time.time()
                    }))
                    
                    logger.info(f"🧬 Manual evolution triggered for {bot_id}")
                    break
                    
                except Exception as e:
                    logger.error(f"Error triggering evolution for {bot_id}: {e}")
    
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
        """Start enhanced WebSocket servers with ECS support"""
        logger.info("🧬 ECS-Enhanced MICRO BOT Bridge Server Starting...")
        logger.info("📡 ESP32 Bot server (ECS-enabled): 0.0.0.0:8765")
        logger.info("💻 PC Client server (ECS-enhanced): 0.0.0.0:8766")
        logger.info("🎯 Ready for evolutionary swarm robotics")
        
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
        
        logger.info("🚀 ECS-Enhanced Bridge Server online!")
        
        # Run both servers
        await asyncio.gather(
            bot_server,
            pc_server,
            self.enhanced_stats_loop(),
            self.evolution_monitoring_loop()
        )
    
    async def enhanced_stats_loop(self):
        """Enhanced statistics loop with ECS metrics"""
        while self.running:
            await asyncio.sleep(30)
            online_bots = len([b for b in self.bot_status.values() if b.is_online])
            total_fitness = sum(b.fitness_score for b in self.bot_status.values())
            avg_fitness = total_fitness / max(1, len(self.bot_status))
            
            ecs_stats = self.ecs.get_evolution_stats()
            
            logger.info(
                f"📊 ECS Stats: {online_bots} bots online, "
                f"avg fitness: {avg_fitness:.3f}, "
                f"generation: {ecs_stats['generation']}, "
                f"evolution success rate: {ecs_stats['success_rate']:.1%}"
            )
    
    async def evolution_monitoring_loop(self):
        """Monitor bot evolution and trigger automatic evolution when needed"""
        while self.running:
            await asyncio.sleep(60)  # Check every minute
            
            for bot_id, bot in self.bot_status.items():
                if not bot.is_online:
                    continue
                
                # Check if bot needs evolution
                should_evolve = False
                evolution_reason = ""
                
                # Check fitness stagnation
                if len(bot.fitness_trend) >= 10:
                    recent_fitness = [f['fitness'] for f in bot.fitness_trend[-10:]]
                    if max(recent_fitness) - min(recent_fitness) < 0.05:  # Less than 5% variation
                        should_evolve = True
                        evolution_reason = "Fitness stagnation detected"
                
                # Check error rate
                recent_errors = [e for e in bot.error_log if time.time() - e['timestamp'] < 300]  # Last 5 minutes
                if len(recent_errors) > 10:  # More than 10 errors in 5 minutes
                    should_evolve = True
                    evolution_reason = "High error rate detected"
                
                # Trigger evolution if needed
                if should_evolve:
                    # Find bot websocket and send evolution trigger
                    for bot_ws in self.bot_clients:
                        if bot_ws.remote_address[0] == bot.ip_address:
                            try:
                                await bot_ws.send(json.dumps({
                                    'type': 'evolution_trigger',
                                    'reason': evolution_reason
                                }))
                                logger.info(f"🧬 Auto-evolution triggered for {bot_id}: {evolution_reason}")
                                break
                            except Exception as e:
                                logger.error(f"Error auto-triggering evolution for {bot_id}: {e}")

async def main():
    """Main entry point for ECS-enhanced bridge"""
    bridge = ECSEnhancedBridge()
    
    try:
        await bridge.start_servers()
    except KeyboardInterrupt:
        logger.info("🛑 Shutting down ECS bridge server...")
        bridge.running = False
        # Save ECS corpus before shutdown
        bridge.ecs.save_corpus()
    except Exception as e:
        logger.error(f"ECS bridge server error: {e}")

if __name__ == "__main__":
    asyncio.run(main())