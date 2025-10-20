#!/usr/bin/env python3
"""
🧬 Project Jumbo: Automated Swarm Testing Framework for ECS v2.0
Comprehensive testing and validation system for evolutionary swarm robotics

This framework provides automated test scenarios, performance benchmarking,
and validation protocols to ensure ECS evolution produces meaningful
improvements in swarm intelligence and coordination.

Key Features:
- Automated test scenario generation
- Performance benchmarking and validation
- Multi-bot coordination testing
- Evolution result verification
- Statistical analysis and reporting
- Real-time visualization and monitoring
"""

import asyncio
import websockets
import json
import time
import logging
import statistics
import matplotlib.pyplot as plt
import numpy as np
from datetime import datetime, timedelta
from typing import Dict, List, Optional, Any, Tuple
from dataclasses import dataclass, asdict
from collections import defaultdict, deque
from enum import Enum
import threading
import queue

# Set up logging
logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s - %(name)s - %(levelname)s - %(message)s'
)
logger = logging.getLogger('SwarmTestFramework')

class TestScenario(Enum):
    """Available test scenarios for swarm validation"""
    OBSTACLE_NAVIGATION = "obstacle_navigation"
    FORMATION_MAINTENANCE = "formation_maintenance"
    TASK_COORDINATION = "task_coordination"
    COMMUNICATION_EFFICIENCY = "communication_efficiency"
    ENERGY_OPTIMIZATION = "energy_optimization"
    ADAPTIVE_BEHAVIOR = "adaptive_behavior"
    EMERGENCY_RESPONSE = "emergency_response"
    COLLECTIVE_DECISION = "collective_decision"
    EXPLORATION_COVERAGE = "exploration_coverage"
    FAULT_TOLERANCE = "fault_tolerance"

class TestPhase(Enum):
    """Phases of test execution"""
    SETUP = "setup"
    BASELINE = "baseline"
    INTERVENTION = "intervention"
    MEASUREMENT = "measurement"
    ANALYSIS = "analysis"
    COMPLETE = "complete"

@dataclass
class TestResult:
    """Result of a single test scenario"""
    scenario: TestScenario
    bot_id: str
    start_time: float
    end_time: float
    success: bool
    metrics: Dict[str, float]
    errors: List[str]
    fitness_before: float
    fitness_after: float
    evolution_triggered: bool
    mutations_applied: int
    raw_data: Dict = None

@dataclass
class SwarmTestSession:
    """Complete test session with multiple scenarios"""
    session_id: str
    start_time: float
    scenarios_completed: int = 0
    scenarios_total: int = 0
    overall_success_rate: float = 0.0
    fitness_improvement: float = 0.0
    bots_tested: List[str] = None
    test_results: List[TestResult] = None
    
    def __post_init__(self):
        if self.bots_tested is None:
            self.bots_tested = []
        if self.test_results is None:
            self.test_results = []

