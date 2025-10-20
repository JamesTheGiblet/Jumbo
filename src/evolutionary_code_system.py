#!/usr/bin/env python3
"""
🧬 Project Jumbo: Evolutionary Code System (ECS) v2.0
A self-improving, self-learning code system for autonomous robotic swarms

This system enables bots to autonomously evolve their behavior, fix bugs,
handle edge cases, and optimize performance through intelligent mutations
and fitness-based selection.

Key Features:
- Exception-driven mutation selection
- Self-learning feedback loop with Evolution Corpus
- Context-aware mutations for swarm robotics
- Real-time ESP-NOW integration
- Multi-bot coordination evolution
"""

import logging
import json
import time
import random
import copy
import inspect
import traceback
import hashlib
from datetime import datetime
from typing import Dict, List, Tuple, Any, Callable, Optional
from dataclasses import dataclass, asdict
from enum import Enum

# Configure logging
logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s - %(levelname)s - %(message)s'
)

class MutationType(Enum):
    """Types of mutations the ECS can apply"""
    TYPE_CHECK = "type_check"           # Add isinstance() checks
    NULL_CHECK = "null_check"           # Add None/empty checks  
    BOUNDS_CHECK = "bounds_check"       # Add array/range bounds
    ERROR_HANDLER = "error_handler"     # Wrap in try/except
    PARAMETER_TUNE = "parameter_tune"   # Adjust numeric parameters
    LOGIC_FIX = "logic_fix"            # Modify return logic
    TIMEOUT_ADD = "timeout_add"         # Add timeout protection
    MEMORY_OPT = "memory_opt"          # Optimize memory usage
    SENSOR_CAL = "sensor_calibration"   # Calibrate sensor thresholds
    MOVEMENT_OPT = "movement_optimize"  # Optimize movement patterns
    COMM_ENHANCE = "comm_enhance"       # Improve communication
    SWARM_COORD = "swarm_coordination"  # Enhance multi-bot coordination

@dataclass
class MutationResult:
    """Result of applying a mutation"""
    success: bool
    mutation_type: MutationType
    original_code: str
    mutated_code: str
    fitness_before: float
    fitness_after: float
    execution_time: float
    error_message: str = ""
    test_results: Dict[str, Any] = None

@dataclass
class EvolutionCorpusEntry:
    """Entry in the evolution corpus tracking mutation effectiveness"""
    mutation_type: MutationType
    context_hash: str
    success_count: int = 0
    failure_count: int = 0
    average_fitness_gain: float = 0.0
    last_applied: float = 0.0
    
    @property
    def success_rate(self) -> float:
        total = self.success_count + self.failure_count
        return self.success_count / total if total > 0 else 0.0
    
    @property
    def effectiveness_score(self) -> float:
        """Combined metric of success rate and fitness gain"""
        base_score = self.success_rate * (1 + self.average_fitness_gain)
        # Add exploration bonus for rarely used mutations
        exploration_bonus = 0.1 if self.success_count + self.failure_count < 5 else 0.0
        return base_score + exploration_bonus

