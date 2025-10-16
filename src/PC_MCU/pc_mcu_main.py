#!/usr/bin/env python3
"""
🖥️ PC MCU - Master Control Unit
Complete PC application for managing WHEELIE, SPEEDIE, and MICRO BOT ecosystem

Features:
- Real-time bot monitoring and control
- Mission planning and execution
- Data collection and analysis
- Swarm coordination
- Performance analytics
- Emergency protocols

Author: Project Jumbo Team
Version: 1.0.0
"""

import asyncio
import websockets
import json
import time
import threading
import tkinter as tk
from tkinter import ttk, scrolledtext, messagebox
from datetime import datetime, timedelta
import matplotlib.pyplot as plt
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg
import numpy as np
from collections import defaultdict, deque
import logging

# Configure logging
logging.basicConfig(level=logging.INFO, format='%(asctime)s - %(levelname)s - %(message)s')
logger = logging.getLogger(__name__)

class BotData:
    """Data structure for bot information"""
    def __init__(self, bot_id):
        self.bot_id = bot_id
        self.is_online = False
        self.ip_address = ""
        self.generation = 0
        self.fitness_score = 0.0
        self.last_seen = None
        self.message_count = 0
        self.performance_history = deque(maxlen=100)
        self.emotional_state = {}
        self.sensor_data = {}
        self.capabilities = []
        
    def update(self, data):
        """Update bot data from message"""
        self.generation = data.get('generation', self.generation)
        self.fitness_score = data.get('fitness_score', self.fitness_score)
        self.emotional_state = data.get('emotional_state', self.emotional_state)
        self.sensor_data = data.get('sensor_data', self.sensor_data)
        self.last_seen = datetime.now()
        self.message_count += 1
        
        # Add to performance history
        self.performance_history.append({
            'timestamp': self.last_seen,
            'fitness': self.fitness_score,
            'generation': self.generation
        })

class MissionPlanner:
    """Mission planning and execution system"""
    def __init__(self):
        self.active_missions = {}
        self.mission_history = []
        
    def create_exploration_mission(self, bot_id, area_bounds, duration_minutes=10):
        """Create an exploration mission"""
        mission = {
            'id': f"explore_{int(time.time())}",
            'type': 'exploration',
            'bot_id': bot_id,
            'parameters': {
                'area_bounds': area_bounds,
                'duration': duration_minutes * 60,
                'scan_resolution': 0.1,
                'return_to_start': True
            },
            'status': 'pending',
            'created_at': datetime.now(),
            'expected_completion': datetime.now() + timedelta(minutes=duration_minutes)
        }
        self.active_missions[mission['id']] = mission
        return mission
        
    def create_patrol_mission(self, bot_id, waypoints, cycles=1):
        """Create a patrol mission"""
        mission = {
            'id': f"patrol_{int(time.time())}",
            'type': 'patrol',
            'bot_id': bot_id,
            'parameters': {
                'waypoints': waypoints,
                'cycles': cycles,
                'speed_mode': 'adaptive',
                'obstacle_avoidance': True
            },
            'status': 'pending',
            'created_at': datetime.now()
        }
        self.active_missions[mission['id']] = mission
        return mission
        
    def create_collaborative_mission(self, bot_ids, mission_type, parameters):
        """Create a collaborative mission for multiple bots"""
        mission = {
            'id': f"collab_{mission_type}_{int(time.time())}",
            'type': f"collaborative_{mission_type}",
            'bot_ids': bot_ids,
            'parameters': parameters,
            'status': 'pending',
            'created_at': datetime.now()
        }
        self.active_missions[mission['id']] = mission
        return mission