class SwarmTestingFramework:
    """
    Automated testing framework for evolutionary swarm robotics
    
    Provides comprehensive test scenarios to validate ECS performance,
    measure improvements, and ensure evolution produces meaningful
    behavioral enhancements.
    """
    
    def __init__(self, bridge_host: str = "localhost", bridge_port: int = 8766):
        self.bridge_host = bridge_host
        self.bridge_port = bridge_port
        self.websocket = None
        self.connected = False
        
        # Test management
        self.active_tests: Dict[str, SwarmTestSession] = {}
        self.test_queue = queue.Queue()
        self.running = False
        
        # Bot tracking
        self.connected_bots: Dict[str, Dict] = {}
        self.bot_baselines: Dict[str, Dict] = {}
        
        # Results storage
        self.test_history: List[SwarmTestSession] = []
        self.performance_trends: Dict[str, List] = defaultdict(list)
        
        # Test configurations
        self.test_scenarios = self._initialize_test_scenarios()
        
        logger.info("🧪 Swarm Testing Framework initialized")
    
    def _initialize_test_scenarios(self) -> Dict[TestScenario, Dict]:
        """Initialize test scenario configurations"""
        return {
            TestScenario.OBSTACLE_NAVIGATION: {
                "duration": 300,  # 5 minutes
                "success_criteria": {
                    "collision_rate": {"max": 0.1},
                    "completion_rate": {"min": 0.8},
                    "navigation_efficiency": {"min": 0.7}
                },
                "metrics": ["collision_avoidance", "movement_efficiency", "task_completion"],
                "description": "Navigate through obstacle course without collisions"
            },
            
            TestScenario.FORMATION_MAINTENANCE: {
                "duration": 240,  # 4 minutes
                "success_criteria": {
                    "formation_stability": {"min": 0.8},
                    "coordination_accuracy": {"min": 0.9},
                    "communication_latency": {"max": 500}  # ms
                },
                "metrics": ["cooperation_score", "communication_success", "formation_stability"],
                "description": "Maintain coordinated formation while moving"
            },
            
            TestScenario.TASK_COORDINATION: {
                "duration": 360,  # 6 minutes
                "success_criteria": {
                    "task_distribution_efficiency": {"min": 0.7},
                    "completion_synchronization": {"min": 0.8},
                    "resource_utilization": {"min": 0.6}
                },
                "metrics": ["cooperation_score", "task_completion", "energy_efficiency"],
                "description": "Coordinate multi-bot task completion"
            },
            
            TestScenario.COMMUNICATION_EFFICIENCY: {
                "duration": 180,  # 3 minutes
                "success_criteria": {
                    "message_delivery_rate": {"min": 0.95},
                    "signal_clarity": {"min": 0.8},
                    "vocabulary_efficiency": {"min": 0.6}
                },
                "metrics": ["communication_success", "adaptation_speed"],
                "description": "Test emergent communication effectiveness"
            },
            
            TestScenario.ENERGY_OPTIMIZATION: {
                "duration": 450,  # 7.5 minutes
                "success_criteria": {
                    "energy_efficiency": {"min": 0.8},
                    "movement_optimization": {"min": 0.7},
                    "idle_time_ratio": {"max": 0.2}
                },
                "metrics": ["energy_efficiency", "movement_efficiency"],
                "description": "Optimize energy usage during tasks"
            },
            
            TestScenario.ADAPTIVE_BEHAVIOR: {
                "duration": 600,  # 10 minutes
                "success_criteria": {
                    "adaptation_speed": {"min": 0.7},
                    "behavioral_flexibility": {"min": 0.6},
                    "learning_retention": {"min": 0.8}
                },
                "metrics": ["adaptation_speed", "task_completion", "cooperation_score"],
                "description": "Adapt behavior to changing conditions"
            }
        }
    
    async def connect_to_bridge(self) -> bool:
        """Connect to the ECS bridge server"""
        try:
            self.websocket = await websockets.connect(f"ws://{self.bridge_host}:{self.bridge_port}")
            self.connected = True
            
            # Request initial status
            await self.websocket.send(json.dumps({
                "type": "command",
                "command": "get_status"
            }))
            
            logger.info(f"🔗 Connected to ECS bridge at {self.bridge_host}:{self.bridge_port}")
            return True
            
        except Exception as e:
            logger.error(f"❌ Failed to connect to bridge: {e}")
            return False
    
    async def start_testing_framework(self):
        """Start the automated testing framework"""
        if not await self.connect_to_bridge():
            return
        
        self.running = True
        logger.info("🧪 Starting automated swarm testing framework")
        
        # Start background tasks
        await asyncio.gather(
            self.message_handler(),
            self.test_scheduler(),
            self.performance_monitor(),
            self.results_analyzer()
        )
    
    async def message_handler(self):
        """Handle messages from the ECS bridge"""
        try:
            async for message in self.websocket:
                data = json.loads(message)
                message_type = data.get('type')
                
                if message_type == 'enhanced_status':
                    await self.handle_status_update(data)
                elif message_type == 'bot_message':
                    await self.handle_bot_message(data)
                elif message_type == 'evolution_triggered':
                    await self.handle_evolution_event(data)
                elif message_type == 'performance_report':
                    await self.handle_performance_report(data)
                    
        except websockets.exceptions.ConnectionClosed:
            logger.warning("🔌 Connection to bridge lost")
            self.connected = False
        except Exception as e:
            logger.error(f"Error in message handler: {e}")
    
    async def handle_status_update(self, data: Dict):
        """Handle status updates from bridge"""
        bots = data.get('bots', [])
        
        for bot_data in bots:
            bot_id = bot_data['bot_id']
            if bot_data['is_online']:
                self.connected_bots[bot_id] = bot_data
                
                # Record baseline if this is a new bot
                if bot_id not in self.bot_baselines:
                    self.bot_baselines[bot_id] = {
                        'fitness': bot_data.get('fitness_score', 0.0),
                        'parameters': bot_data.get('parameters', {}),
                        'timestamp': time.time()
                    }
        
        logger.info(f"📊 Status update: {len(self.connected_bots)} bots online")
    
    async def handle_bot_message(self, data: Dict):
        """Handle individual bot messages"""
        bot_id = data.get('bot_id')
        if bot_id in self.connected_bots:
            # Update bot data
            self.connected_bots[bot_id].update(data.get('data', {}))
    
    async def handle_evolution_event(self, data: Dict):
        """Handle evolution events"""
        bot_id = data.get('bot_id')
        logger.info(f"🧬 Evolution event for {bot_id}")
        
        # If we have active tests for this bot, update them
        for session in self.active_tests.values():
            for result in session.test_results:
                if result.bot_id == bot_id and result.end_time == 0:
                    result.evolution_triggered = True
    
    async def test_scheduler(self):
        """Schedule and run automated tests"""
        while self.running:
            await asyncio.sleep(30)  # Check every 30 seconds
            
            # Check if we should run tests
            if len(self.connected_bots) >= 1 and not self.active_tests:
                await self.schedule_test_session()
    
    async def schedule_test_session(self):
        """Schedule a comprehensive test session"""
        session_id = f"session_{int(time.time())}"
        
        # Select scenarios based on connected bots
        scenarios_to_run = []
        if len(self.connected_bots) >= 1:
            scenarios_to_run.extend([
                TestScenario.OBSTACLE_NAVIGATION,
                TestScenario.COMMUNICATION_EFFICIENCY,
                TestScenario.ENERGY_OPTIMIZATION,
                TestScenario.ADAPTIVE_BEHAVIOR
            ])
        
        if len(self.connected_bots) >= 2:
            scenarios_to_run.extend([
                TestScenario.FORMATION_MAINTENANCE,
                TestScenario.TASK_COORDINATION
            ])
        
        # Create test session
        session = SwarmTestSession(
            session_id=session_id,
            start_time=time.time(),
            scenarios_total=len(scenarios_to_run),
            bots_tested=list(self.connected_bots.keys())
        )
        
        self.active_tests[session_id] = session
        
        logger.info(f"🧪 Scheduled test session {session_id} with {len(scenarios_to_run)} scenarios")
        
        # Run scenarios
        for scenario in scenarios_to_run:
            await self.run_test_scenario(session, scenario)
            await asyncio.sleep(60)  # Rest between tests
        
        # Complete session
        await self.complete_test_session(session)
    
    async def run_test_scenario(self, session: SwarmTestSession, scenario: TestScenario):
        """Run a specific test scenario"""
        config = self.test_scenarios[scenario]
        
        logger.info(f"🎯 Starting test scenario: {scenario.value}")
        
        for bot_id in session.bots_tested:
            if bot_id not in self.connected_bots:
                continue
                
            # Create test result
            test_result = TestResult(
                scenario=scenario,
                bot_id=bot_id,
                start_time=time.time(),
                end_time=0,
                success=False,
                metrics={},
                errors=[],
                fitness_before=self.connected_bots[bot_id].get('fitness_score', 0.0),
                fitness_after=0.0,
                evolution_triggered=False,
                mutations_applied=0
            )
            
            # Record baseline metrics
            await self.record_baseline_metrics(test_result)
            
            # Run test phases
            await self.execute_test_phases(test_result, config)
            
            # Finalize result
            test_result.end_time = time.time()
            test_result.fitness_after = self.connected_bots[bot_id].get('fitness_score', 0.0)
            
            # Evaluate success
            test_result.success = self.evaluate_test_success(test_result, config)
            
            session.test_results.append(test_result)
            
            logger.info(f"✅ Test completed for {bot_id}: {scenario.value} - {'PASS' if test_result.success else 'FAIL'}")
    
    async def record_baseline_metrics(self, test_result: TestResult):
        """Record baseline performance metrics before test"""
        bot_data = self.connected_bots[test_result.bot_id]
        
        baseline_metrics = {
            'fitness_score': bot_data.get('fitness_score', 0.0),
            'task_completion_rate': bot_data.get('performance_metrics', {}).get('task_completion', 0.5),
            'energy_efficiency': bot_data.get('performance_metrics', {}).get('energy_efficiency', 0.5),
            'communication_success': bot_data.get('performance_metrics', {}).get('communication_success', 0.5),
            'collision_avoidance': bot_data.get('performance_metrics', {}).get('collision_avoidance', 0.5),
            'cooperation_score': bot_data.get('performance_metrics', {}).get('cooperation_score', 0.5)
        }
        
        test_result.metrics.update(baseline_metrics)
    
    async def execute_test_phases(self, test_result: TestResult, config: Dict):
        """Execute all phases of a test scenario"""
        duration = config['duration']
        
        # Phase 1: Setup (10% of duration)
        setup_time = duration * 0.1
        await self.execute_setup_phase(test_result, setup_time)
        
        # Phase 2: Baseline measurement (20% of duration)  
        baseline_time = duration * 0.2
        await self.execute_baseline_phase(test_result, baseline_time)
        
        # Phase 3: Active testing (60% of duration)
        test_time = duration * 0.6
        await self.execute_active_phase(test_result, test_time)
        
        # Phase 4: Measurement (10% of duration)
        measurement_time = duration * 0.1
        await self.execute_measurement_phase(test_result, measurement_time)
    
    async def execute_setup_phase(self, test_result: TestResult, duration: float):
        """Setup phase - prepare bot for testing"""
        logger.info(f"⚙️  Setup phase for {test_result.bot_id}: {test_result.scenario.value}")
        
        # Send setup commands to bot
        setup_command = {
            "type": "bot_command",
            "bot_id": test_result.bot_id,
            "command": "prepare_test",
            "parameters": {
                "scenario": test_result.scenario.value,
                "duration": duration
            }
        }
        
        if self.connected:
            await self.websocket.send(json.dumps(setup_command))
        
        await asyncio.sleep(duration)
    
    async def execute_baseline_phase(self, test_result: TestResult, duration: float):
        """Baseline phase - measure initial performance"""
        logger.info(f"📏 Baseline phase for {test_result.bot_id}")
        
        start_metrics = dict(test_result.metrics)
        await asyncio.sleep(duration)
        
        # Record any changes during baseline
        await self.record_baseline_metrics(test_result)
    
    async def execute_active_phase(self, test_result: TestResult, duration: float):
        """Active testing phase - main test execution"""
        logger.info(f"🎯 Active testing phase for {test_result.bot_id}")
        
        # Send test commands based on scenario
        test_commands = self.generate_scenario_commands(test_result.scenario, test_result.bot_id)
        
        for command in test_commands:
            if self.connected:
                await self.websocket.send(json.dumps(command))
            await asyncio.sleep(duration / len(test_commands))
    
    async def execute_measurement_phase(self, test_result: TestResult, duration: float):
        """Measurement phase - collect final metrics"""
        logger.info(f"📊 Measurement phase for {test_result.bot_id}")
        
        # Request final performance report
        if self.connected:
            await self.websocket.send(json.dumps({
                "type": "command",
                "command": "get_status"
            }))
        
        await asyncio.sleep(duration)
    
    def generate_scenario_commands(self, scenario: TestScenario, bot_id: str) -> List[Dict]:
        """Generate scenario-specific test commands"""
        commands = []
        
        if scenario == TestScenario.OBSTACLE_NAVIGATION:
            commands = [
                {
                    "type": "bot_command",
                    "bot_id": bot_id,
                    "command": "navigate_obstacles",
                    "parameters": {"complexity": "medium"}
                },
                {
                    "type": "bot_command", 
                    "bot_id": bot_id,
                    "command": "increase_speed",
                    "parameters": {"factor": 1.2}
                }
            ]
        
        elif scenario == TestScenario.COMMUNICATION_EFFICIENCY:
            commands = [
                {
                    "type": "bot_command",
                    "bot_id": bot_id,
                    "command": "test_communication",
                    "parameters": {"message_rate": "high"}
                }
            ]
        
        elif scenario == TestScenario.ADAPTIVE_BEHAVIOR:
            commands = [
                {
                    "type": "bot_command",
                    "bot_id": bot_id,
                    "command": "change_environment",
                    "parameters": {"difficulty": "increasing"}
                },
                {
                    "type": "evolution_command",
                    "command": "trigger_evolution",
                    "bot_id": bot_id
                }
            ]
        
        return commands
    
    def evaluate_test_success(self, test_result: TestResult, config: Dict) -> bool:
        """Evaluate if test passed based on success criteria"""
        success_criteria = config.get('success_criteria', {})
        
        for metric_name, criteria in success_criteria.items():
            metric_value = test_result.metrics.get(metric_name, 0.0)
            
            if 'min' in criteria and metric_value < criteria['min']:
                test_result.errors.append(f"{metric_name} below minimum: {metric_value} < {criteria['min']}")
                return False
            
            if 'max' in criteria and metric_value > criteria['max']:
                test_result.errors.append(f"{metric_name} above maximum: {metric_value} > {criteria['max']}")
                return False
        
        return True
    
    async def complete_test_session(self, session: SwarmTestSession):
        """Complete and analyze test session"""
        session.scenarios_completed = len(session.test_results)
        
        # Calculate overall success rate
        successful_tests = sum(1 for result in session.test_results if result.success)
        session.overall_success_rate = successful_tests / max(1, len(session.test_results))
        
        # Calculate fitness improvement
        fitness_deltas = []
        for result in session.test_results:
            if result.fitness_after > 0:
                delta = result.fitness_after - result.fitness_before
                fitness_deltas.append(delta)
        
        if fitness_deltas:
            session.fitness_improvement = statistics.mean(fitness_deltas)
        
        # Store in history
        self.test_history.append(session)
        
        # Remove from active tests
        del self.active_tests[session.session_id]
        
        # Generate report
        await self.generate_test_report(session)
        
        logger.info(f"🎉 Test session {session.session_id} completed: "
                   f"{session.overall_success_rate:.1%} success rate, "
                   f"fitness Δ: {session.fitness_improvement:.3f}")
    
    async def generate_test_report(self, session: SwarmTestSession):
        """Generate comprehensive test report"""
        report = {
            "session_id": session.session_id,
            "timestamp": datetime.fromtimestamp(session.start_time).isoformat(),
            "duration": time.time() - session.start_time,
            "scenarios_completed": session.scenarios_completed,
            "overall_success_rate": session.overall_success_rate,
            "fitness_improvement": session.fitness_improvement,
            "bots_tested": session.bots_tested,
            "detailed_results": []
        }
        
        for result in session.test_results:
            result_dict = {
                "scenario": result.scenario.value,
                "bot_id": result.bot_id,
                "success": result.success,
                "duration": result.end_time - result.start_time,
                "fitness_delta": result.fitness_after - result.fitness_before,
                "evolution_triggered": result.evolution_triggered,
                "mutations_applied": result.mutations_applied,
                "metrics": result.metrics,
                "errors": result.errors
            }
            report["detailed_results"].append(result_dict)
        
        # Save report to file
        filename = f"test_report_{session.session_id}.json"
        with open(filename, 'w') as f:
            json.dump(report, f, indent=2)
        
        logger.info(f"📋 Test report saved: {filename}")
    
    async def performance_monitor(self):
        """Monitor performance trends"""
        while self.running:
            await asyncio.sleep(60)  # Check every minute
            
            for bot_id, bot_data in self.connected_bots.items():
                fitness = bot_data.get('fitness_score', 0.0)
                self.performance_trends[bot_id].append({
                    'timestamp': time.time(),
                    'fitness': fitness,
                    'generation': bot_data.get('generation', 0)
                })
                
                # Keep only last 100 readings per bot
                if len(self.performance_trends[bot_id]) > 100:
                    self.performance_trends[bot_id].pop(0)
    
    async def results_analyzer(self):
        """Analyze results and generate insights"""
        while self.running:
            await asyncio.sleep(300)  # Analyze every 5 minutes
            
            if len(self.test_history) >= 2:
                await self.analyze_evolution_effectiveness()
                await self.detect_performance_patterns()
    
    async def analyze_evolution_effectiveness(self):
        """Analyze effectiveness of evolution over time"""
        recent_sessions = self.test_history[-5:]  # Last 5 sessions
        
        fitness_improvements = [s.fitness_improvement for s in recent_sessions if s.fitness_improvement > 0]
        success_rates = [s.overall_success_rate for s in recent_sessions]
        
        if fitness_improvements:
            avg_improvement = statistics.mean(fitness_improvements)
            avg_success = statistics.mean(success_rates)
            
            logger.info(f"📈 Evolution effectiveness: "
                       f"avg fitness improvement: {avg_improvement:.3f}, "
                       f"avg success rate: {avg_success:.1%}")
    
    async def detect_performance_patterns(self):
        """Detect patterns in performance data"""
        for bot_id, trend_data in self.performance_trends.items():
            if len(trend_data) >= 10:
                fitness_values = [d['fitness'] for d in trend_data[-10:]]
                
                # Check for improvement trend
                if len(fitness_values) >= 2:
                    slope = (fitness_values[-1] - fitness_values[0]) / len(fitness_values)
                    
                    if slope > 0.01:
                        logger.info(f"📊 {bot_id} showing positive fitness trend: +{slope:.3f}/reading")
                    elif slope < -0.01:
                        logger.warning(f"📉 {bot_id} showing negative fitness trend: {slope:.3f}/reading")