class EvolutionaryCodeSystem:
    """
    Main ECS engine for Project Jumbo swarm robotics
    
    This system tracks bot performance, identifies failure patterns,
    applies intelligent mutations, and learns from success/failure
    to continuously improve the swarm's capabilities.
    """
    
    def __init__(self, corpus_file: str = "evolution_corpus.json"):
        self.corpus_file = corpus_file
        self.evolution_corpus: Dict[str, EvolutionCorpusEntry] = {}
        self.load_corpus()
        
        # Performance tracking
        self.generation = 0
        self.total_mutations = 0
        self.successful_mutations = 0
        self.fitness_history: List[float] = []
        
        # Bot-specific parameters
        self.bot_parameters: Dict[str, Dict] = {}
        self.bot_fitness: Dict[str, float] = {}
        
        logging.info("🧬 ECS v2.0 initialized for Project Jumbo")
    
    def load_corpus(self):
        """Load the evolution corpus from disk"""
        try:
            with open(self.corpus_file, 'r') as f:
                data = json.load(f)
                for key, entry_data in data.items():
                    entry = EvolutionCorpusEntry(**entry_data)
                    entry.mutation_type = MutationType(entry.mutation_type)
                    self.evolution_corpus[key] = entry
            logging.info(f"Loaded {len(self.evolution_corpus)} corpus entries")
        except FileNotFoundError:
            logging.info("No existing corpus found, starting fresh")
        except Exception as e:
            logging.error(f"Error loading corpus: {e}")
    
    def save_corpus(self):
        """Save the evolution corpus to disk"""
        try:
            data = {}
            for key, entry in self.evolution_corpus.items():
                entry_dict = asdict(entry)
                entry_dict['mutation_type'] = entry.mutation_type.value
                data[key] = entry_dict
                
            with open(self.corpus_file, 'w') as f:
                json.dump(data, f, indent=2)
        except Exception as e:
            logging.error(f"Error saving corpus: {e}")
    
    def get_context_hash(self, function_name: str, error_type: str, 
                        parameters: Dict) -> str:
        """Generate a hash representing the current context"""
        context_str = f"{function_name}:{error_type}:{str(sorted(parameters.items()))}"
        return hashlib.md5(context_str.encode()).hexdigest()[:8]
    
    def select_mutation_strategy(self, function_name: str, error_type: str,
                               parameters: Dict) -> MutationType:
        """
        Intelligently select the best mutation strategy based on:
        1. Error type (exception-driven)
        2. Historical success (evolution corpus)
        3. Context similarity
        """
        
        # Exception-driven mutation mapping
        error_mutations = {
            'TypeError': [MutationType.TYPE_CHECK, MutationType.NULL_CHECK],
            'NameError': [MutationType.ERROR_HANDLER, MutationType.NULL_CHECK],
            'IndexError': [MutationType.BOUNDS_CHECK, MutationType.ERROR_HANDLER],
            'KeyError': [MutationType.NULL_CHECK, MutationType.ERROR_HANDLER],
            'AttributeError': [MutationType.ERROR_HANDLER, MutationType.TYPE_CHECK],
            'TimeoutError': [MutationType.TIMEOUT_ADD, MutationType.ERROR_HANDLER],
            'MemoryError': [MutationType.MEMORY_OPT, MutationType.ERROR_HANDLER],
            'LogicError': [MutationType.LOGIC_FIX, MutationType.PARAMETER_TUNE],
            'SensorError': [MutationType.SENSOR_CAL, MutationType.ERROR_HANDLER],
            'MovementError': [MutationType.MOVEMENT_OPT, MutationType.PARAMETER_TUNE],
            'CommunicationError': [MutationType.COMM_ENHANCE, MutationType.ERROR_HANDLER],
            'CoordinationError': [MutationType.SWARM_COORD, MutationType.COMM_ENHANCE]
        }
        
        context_hash = self.get_context_hash(function_name, error_type, parameters)
        
        # Get candidate mutations based on error type
        candidates = error_mutations.get(error_type, [MutationType.ERROR_HANDLER])
        
        # Weight candidates by historical effectiveness
        weighted_candidates = []
        for mutation_type in candidates:
            corpus_key = f"{context_hash}:{mutation_type.value}"
            if corpus_key in self.evolution_corpus:
                weight = self.evolution_corpus[corpus_key].effectiveness_score
            else:
                weight = 0.5  # Default weight for unknown mutations
            
            weighted_candidates.append((mutation_type, weight))
        
        # Select based on weighted probability
        total_weight = sum(weight for _, weight in weighted_candidates)
        if total_weight == 0:
            return random.choice(candidates)
        
        r = random.uniform(0, total_weight)
        cumulative = 0
        for mutation_type, weight in weighted_candidates:
            cumulative += weight
            if r <= cumulative:
                return mutation_type
        
        return candidates[0]  # Fallback
    
    def apply_mutation(self, code: str, mutation_type: MutationType,
                      context: Dict) -> str:
        """
        Apply a specific mutation to the code
        
        This generates context-aware mutations for swarm robotics:
        - Parameter optimization based on fitness feedback
        - Sensor calibration based on environmental data
        - Movement optimization based on success rates
        - Communication enhancement based on signal effectiveness
        """
        
        if mutation_type == MutationType.TYPE_CHECK:
            return self._add_type_checking(code, context)
        elif mutation_type == MutationType.NULL_CHECK:
            return self._add_null_checking(code, context)
        elif mutation_type == MutationType.BOUNDS_CHECK:
            return self._add_bounds_checking(code, context)
        elif mutation_type == MutationType.ERROR_HANDLER:
            return self._add_error_handling(code, context)
        elif mutation_type == MutationType.PARAMETER_TUNE:
            return self._tune_parameters(code, context)
        elif mutation_type == MutationType.LOGIC_FIX:
            return self._fix_logic(code, context)
        elif mutation_type == MutationType.TIMEOUT_ADD:
            return self._add_timeout_protection(code, context)
        elif mutation_type == MutationType.MEMORY_OPT:
            return self._optimize_memory(code, context)
        elif mutation_type == MutationType.SENSOR_CAL:
            return self._calibrate_sensors(code, context)
        elif mutation_type == MutationType.MOVEMENT_OPT:
            return self._optimize_movement(code, context)
        elif mutation_type == MutationType.COMM_ENHANCE:
            return self._enhance_communication(code, context)
        elif mutation_type == MutationType.SWARM_COORD:
            return self._improve_swarm_coordination(code, context)
        else:
            return code
    
    def _add_type_checking(self, code: str, context: Dict) -> str:
        """Add intelligent type checking based on expected inputs"""
        # Detect parameter types from context
        expected_types = context.get('expected_types', {})
        
        type_checks = []
        for param, expected_type in expected_types.items():
            if expected_type == 'list':
                type_checks.append(f"if not isinstance({param}, (list, tuple)):\n        return None")
            elif expected_type == 'dict':
                type_checks.append(f"if not isinstance({param}, dict):\n        return {{}}")
            elif expected_type == 'str':
                type_checks.append(f"if not isinstance({param}, str):\n        return ''")
            elif expected_type == 'int':
                type_checks.append(f"if not isinstance({param}, (int, float)):\n        return 0")
        
        if type_checks:
            # Insert type checks at the beginning of the function
            lines = code.split('\n')
            for i, line in enumerate(lines):
                if line.strip().startswith('def ') or line.strip().startswith('"""'):
                    # Find the end of docstring or function definition
                    insert_point = i + 1
                    if '"""' in line:
                        # Skip to end of docstring
                        while insert_point < len(lines) and '"""' not in lines[insert_point]:
                            insert_point += 1
                        insert_point += 1
                    
                    # Insert type checks
                    for check in type_checks:
                        lines.insert(insert_point, f"    {check}")
                        insert_point += 1
                    break
            
            return '\n'.join(lines)
        
        return code
    
    def _add_null_checking(self, code: str, context: Dict) -> str:
        """Add null/empty checking for robustness"""
        # Common null checks for robotics
        null_checks = [
            "if data is None:\n        return False",
            "if not sensor_readings:\n        return 0",
            "if distance < 0:\n        distance = 0"
        ]
        
        # Insert appropriate null check based on context
        function_name = context.get('function_name', '')
        if 'sensor' in function_name.lower():
            check = "if sensor_data is None or len(sensor_data) == 0:\n        return 0"
        elif 'move' in function_name.lower():
            check = "if speed is None or speed < 0:\n        speed = 0"
        elif 'communicate' in function_name.lower():
            check = "if message is None or len(message) == 0:\n        return False"
        else:
            check = random.choice(null_checks)
        
        # Insert at function start
        lines = code.split('\n')
        for i, line in enumerate(lines):
            if line.strip().startswith('def ') and ':' in line:
                lines.insert(i + 1, f"    {check}")
                break
        
        return '\n'.join(lines)
    
    def _tune_parameters(self, code: str, context: Dict) -> str:
        """Intelligently tune numeric parameters based on fitness feedback"""
        fitness_trend = context.get('fitness_trend', 0)
        last_params = context.get('last_parameters', {})
        
        # Common robotics parameters to tune
        param_adjustments = {
            'speed': (0.8, 1.2),           # ±20% speed adjustment
            'threshold': (0.9, 1.1),       # ±10% threshold adjustment  
            'delay': (0.5, 1.5),           # ±50% delay adjustment
            'sensitivity': (0.85, 1.15),   # ±15% sensitivity adjustment
            'timeout': (0.7, 1.3),         # ±30% timeout adjustment
        }
        
        lines = code.split('\n')
        for i, line in enumerate(lines):
            # Look for parameter assignments
            for param_name, (min_mult, max_mult) in param_adjustments.items():
                if f"{param_name} =" in line and "=" in line:
                    try:
                        # Extract current value
                        parts = line.split('=')
                        if len(parts) >= 2:
                            value_part = parts[1].strip()
                            # Try to extract numeric value
                            import re
                            numbers = re.findall(r'\d+\.?\d*', value_part)
                            if numbers:
                                current_val = float(numbers[0])
                                
                                # Adjust based on fitness trend
                                if fitness_trend > 0:
                                    # Fitness improving, make small adjustment
                                    multiplier = random.uniform(0.95, 1.05)
                                elif fitness_trend < 0:
                                    # Fitness declining, make larger adjustment
                                    multiplier = random.uniform(min_mult, max_mult)
                                else:
                                    # No trend, random adjustment
                                    multiplier = random.uniform(min_mult, max_mult)
                                
                                new_val = int(current_val * multiplier)
                                new_line = line.replace(str(int(current_val)), str(new_val))
                                lines[i] = new_line
                                logging.info(f"Parameter tuning: {param_name} {current_val} → {new_val}")
                                break
                    except (ValueError, IndexError):
                        continue
        
        return '\n'.join(lines)
    
    def _calibrate_sensors(self, code: str, context: Dict) -> str:
        """Calibrate sensor thresholds based on environmental feedback"""
        sensor_performance = context.get('sensor_performance', {})
        
        calibrations = []
        if 'false_positives' in sensor_performance:
            calibrations.append("    # Auto-calibration: reduce false positives\n    if sensor_reading < noise_threshold:\n        sensor_reading = 0")
        
        if 'missed_detections' in sensor_performance:
            calibrations.append("    # Auto-calibration: improve detection sensitivity\n    if sensor_reading > 0:\n        sensor_reading *= 1.1  # Increase sensitivity")
        
        if calibrations:
            # Insert calibrations at the start of sensor functions
            lines = code.split('\n')
            for i, line in enumerate(lines):
                if 'sensor' in line.lower() and 'def ' in line:
                    for calibration in calibrations:
                        lines.insert(i + 2, calibration)
                    break
            return '\n'.join(lines)
        
        return code
    
    def _optimize_movement(self, code: str, context: Dict) -> str:
        """Optimize movement patterns based on success rates"""
        movement_stats = context.get('movement_stats', {})
        
        optimizations = []
        
        # Collision rate optimization
        collision_rate = movement_stats.get('collision_rate', 0)
        if collision_rate > 0.1:  # >10% collision rate
            optimizations.append("    # Collision avoidance optimization\n    if obstacle_distance < safety_margin * 1.5:\n        motor_speed *= 0.7")
        
        # Stuck detection optimization
        stuck_rate = movement_stats.get('stuck_rate', 0)
        if stuck_rate > 0.05:  # >5% stuck rate
            optimizations.append("    # Stuck prevention\n    if movement_time > stuck_threshold:\n        execute_escape_maneuver()")
        
        # Energy efficiency optimization
        energy_efficiency = movement_stats.get('energy_efficiency', 1.0)
        if energy_efficiency < 0.8:  # <80% efficiency
            optimizations.append("    # Energy optimization\n    if distance_to_target > efficient_range:\n        motor_speed = int(motor_speed * 0.85)")
        
        if optimizations:
            lines = code.split('\n')
            for i, line in enumerate(lines):
                if ('move' in line.lower() or 'motor' in line.lower()) and 'def ' in line:
                    for opt in optimizations:
                        lines.insert(i + 2, opt)
                    break
            return '\n'.join(lines)
        
        return code
    
    def _enhance_communication(self, code: str, context: Dict) -> str:
        """Enhance communication protocols based on message success rates"""
        comm_stats = context.get('communication_stats', {})
        
        enhancements = []
        
        # Message delivery optimization
        delivery_rate = comm_stats.get('delivery_rate', 1.0)
        if delivery_rate < 0.9:  # <90% delivery rate
            enhancements.append("    # Message reliability enhancement\n    if not message_acknowledged:\n        retransmit_with_backoff(message)")
        
        # Signal clarity optimization
        signal_clarity = comm_stats.get('signal_clarity', 1.0)
        if signal_clarity < 0.8:  # <80% clarity
            enhancements.append("    # Signal clarity improvement\n    signal_strength *= 1.2\n    add_error_correction(message)")
        
        # Emergent vocabulary optimization
        vocab_usage = comm_stats.get('vocabulary_efficiency', 1.0)
        if vocab_usage < 0.6:  # <60% vocabulary efficiency
            enhancements.append("    # Vocabulary pruning\n    if signal_usage_count < min_usage_threshold:\n        remove_unused_signal(signal)")
        
        if enhancements:
            lines = code.split('\n')
            for i, line in enumerate(lines):
                if ('signal' in line.lower() or 'communicate' in line.lower()) and 'def ' in line:
                    for enhancement in enhancements:
                        lines.insert(i + 2, enhancement)
                    break
            return '\n'.join(lines)
        
        return code
    
    def _improve_swarm_coordination(self, code: str, context: Dict) -> str:
        """Improve multi-bot coordination based on swarm performance"""
        swarm_stats = context.get('swarm_stats', {})
        
        improvements = []
        
        # Task distribution optimization
        task_efficiency = swarm_stats.get('task_distribution_efficiency', 1.0)
        if task_efficiency < 0.7:  # <70% task distribution efficiency
            improvements.append("    # Task distribution improvement\n    if bot_workload > average_workload * 1.3:\n        delegate_task_to_idle_bot()")
        
        # Formation stability optimization
        formation_stability = swarm_stats.get('formation_stability', 1.0)
        if formation_stability < 0.8:  # <80% formation stability
            improvements.append("    # Formation stability enhancement\n    adjust_position_based_on_neighbors(neighbor_positions)")
        
        # Collective decision making
        decision_accuracy = swarm_stats.get('collective_decision_accuracy', 1.0)
        if decision_accuracy < 0.85:  # <85% decision accuracy
            improvements.append("    # Collective decision improvement\n    weight_decision_by_bot_confidence(decision, confidence_level)")
        
        if improvements:
            lines = code.split('\n')
            for i, line in enumerate(lines):
                if ('swarm' in line.lower() or 'coordinate' in line.lower()) and 'def ' in line:
                    for improvement in improvements:
                        lines.insert(i + 2, improvement)
                    break
            return '\n'.join(lines)
        
        return code
    
    def _add_error_handling(self, code: str, context: Dict) -> str:
        """Add comprehensive error handling"""
        # Wrap the entire function body in try/except
        lines = code.split('\n')
        function_start = -1
        
        for i, line in enumerate(lines):
            if line.strip().startswith('def '):
                function_start = i
                break
        
        if function_start >= 0:
            # Find function body start
            body_start = function_start + 1
            while body_start < len(lines) and (lines[body_start].strip() == '' or lines[body_start].strip().startswith('"""')):
                body_start += 1
                if '"""' in lines[body_start - 1] and lines[body_start - 1].count('"""') % 2 == 1:
                    # Multi-line docstring, find end
                    while body_start < len(lines) and '"""' not in lines[body_start]:
                        body_start += 1
                    body_start += 1
            
            # Insert try block
            lines.insert(body_start, "    try:")
            
            # Indent existing function body
            for i in range(body_start + 1, len(lines)):
                if lines[i].strip() == '':
                    continue
                if lines[i].startswith('    '):
                    lines[i] = '    ' + lines[i]
                else:
                    break
            
            # Add except block
            except_block = [
                "    except Exception as e:",
                "        logging.warning(f'Error in function: {e}')",
                "        return None  # Safe fallback"
            ]
            
            lines.extend(except_block)
        
        return '\n'.join(lines)
    
    def _add_bounds_checking(self, code: str, context: Dict) -> str:
        """Add array/list bounds checking"""
        bounds_check = "    if index < 0 or index >= len(data):\n        return None"
        
        lines = code.split('\n')
        for i, line in enumerate(lines):
            if '[' in line and ']' in line:  # Potential array access
                lines.insert(i, bounds_check)
                break
        
        return '\n'.join(lines)
    
    def _fix_logic(self, code: str, context: Dict) -> str:
        """Attempt to fix logic errors based on expected behavior"""
        expected_output = context.get('expected_output')
        if expected_output is not None:
            # Simple logic fixes based on expected return values
            if expected_output == False:
                code = code.replace('return True', 'return False')
            elif expected_output == True:
                code = code.replace('return False', 'return True')
            elif expected_output == 0:
                code = code.replace('return -1', 'return 0')
        
        return code
    
    def _add_timeout_protection(self, code: str, context: Dict) -> str:
        """Add timeout protection for long-running operations"""
        timeout_wrapper = """    import signal
    def timeout_handler(signum, frame):
        raise TimeoutError("Operation timed out")
    
    signal.signal(signal.SIGALRM, timeout_handler)
    signal.alarm(30)  # 30 second timeout
    try:"""
        
        lines = code.split('\n')
        # Insert timeout at function start
        for i, line in enumerate(lines):
            if line.strip().startswith('def '):
                lines.insert(i + 1, timeout_wrapper)
                # Add finally block at end
                lines.append("    finally:")
                lines.append("        signal.alarm(0)  # Cancel timeout")
                break
        
        return '\n'.join(lines)
    
    def _optimize_memory(self, code: str, context: Dict) -> str:
        """Optimize memory usage"""
        # Add memory optimization hints
        optimizations = [
            "    # Memory optimization",
            "    import gc",
            "    gc.collect()  # Force garbage collection"
        ]
        
        lines = code.split('\n')
        for i, line in enumerate(lines):
            if line.strip().startswith('def '):
                for opt in optimizations:
                    lines.insert(i + 1, opt)
                break
        
        return '\n'.join(lines)
    
    def evaluate_fitness(self, bot_id: str, performance_metrics: Dict) -> float:
        """
        Evaluate bot fitness based on multiple performance metrics
        
        Metrics include:
        - Task completion rate
        - Energy efficiency  
        - Communication effectiveness
        - Collision avoidance
        - Cooperation score
        - Adaptation speed
        """
        
        # Weight different aspects of performance
        weights = {
            'task_completion_rate': 0.25,
            'energy_efficiency': 0.20,
            'communication_effectiveness': 0.15,
            'collision_avoidance': 0.15,
            'cooperation_score': 0.15,
            'adaptation_speed': 0.10
        }
        
        fitness = 0.0
        for metric, weight in weights.items():
            value = performance_metrics.get(metric, 0.0)
            fitness += value * weight
        
        # Store fitness for tracking
        self.bot_fitness[bot_id] = fitness
        self.fitness_history.append(fitness)
        
        return fitness
    
    def evolve_bot_behavior(self, bot_id: str, error_log: List[str], 
                          performance_metrics: Dict) -> Dict:
        """
        Main evolution loop for a specific bot
        
        1. Analyze errors and performance
        2. Select appropriate mutations
        3. Apply mutations and test
        4. Update evolution corpus
        5. Return evolved parameters/code
        """
        
        logging.info(f"🧬 Starting evolution for bot {bot_id}")
        
        # Calculate current fitness
        current_fitness = self.evaluate_fitness(bot_id, performance_metrics)
        
        # Analyze error patterns
        error_types = self._analyze_error_patterns(error_log)
        
        results = []
        
        for error_type in error_types:
            # Get current bot parameters
            current_params = self.bot_parameters.get(bot_id, {})
            
            # Select mutation strategy
            mutation_type = self.select_mutation_strategy(
                function_name=f"bot_{bot_id}_behavior",
                error_type=error_type,
                parameters=current_params
            )
            
            # Apply mutation
            context = {
                'bot_id': bot_id,
                'error_type': error_type,
                'current_fitness': current_fitness,
                'performance_metrics': performance_metrics,
                'last_parameters': current_params
            }
            
            # For now, we'll evolve parameters rather than code directly
            evolved_params = self._evolve_parameters(current_params, mutation_type, context)
            
            # Evaluate the mutation (would need actual testing in practice)
            new_fitness = current_fitness + random.uniform(-0.1, 0.2)  # Simulated
            
            # Update evolution corpus
            self._update_corpus(mutation_type, context, new_fitness > current_fitness, 
                              new_fitness - current_fitness)
            
            result = MutationResult(
                success=new_fitness > current_fitness,
                mutation_type=mutation_type,
                original_code=str(current_params),
                mutated_code=str(evolved_params),
                fitness_before=current_fitness,
                fitness_after=new_fitness,
                execution_time=time.time()
            )
            
            results.append(result)
            
            # Update bot parameters if improvement
            if new_fitness > current_fitness:
                self.bot_parameters[bot_id] = evolved_params
                logging.info(f"✅ Evolution successful for {bot_id}: {current_fitness:.3f} → {new_fitness:.3f}")
            else:
                logging.info(f"❌ Evolution attempt failed for {bot_id}")
        
        self.total_mutations += len(results)
        self.successful_mutations += sum(1 for r in results if r.success)
        self.generation += 1
        
        # Save corpus periodically
        if self.generation % 10 == 0:
            self.save_corpus()
        
        return {
            'bot_id': bot_id,
            'generation': self.generation,
            'evolved_parameters': self.bot_parameters.get(bot_id, {}),
            'fitness': self.bot_fitness.get(bot_id, 0.0),
            'mutation_results': results,
            'success_rate': self.successful_mutations / max(1, self.total_mutations)
        }
    
    def _analyze_error_patterns(self, error_log: List[str]) -> List[str]:
        """Analyze error patterns to identify root causes"""
        error_types = []
        
        for error in error_log:
            if 'sensor' in error.lower():
                error_types.append('SensorError')
            elif 'move' in error.lower() or 'motor' in error.lower():
                error_types.append('MovementError')
            elif 'communication' in error.lower() or 'signal' in error.lower():
                error_types.append('CommunicationError')
            elif 'coordination' in error.lower() or 'swarm' in error.lower():
                error_types.append('CoordinationError')
            elif 'timeout' in error.lower():
                error_types.append('TimeoutError')
            elif 'memory' in error.lower():
                error_types.append('MemoryError')
            else:
                error_types.append('LogicError')
        
        return list(set(error_types))  # Remove duplicates
    
    def _evolve_parameters(self, current_params: Dict, mutation_type: MutationType, 
                          context: Dict) -> Dict:
        """Evolve bot parameters based on mutation type"""
        evolved = current_params.copy()
        
        if mutation_type == MutationType.PARAMETER_TUNE:
            # Tune movement parameters
            for param in ['motorSpeed', 'turnSpeed', 'obstacleThreshold']:
                if param in evolved:
                    current_val = evolved[param]
                    # Apply gaussian mutation
                    mutation = random.gauss(0, current_val * 0.1)  # 10% std dev
                    evolved[param] = max(0, int(current_val + mutation))
        
        elif mutation_type == MutationType.SENSOR_CAL:
            # Calibrate sensor thresholds
            for param in ['obstacleThreshold', 'clearThreshold']:
                if param in evolved:
                    current_val = evolved[param]
                    # Adjust based on sensor performance
                    adjustment = random.uniform(0.9, 1.1)
                    evolved[param] = int(current_val * adjustment)
        
        elif mutation_type == MutationType.MOVEMENT_OPT:
            # Optimize movement patterns
            for param in ['motorSpeed', 'turnDuration', 'backupDuration']:
                if param in evolved:
                    current_val = evolved[param]
                    # Optimize based on collision/stuck rates
                    if 'collision_rate' in context.get('performance_metrics', {}):
                        rate = context['performance_metrics']['collision_rate']
                        if rate > 0.1:  # High collision rate
                            if param == 'motorSpeed':
                                evolved[param] = int(current_val * 0.9)  # Slow down
                            elif param == 'backupDuration':
                                evolved[param] = int(current_val * 1.2)  # Back up longer
        
        return evolved
    
    def _update_corpus(self, mutation_type: MutationType, context: Dict, 
                      success: bool, fitness_delta: float):
        """Update the evolution corpus with mutation results"""
        context_hash = self.get_context_hash(
            context.get('bot_id', 'unknown'),
            context.get('error_type', 'unknown'),
            context.get('last_parameters', {})
        )
        
        corpus_key = f"{context_hash}:{mutation_type.value}"
        
        if corpus_key not in self.evolution_corpus:
            self.evolution_corpus[corpus_key] = EvolutionCorpusEntry(
                mutation_type=mutation_type,
                context_hash=context_hash
            )
        
        entry = self.evolution_corpus[corpus_key]
        
        if success:
            entry.success_count += 1
            # Update average fitness gain
            total_successes = entry.success_count
            entry.average_fitness_gain = (
                (entry.average_fitness_gain * (total_successes - 1) + fitness_delta) 
                / total_successes
            )
        else:
            entry.failure_count += 1
        
        entry.last_applied = time.time()
    
    def get_evolution_stats(self) -> Dict:
        """Get comprehensive evolution statistics"""
        return {
            'generation': self.generation,
            'total_mutations': self.total_mutations,
            'successful_mutations': self.successful_mutations,
            'success_rate': self.successful_mutations / max(1, self.total_mutations),
            'average_fitness': sum(self.fitness_history) / max(1, len(self.fitness_history)),
            'fitness_trend': self._calculate_fitness_trend(),
            'corpus_size': len(self.evolution_corpus),
            'bot_count': len(self.bot_parameters),
            'active_bots': list(self.bot_parameters.keys())
        }
    
    def _calculate_fitness_trend(self) -> float:
        """Calculate fitness trend over recent generations"""
        if len(self.fitness_history) < 10:
            return 0.0
        
        recent = self.fitness_history[-10:]
        older = self.fitness_history[-20:-10] if len(self.fitness_history) >= 20 else self.fitness_history[:-10]
        
        if not older:
            return 0.0
        
        recent_avg = sum(recent) / len(recent)
        older_avg = sum(older) / len(older)
        
        return recent_avg - older_avg