class PCMCUApp:
    """Main PC MCU Application"""
    
    def __init__(self):
        self.root = tk.Tk()
        self.root.title("🖥️ PC MCU - Project Jumbo Control Center")
        self.root.geometry("1400x900")
        
        # Data structures
        self.bots = {}
        self.mission_planner = MissionPlanner()
        self.websocket = None
        self.connected = False
        self.message_log = deque(maxlen=1000)
        
        # Connection settings
        self.micro_bot_ip = "192.168.4.1"
        self.micro_bot_port = 8766
        
        # Initialize GUI
        self.setup_gui()
        
        # Start connection manager
        self.connection_thread = None
        self.running = True
        
    def setup_gui(self):
        """Setup the GUI interface"""
        # Create main notebook for tabs
        notebook = ttk.Notebook(self.root)
        notebook.pack(fill=tk.BOTH, expand=True, padx=5, pady=5)
        
        # Connection Tab
        self.setup_connection_tab(notebook)
        
        # Monitoring Tab
        self.setup_monitoring_tab(notebook)
        
        # Mission Control Tab
        self.setup_mission_tab(notebook)
        
        # Analytics Tab
        self.setup_analytics_tab(notebook)
        
        # Logs Tab
        self.setup_logs_tab(notebook)
        
        # Status bar
        self.status_bar = ttk.Label(self.root, text="Disconnected", relief=tk.SUNKEN)
        self.status_bar.pack(side=tk.BOTTOM, fill=tk.X)
        
    def setup_connection_tab(self, notebook):
        """Setup connection management tab"""
        conn_frame = ttk.Frame(notebook)
        notebook.add(conn_frame, text="🔗 Connection")
        
        # Connection settings
        settings_frame = ttk.LabelFrame(conn_frame, text="Connection Settings")
        settings_frame.pack(fill=tk.X, padx=5, pady=5)
        
        ttk.Label(settings_frame, text="MICRO BOT IP:").grid(row=0, column=0, padx=5, pady=5)
        self.ip_entry = ttk.Entry(settings_frame)
        self.ip_entry.insert(0, self.micro_bot_ip)
        self.ip_entry.grid(row=0, column=1, padx=5, pady=5)
        
        ttk.Label(settings_frame, text="Port:").grid(row=0, column=2, padx=5, pady=5)
        self.port_entry = ttk.Entry(settings_frame, width=10)
        self.port_entry.insert(0, str(self.micro_bot_port))
        self.port_entry.grid(row=0, column=3, padx=5, pady=5)
        
        self.connect_btn = ttk.Button(settings_frame, text="Connect", command=self.toggle_connection)
        self.connect_btn.grid(row=0, column=4, padx=5, pady=5)
        
        # Connection status
        status_frame = ttk.LabelFrame(conn_frame, text="Status")
        status_frame.pack(fill=tk.BOTH, expand=True, padx=5, pady=5)
        
        self.connection_status = scrolledtext.ScrolledText(status_frame, height=20)
        self.connection_status.pack(fill=tk.BOTH, expand=True, padx=5, pady=5)
        
    def setup_monitoring_tab(self, notebook):
        """Setup bot monitoring tab"""
        monitor_frame = ttk.Frame(notebook)
        notebook.add(monitor_frame, text="🤖 Monitoring")
        
        # Bot list
        bots_frame = ttk.LabelFrame(monitor_frame, text="Connected Bots")
        bots_frame.pack(fill=tk.X, padx=5, pady=5)
        
        self.bot_tree = ttk.Treeview(bots_frame, columns=('status', 'generation', 'fitness', 'messages'), height=6)
        self.bot_tree.heading('#0', text='Bot ID')
        self.bot_tree.heading('status', text='Status')
        self.bot_tree.heading('generation', text='Generation')
        self.bot_tree.heading('fitness', text='Fitness')
        self.bot_tree.heading('messages', text='Messages')
        self.bot_tree.pack(fill=tk.X, padx=5, pady=5)
        
        # Bot details
        details_frame = ttk.LabelFrame(monitor_frame, text="Bot Details")
        details_frame.pack(fill=tk.BOTH, expand=True, padx=5, pady=5)
        
        self.bot_details = scrolledtext.ScrolledText(details_frame, height=15)
        self.bot_details.pack(fill=tk.BOTH, expand=True, padx=5, pady=5)
        
        # Bind tree selection
        self.bot_tree.bind('<<TreeviewSelect>>', self.on_bot_select)
        
    def setup_mission_tab(self, notebook):
        """Setup mission control tab"""
        mission_frame = ttk.Frame(notebook)
        notebook.add(mission_frame, text="🎯 Missions")
        
        # Mission creation
        create_frame = ttk.LabelFrame(mission_frame, text="Create Mission")
        create_frame.pack(fill=tk.X, padx=5, pady=5)
        
        # Mission type selection
        ttk.Label(create_frame, text="Mission Type:").grid(row=0, column=0, padx=5, pady=5)
        self.mission_type = ttk.Combobox(create_frame, values=['Exploration', 'Patrol', 'Collaborative Search'])
        self.mission_type.grid(row=0, column=1, padx=5, pady=5)
        
        ttk.Label(create_frame, text="Target Bot:").grid(row=0, column=2, padx=5, pady=5)
        self.target_bot = ttk.Combobox(create_frame, values=[])
        self.target_bot.grid(row=0, column=3, padx=5, pady=5)
        
        ttk.Button(create_frame, text="Create Mission", command=self.create_mission).grid(row=0, column=4, padx=5, pady=5)
        
        # Active missions
        active_frame = ttk.LabelFrame(mission_frame, text="Active Missions")
        active_frame.pack(fill=tk.BOTH, expand=True, padx=5, pady=5)
        
        self.mission_tree = ttk.Treeview(active_frame, columns=('type', 'bot', 'status', 'progress'), height=10)
        self.mission_tree.heading('#0', text='Mission ID')
        self.mission_tree.heading('type', text='Type')
        self.mission_tree.heading('bot', text='Bot')
        self.mission_tree.heading('status', text='Status')
        self.mission_tree.heading('progress', text='Progress')
        self.mission_tree.pack(fill=tk.BOTH, expand=True, padx=5, pady=5)
        
    def setup_analytics_tab(self, notebook):
        """Setup analytics and visualization tab"""
        analytics_frame = ttk.Frame(notebook)
        notebook.add(analytics_frame, text="📊 Analytics")
        
        # Create matplotlib figure
        self.fig, ((self.ax1, self.ax2), (self.ax3, self.ax4)) = plt.subplots(2, 2, figsize=(12, 8))
        self.fig.suptitle('Project Jumbo Analytics Dashboard')
        
        # Embed in tkinter
        self.canvas = FigureCanvasTkAgg(self.fig, analytics_frame)
        self.canvas.get_tk_widget().pack(fill=tk.BOTH, expand=True)
        
        # Update analytics every 5 seconds
        self.root.after(5000, self.update_analytics)
        
    def setup_logs_tab(self, notebook):
        """Setup logs tab"""
        logs_frame = ttk.Frame(notebook)
        notebook.add(logs_frame, text="📝 Logs")
        
        # Log controls
        controls_frame = ttk.Frame(logs_frame)
        controls_frame.pack(fill=tk.X, padx=5, pady=5)
        
        ttk.Button(controls_frame, text="Clear Logs", command=self.clear_logs).pack(side=tk.LEFT, padx=5)
        ttk.Button(controls_frame, text="Save Logs", command=self.save_logs).pack(side=tk.LEFT, padx=5)
        
        # Log display
        self.log_display = scrolledtext.ScrolledText(logs_frame)
        self.log_display.pack(fill=tk.BOTH, expand=True, padx=5, pady=5)
        
    def toggle_connection(self):
        """Toggle connection to MICRO BOT"""
        if self.connected:
            self.disconnect()
        else:
            self.connect()
            
    def connect(self):
        """Connect to MICRO BOT"""
        self.micro_bot_ip = self.ip_entry.get()
        self.micro_bot_port = int(self.port_entry.get())
        
        self.connection_thread = threading.Thread(target=self.connection_worker, daemon=True)
        self.connection_thread.start()
        
    def disconnect(self):
        """Disconnect from MICRO BOT"""
        self.running = False
        if self.websocket:
            asyncio.run_coroutine_threadsafe(self.websocket.close(), asyncio.get_event_loop())
        self.connected = False
        self.update_connection_status("Disconnected")
        self.connect_btn.config(text="Connect")
        
    def connection_worker(self):
        """Connection worker thread"""
        asyncio.run(self.async_connection_worker())
        
    async def async_connection_worker(self):
        """Async connection worker"""
        try:
            uri = f"ws://{self.micro_bot_ip}:{self.micro_bot_port}"
            self.update_connection_status(f"Connecting to {uri}...")
            
            self.websocket = await websockets.connect(uri)
            self.connected = True
            
            self.root.after(0, lambda: self.update_connection_status("✅ Connected to MICRO BOT"))
            self.root.after(0, lambda: self.connect_btn.config(text="Disconnect"))
            
            # Start listening for messages
            await self.listen_for_messages()
            
        except Exception as e:
            self.root.after(0, lambda: self.update_connection_status(f"❌ Connection failed: {e}"))
            self.connected = False
            
    async def listen_for_messages(self):
        """Listen for messages from MICRO BOT"""
        try:
            async for message in self.websocket:
                data = json.loads(message)
                self.root.after(0, lambda d=data: self.handle_message(d))
                
        except websockets.exceptions.ConnectionClosed:
            self.root.after(0, lambda: self.update_connection_status("🔌 Connection closed"))
            self.connected = False
        except Exception as e:
            self.root.after(0, lambda: self.update_connection_status(f"❌ Message error: {e}"))
            
    def handle_message(self, data):
        """Handle incoming messages"""
        msg_type = data.get('type', 'unknown')
        timestamp = datetime.now().strftime('%H:%M:%S')
        
        # Log message
        self.message_log.append(f"[{timestamp}] {msg_type}: {data}")
        self.log_message(f"[{timestamp}] Received {msg_type}")
        
        if msg_type == 'bot_list':
            self.update_bot_list(data.get('bots', []))
        elif msg_type == 'bot_message':
            self.handle_bot_message(data)
        elif msg_type == 'status_update':
            self.handle_status_update(data)
        elif msg_type == 'bot_offline':
            self.handle_bot_offline(data)
            
    def update_bot_list(self, bots_data):
        """Update bot list display"""
        # Clear existing items
        for item in self.bot_tree.get_children():
            self.bot_tree.delete(item)
            
        # Update bot data and display
        for bot_data in bots_data:
            bot_id = bot_data.get('bot_id', 'Unknown')
            
            if bot_id not in self.bots:
                self.bots[bot_id] = BotData(bot_id)
                
            bot = self.bots[bot_id]
            bot.is_online = bot_data.get('is_online', False)
            bot.ip_address = bot_data.get('ip_address', '')
            bot.generation = bot_data.get('generation', 0)
            bot.fitness_score = bot_data.get('fitness_score', 0.0)
            bot.message_count = bot_data.get('message_count', 0)
            
            # Add to tree
            status = "🟢 Online" if bot.is_online else "🔴 Offline"
            self.bot_tree.insert('', 'end', text=bot_id, values=(
                status, bot.generation, f"{bot.fitness_score:.3f}", bot.message_count
            ))
            
        # Update mission target bot combobox
        online_bots = [bot_id for bot_id, bot in self.bots.items() if bot.is_online]
        self.target_bot['values'] = online_bots
        
    def handle_bot_message(self, data):
        """Handle bot message"""
        bot_id = data.get('bot_id', 'Unknown')
        if bot_id in self.bots:
            self.bots[bot_id].update(data.get('data', {}))
            
    def handle_status_update(self, data):
        """Handle status update"""
        connected_bots = data.get('connected_bots', 0)
        uptime = data.get('uptime', 0)
        self.status_bar.config(text=f"Connected: {connected_bots} bots | Uptime: {uptime:.1f}s")
        
    def handle_bot_offline(self, data):
        """Handle bot offline notification"""
        bot_id = data.get('bot_id', 'Unknown')
        if bot_id in self.bots:
            self.bots[bot_id].is_online = False
        self.log_message(f"🔴 Bot {bot_id} went offline")
        
    def on_bot_select(self, event):
        """Handle bot selection in tree"""
        selection = self.bot_tree.selection()
        if selection:
            bot_id = self.bot_tree.item(selection[0])['text']
            if bot_id in self.bots:
                self.display_bot_details(self.bots[bot_id])
                
    def display_bot_details(self, bot):
        """Display detailed bot information"""
        self.bot_details.delete('1.0', tk.END)
        
        details = f"""Bot ID: {bot.bot_id}
Status: {'🟢 Online' if bot.is_online else '🔴 Offline'}
IP Address: {bot.ip_address}
Generation: {bot.generation}
Fitness Score: {bot.fitness_score:.6f}
Message Count: {bot.message_count}
Last Seen: {bot.last_seen.strftime('%Y-%m-%d %H:%M:%S') if bot.last_seen else 'Never'}

Emotional State:
"""
        
        for emotion, value in bot.emotional_state.items():
            details += f"  {emotion}: {value}\n"
            
        details += "\nSensor Data:\n"
        for sensor, value in bot.sensor_data.items():
            details += f"  {sensor}: {value}\n"
            
        self.bot_details.insert('1.0', details)
        
    def create_mission(self):
        """Create a new mission"""
        mission_type = self.mission_type.get()
        target_bot = self.target_bot.get()
        
        if not mission_type or not target_bot:
            messagebox.showwarning("Warning", "Please select mission type and target bot")
            return
            
        if mission_type == "Exploration":
            mission = self.mission_planner.create_exploration_mission(
                target_bot, 
                area_bounds=[(0, 0), (10, 10)],  # Default 10x10 area
                duration_minutes=5
            )
        elif mission_type == "Patrol":
            mission = self.mission_planner.create_patrol_mission(
                target_bot,
                waypoints=[(0, 0), (5, 0), (5, 5), (0, 5)],  # Square patrol
                cycles=2
            )
        else:
            messagebox.showinfo("Info", "Collaborative missions not yet implemented")
            return
            
        # Send mission to bot via MICRO BOT
        if self.connected:
            asyncio.run_coroutine_threadsafe(
                self.send_mission(mission), 
                asyncio.get_event_loop()
            )
            
        self.update_mission_display()
        self.log_message(f"📋 Created {mission_type} mission for {target_bot}")
        
    async def send_mission(self, mission):
        """Send mission to bot"""
        if self.websocket:
            message = {
                'type': 'mission',
                'target_bot': mission['bot_id'],
                'mission_data': mission,
                'timestamp': time.time()
            }
            await self.websocket.send(json.dumps(message))
            
    def update_mission_display(self):
        """Update mission display"""
        # Clear existing items
        for item in self.mission_tree.get_children():
            self.mission_tree.delete(item)
            
        # Add active missions
        for mission_id, mission in self.mission_planner.active_missions.items():
            bot_id = mission.get('bot_id', mission.get('bot_ids', ['Multiple']))
            if isinstance(bot_id, list):
                bot_id = ', '.join(bot_id)
                
            self.mission_tree.insert('', 'end', text=mission_id, values=(
                mission['type'], bot_id, mission['status'], "0%"
            ))
            
    def update_analytics(self):
        """Update analytics charts"""
        if not self.bots:
            self.root.after(5000, self.update_analytics)
            return
            
        # Clear previous plots
        for ax in [self.ax1, self.ax2, self.ax3, self.ax4]:
            ax.clear()
            
        # Plot 1: Fitness scores over time
        self.ax1.set_title('Fitness Score Trends')
        for bot_id, bot in self.bots.items():
            if bot.performance_history:
                times = [p['timestamp'] for p in bot.performance_history]
                fitness = [p['fitness'] for p in bot.performance_history]
                self.ax1.plot(times, fitness, label=bot_id, marker='o', markersize=2)
        self.ax1.legend()
        self.ax1.tick_params(axis='x', rotation=45)
        
        # Plot 2: Generation progress
        self.ax2.set_title('Generation Progress')
        bot_names = list(self.bots.keys())
        generations = [bot.generation for bot in self.bots.values()]
        self.ax2.bar(bot_names, generations)
        
        # Plot 3: Message activity
        self.ax3.set_title('Message Activity')
        message_counts = [bot.message_count for bot in self.bots.values()]
        self.ax3.bar(bot_names, message_counts)
        
        # Plot 4: Bot status pie chart
        self.ax4.set_title('Bot Status')
        online_count = sum(1 for bot in self.bots.values() if bot.is_online)
        offline_count = len(self.bots) - online_count
        
        if online_count > 0 or offline_count > 0:
            self.ax4.pie([online_count, offline_count], 
                        labels=['Online', 'Offline'], 
                        colors=['green', 'red'],
                        autopct='%1.1f%%')
        
        # Refresh canvas
        self.canvas.draw()
        
        # Schedule next update
        self.root.after(5000, self.update_analytics)
        
    def update_connection_status(self, message):
        """Update connection status display"""
        timestamp = datetime.now().strftime('%H:%M:%S')
        self.connection_status.insert(tk.END, f"[{timestamp}] {message}\n")
        self.connection_status.see(tk.END)
        
    def log_message(self, message):
        """Log message to display"""
        self.log_display.insert(tk.END, f"{message}\n")
        self.log_display.see(tk.END)
        
    def clear_logs(self):
        """Clear log display"""
        self.log_display.delete('1.0', tk.END)
        self.message_log.clear()
        
    def save_logs(self):
        """Save logs to file"""
        timestamp = datetime.now().strftime('%Y%m%d_%H%M%S')
        filename = f"pc_mcu_logs_{timestamp}.txt"
        
        try:
            with open(filename, 'w') as f:
                for log_entry in self.message_log:
                    f.write(f"{log_entry}\n")
            messagebox.showinfo("Success", f"Logs saved to {filename}")
        except Exception as e:
            messagebox.showerror("Error", f"Failed to save logs: {e}")
            
    def on_closing(self):
        """Handle application closing"""
        self.running = False
        if self.connected:
            self.disconnect()
        self.root.destroy()
        
    def run(self):
        """Run the application"""
        self.root.protocol("WM_DELETE_WINDOW", self.on_closing)
        
        # Request initial data if connected
        if self.connected:
            self.root.after(1000, self.request_initial_data)
            
        self.root.mainloop()
        
    def request_initial_data(self):
        """Request initial data from MICRO BOT"""
        if self.connected and self.websocket:
            async def send_requests():
                await self.websocket.send(json.dumps({'type': 'command', 'command': 'get_status'}))
                await asyncio.sleep(0.1)
                await self.websocket.send(json.dumps({'type': 'command', 'command': 'get_bots'}))
                
            asyncio.run_coroutine_threadsafe(send_requests(), asyncio.get_event_loop())

def main():
    """Main function"""
    print("🖥️ Starting PC MCU - Project Jumbo Control Center")
    print("==================================================")
    
    try:
        app = PCMCUApp()
        app.run()
    except KeyboardInterrupt:
        print("\n👋 Goodbye!")
    except Exception as e:
        print(f"❌ Application error: {e}")
        logger.exception("Application error")

if __name__ == "__main__":
    main()