# Testing utilities
class TestResultVisualizer:
    """Visualize test results and performance trends"""
    
    @staticmethod
    def plot_fitness_trends(performance_trends: Dict[str, List]) -> str:
        """Create fitness trend plots"""
        plt.figure(figsize=(12, 8))
        
        for bot_id, trend_data in performance_trends.items():
            if len(trend_data) >= 2:
                timestamps = [d['timestamp'] for d in trend_data]
                fitness_values = [d['fitness'] for d in trend_data]
                
                # Convert timestamps to minutes from start
                start_time = min(timestamps)
                time_minutes = [(t - start_time) / 60 for t in timestamps]
                
                plt.plot(time_minutes, fitness_values, label=f'{bot_id}', marker='o')
        
        plt.xlabel('Time (minutes)')
        plt.ylabel('Fitness Score')
        plt.title('Bot Fitness Evolution Over Time')
        plt.legend()
        plt.grid(True)
        
        filename = f'fitness_trends_{int(time.time())}.png'
        plt.savefig(filename)
        plt.close()
        
        return filename
    
    @staticmethod
    def plot_test_results(test_history: List[SwarmTestSession]) -> str:
        """Create test results summary plot"""
        if not test_history:
            return ""
        
        fig, (ax1, ax2) = plt.subplots(2, 1, figsize=(12, 10))
        
        # Success rates over time
        session_times = [s.start_time for s in test_history]
        success_rates = [s.overall_success_rate for s in test_history]
        
        ax1.plot(session_times, success_rates, 'b-o')
        ax1.set_ylabel('Success Rate')
        ax1.set_title('Test Success Rate Over Time')
        ax1.grid(True)
        
        # Fitness improvements over time
        fitness_improvements = [s.fitness_improvement for s in test_history]
        
        ax2.plot(session_times, fitness_improvements, 'g-o')
        ax2.set_ylabel('Fitness Improvement')
        ax2.set_xlabel('Session Time')
        ax2.set_title('Fitness Improvement Over Time')
        ax2.grid(True)
        
        plt.tight_layout()
        
        filename = f'test_results_summary_{int(time.time())}.png'
        plt.savefig(filename)
        plt.close()
        
        return filename

# Example usage
async def main():
    """Example usage of the testing framework"""
    # Initialize testing framework
    framework = SwarmTestingFramework(bridge_host="192.168.1.207", bridge_port=8766)
    
    try:
        # Start automated testing
        await framework.start_testing_framework()
        
    except KeyboardInterrupt:
        logger.info("🛑 Testing framework shutting down...")
        framework.running = False
        
        # Generate final reports
        if framework.test_history:
            visualizer = TestResultVisualizer()
            fitness_plot = visualizer.plot_fitness_trends(framework.performance_trends)
            results_plot = visualizer.plot_test_results(framework.test_history)
            
            logger.info(f"📊 Visualization saved: {fitness_plot}, {results_plot}")

if __name__ == "__main__":
    asyncio.run(main())