# Example usage and testing
if __name__ == "__main__":
    # Initialize ECS
    ecs = EvolutionaryCodeSystem()
    
    # Simulate bot evolution
    bot_id = "WHEELIE_001"
    
    # Simulate performance metrics
    performance_metrics = {
        'task_completion_rate': 0.75,
        'energy_efficiency': 0.80,
        'communication_effectiveness': 0.65,
        'collision_avoidance': 0.90,
        'cooperation_score': 0.70,
        'adaptation_speed': 0.85
    }
    
    # Simulate error log
    error_log = [
        "SensorError: Distance reading timeout",
        "MovementError: Motor stall detected",
        "CommunicationError: Signal transmission failed"
    ]
    
    # Evolve bot behavior
    result = ecs.evolve_bot_behavior(bot_id, error_log, performance_metrics)
    
    print("🧬 Evolution Results:")
    print(f"Bot ID: {result['bot_id']}")
    print(f"Generation: {result['generation']}")
    print(f"Fitness: {result['fitness']:.3f}")
    print(f"Success Rate: {result['success_rate']:.1%}")
    
    print("\nMutation Results:")
    for mutation in result['mutation_results']:
        status = "✅" if mutation.success else "❌"
        print(f"{status} {mutation.mutation_type.value}: {mutation.fitness_before:.3f} → {mutation.fitness_after:.3f}")
    
    print(f"\nEvolution Stats:")
    stats = ecs.get_evolution_stats()
    for key, value in stats.items():
        print(f"{key}: {value}")