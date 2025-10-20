#!/usr/bin/env python3
"""
PC MCU - Project Jumbo Control Center
Central command and control application for the Jumbo Bot swarm ecosystem.
"""
import tkinter as tk
from tkinter import ttk, messagebox, scrolledtext
import asyncio
import websockets
import json
import threading
from datetime import datetime
from collections import deque
import logging
from dataclasses import dataclass, field
from typing import Dict, List, Optional, Any
import matplotlib.pyplot as plt
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg
from matplotlib.figure import Figure
import time

# Set up logging
logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s - %(name)s - %(levelname)s - %(message)s'
)
logger = logging.getLogger('PC_MCU')

@dataclass
class BotData:
    """Data structure for bot information"""
    bot_id: str
    is_online: bool = False
    ip_address: str = ""
    generation: int = 0
    fitness_score: float = 0.0
    last_seen: Optional[datetime] = None
    message_count: int = 0
    performance_history: deque = field(default_factory=lambda: deque(maxlen=100))
    emotional_state: Dict = field(default_factory=dict)
    sensor_data: Dict = field(default_factory=dict)
    capabilities: List = field(default_factory=list)
    
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

@dataclass 
class MissionData:
    """Data structure for mission information"""
    mission_id: str
    mission_type: str
    assigned_bots: List[str] = field(default_factory=list)
    status: str = "pending"
    progress: float = 0.0
    created_at: datetime = field(default_factory=datetime.now)
    parameters: Dict = field(default_factory=dict)

class MissionPlanner:
    """Mission planning and execution system"""
    
    def __init__(self):
        self.missions = {}
        self.mission_counter = 0
        
    def create_mission(self, mission_type: str, bot_ids: List[str], parameters: Dict = None) -> str:
        """Create a new mission"""
        self.mission_counter += 1
        mission_id = f"MISSION_{self.mission_counter:03d}"
        
        mission = MissionData(
            mission_id=mission_id,
            mission_type=mission_type,
            assigned_bots=bot_ids.copy(),
            parameters=parameters or {}
        )
        
        self.missions[mission_id] = mission
        logger.info(f"Created mission {mission_id}: {mission_type}")
        return mission_id
        
    def get_mission(self, mission_id: str) -> Optional[MissionData]:
        """Get mission by ID"""
        return self.missions.get(mission_id)
        
    def update_mission_progress(self, mission_id: str, progress: float):
        """Update mission progress"""
        if mission_id in self.missions:
            self.missions[mission_id].progress = progress
            
    def set_mission_status(self, mission_id: str, status: str):
        """Set mission status"""
        if mission_id in self.missions:
            self.missions[mission_id].status = status

class PCMCUApp:
    """Main PC MCU Application"""
    
    def __init__(self):
        self.root = tk.Tk()
        self.root.title("PC MCU - Project Jumbo Control Center")
        self.root.geometry("1200x800")
        
        # Application state
        self.bots = {}
        self.mission_planner = MissionPlanner()
        self.websocket = None
        self.connected = False
        self.running = True
        self.message_log = deque(maxlen=1000)
        
        # Connection settings
        self.micro_bot_ip = "192.168.1.207"  # Pi's current IP
        self.micro_bot_port = 8766
        
        # Initialize GUI
        self.setup_gui()
        
        # Start connection manager
        self.connection_thread = None
        
        # Setup periodic updates
        self.root.after(1000, self.periodic_update)
        self.root.after(5000, self.update_analytics)
        
    def setup_gui(self):
        """Set up the GUI"""
        # Create notebook for tabs
        self.notebook = ttk.Notebook(self.root)
        self.notebook.pack(fill='both', expand=True, padx=5, pady=5)
        
        # Create tabs
        self.setup_connection_tab()
        self.setup_monitoring_tab()
        self.setup_missions_tab()
        self.setup_analytics_tab()
        self.setup_logs_tab()
        
    def setup_connection_tab(self):
        """Set up connection tab"""
        self.connection_frame = ttk.Frame(self.notebook)
        self.notebook.add(self.connection_frame, text="Connection")
        
        # Connection controls
        conn_controls = ttk.LabelFrame(self.connection_frame, text="MICRO BOT Connection")
        conn_controls.pack(fill='x', padx=10, pady=10)
        
        # IP and Port
        ttk.Label(conn_controls, text="IP Address:").grid(row=0, column=0, sticky='w', padx=5, pady=5)
        self.ip_entry = ttk.Entry(conn_controls)
        self.ip_entry.insert(0, self.micro_bot_ip)
        self.ip_entry.grid(row=0, column=1, padx=5, pady=5)
        
        ttk.Label(conn_controls, text="Port:").grid(row=0, column=2, sticky='w', padx=5, pady=5)
        self.port_entry = ttk.Entry(conn_controls, width=10)
        self.port_entry.insert(0, str(self.micro_bot_port))
        self.port_entry.grid(row=0, column=3, padx=5, pady=5)
        
        # Connect button
        self.connect_btn = ttk.Button(conn_controls, text="Connect", command=self.connect)
        self.connect_btn.grid(row=0, column=4, padx=10, pady=5)
        
        # Connection status
        status_frame = ttk.LabelFrame(self.connection_frame, text="Connection Status")
        status_frame.pack(fill='both', expand=True, padx=10, pady=10)
        
        self.connection_status = scrolledtext.ScrolledText(status_frame, height=20)
        self.connection_status.pack(fill='both', expand=True, padx=5, pady=5)
        
    def setup_monitoring_tab(self):
        """Set up monitoring tab"""
        self.monitoring_frame = ttk.Frame(self.notebook)
        self.notebook.add(self.monitoring_frame, text="Monitoring")
        
        # Bot list
        bot_frame = ttk.LabelFrame(self.monitoring_frame, text="Connected Bots")
        bot_frame.pack(fill='both', expand=True, padx=10, pady=10)
        
        # Bot tree
        columns = ('Status', 'IP', 'Generation', 'Fitness', 'Messages', 'Last Seen')
        self.bot_tree = ttk.Treeview(bot_frame, columns=columns, show='tree headings')
        
        self.bot_tree.heading('#0', text='Bot ID')
        for col in columns:
            self.bot_tree.heading(col, text=col)
            self.bot_tree.column(col, width=100)
            
        bot_scrollbar = ttk.Scrollbar(bot_frame, orient='vertical', command=self.bot_tree.yview)
        self.bot_tree.configure(yscrollcommand=bot_scrollbar.set)
        
        self.bot_tree.pack(side='left', fill='both', expand=True)
        bot_scrollbar.pack(side='right', fill='y')
        
    def setup_missions_tab(self):
        """Set up missions tab"""
        self.missions_frame = ttk.Frame(self.notebook)
        self.notebook.add(self.missions_frame, text="Missions")
        
        # Mission controls
        controls_frame = ttk.LabelFrame(self.missions_frame, text="Mission Control")
        controls_frame.pack(fill='x', padx=10, pady=10)
        
        # Mission type selection
        ttk.Label(controls_frame, text="Mission Type:").grid(row=0, column=0, sticky='w', padx=5, pady=5)
        self.mission_type = ttk.Combobox(controls_frame, values=[
            'exploration', 'formation', 'patrol', 'gather', 'emergency'
        ])
        self.mission_type.grid(row=0, column=1, padx=5, pady=5)
        self.mission_type.set('exploration')
        
        # Create mission button
        ttk.Button(controls_frame, text="Create Mission", command=self.create_mission).grid(row=0, column=2, padx=10, pady=5)
        
        # Mission list
        mission_frame = ttk.LabelFrame(self.missions_frame, text="Active Missions")
        mission_frame.pack(fill='both', expand=True, padx=10, pady=10)
        
        mission_columns = ('Type', 'Assigned Bots', 'Status', 'Progress')
        self.mission_tree = ttk.Treeview(mission_frame, columns=mission_columns, show='tree headings')
        
        self.mission_tree.heading('#0', text='Mission ID')
        for col in mission_columns:
            self.mission_tree.heading(col, text=col)
            self.mission_tree.column(col, width=150)
            
        mission_scrollbar = ttk.Scrollbar(mission_frame, orient='vertical', command=self.mission_tree.yview)
        self.mission_tree.configure(yscrollcommand=mission_scrollbar.set)
        
        self.mission_tree.pack(side='left', fill='both', expand=True)
        mission_scrollbar.pack(side='right', fill='y')
        
    def setup_analytics_tab(self):
        """Set up analytics tab"""
        self.analytics_frame = ttk.Frame(self.notebook)
        self.notebook.add(self.analytics_frame, text="Analytics")
        
        # Create matplotlib figure
        self.fig = Figure(figsize=(12, 8), dpi=80)
        
        # Create subplots
        self.ax1 = self.fig.add_subplot(221)
        self.ax2 = self.fig.add_subplot(222) 
        self.ax3 = self.fig.add_subplot(223)
        self.ax4 = self.fig.add_subplot(224)
        
        # Create canvas
        self.canvas = FigureCanvasTkAgg(self.fig, self.analytics_frame)
        self.canvas.get_tk_widget().pack(fill='both', expand=True, padx=10, pady=10)
        
    def setup_logs_tab(self):
        """Set up logs tab"""
        self.logs_frame = ttk.Frame(self.notebook)
        self.notebook.add(self.logs_frame, text="Logs")
        
        # Log display
        log_frame = ttk.LabelFrame(self.logs_frame, text="System Logs")
        log_frame.pack(fill='both', expand=True, padx=10, pady=10)
        
        self.log_display = scrolledtext.ScrolledText(log_frame)
        self.log_display.pack(fill='both', expand=True, padx=5, pady=5)
        
        # Log controls
        controls = ttk.Frame(log_frame)
        controls.pack(fill='x', padx=5, pady=5)
        
        ttk.Button(controls, text="Clear Logs", command=self.clear_logs).pack(side='left', padx=5)
        ttk.Button(controls, text="Export Logs", command=self.export_logs).pack(side='left', padx=5)
            
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
            error_msg = str(e)
            self.root.after(0, lambda: self.update_connection_status(f"❌ Connection failed: {error_msg}"))
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
            bot_id = bot_data.get('bot_id', 'unknown')
            
            # Update or create bot data
            if bot_id not in self.bots:
                self.bots[bot_id] = BotData(bot_id=bot_id)
                
            bot = self.bots[bot_id]
            bot.update(bot_data)
            
            # Update tree display
            status = "🟢 Online" if bot.is_online else "🔴 Offline"
            last_seen = bot.last_seen.strftime('%H:%M:%S') if bot.last_seen else "Never"
            
            self.bot_tree.insert('', 'end', text=bot_id, values=(
                status, bot.ip_address, bot.generation, 
                f"{bot.fitness_score:.1f}", bot.message_count, last_seen
            ))
            
    def handle_bot_message(self, data):
        """Handle individual bot messages"""
        bot_id = data.get('bot_id')
        if bot_id and bot_id in self.bots:
            self.bots[bot_id].update(data)
            
    def handle_status_update(self, data):
        """Handle status updates from bridge"""
        bots_data = data.get('bots', [])
        if bots_data:
            self.update_bot_list(bots_data)
            
        # Log system stats
        stats = data.get('stats', {})
        uptime = data.get('uptime', 0)
        self.log_message(f"Bridge uptime: {uptime:.1f}s, Messages processed: {stats.get('messages_processed', 0)}")
        
    def handle_bot_offline(self, data):
        """Handle bot going offline"""
        bot_id = data.get('bot_id')
        if bot_id and bot_id in self.bots:
            self.bots[bot_id].is_online = False
            
    def create_mission(self):
        """Create a new mission"""
        mission_type = self.mission_type.get()
        selected_bots = []
        
        # Get selected bots from tree
        for item in self.bot_tree.selection():
            bot_id = self.bot_tree.item(item)['text']
            selected_bots.append(bot_id)
            
        if not selected_bots:
            # Use all online bots if none selected
            selected_bots = [bot_id for bot_id, bot in self.bots.items() if bot.is_online]
            
        if not selected_bots:
            messagebox.showwarning("No Bots", "No bots available for mission")
            return
            
        mission_id = self.mission_planner.create_mission(mission_type, selected_bots)
        self.update_mission_display()
        self.log_message(f"Created mission {mission_id} with {len(selected_bots)} bots")
        
    def update_mission_display(self):
        """Update mission display"""
        # Clear existing missions
        for item in self.mission_tree.get_children():
            self.mission_tree.delete(item)
            
        # Add current missions
        for mission_id, mission in self.mission_planner.missions.items():
            bot_id = mission.assigned_bots
            if isinstance(bot_id, list):
                bot_id = ', '.join(bot_id)
                
            self.mission_tree.insert('', 'end', text=mission_id, values=(
                mission.mission_type, bot_id, mission.status, f"{mission.progress:.0f}%"
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
        
    def periodic_update(self):
        """Periodic update for GUI elements"""
        if self.connected and self.websocket:
            # Request bot list update
            async def send_requests():
                await asyncio.sleep(0.1)
                await self.websocket.send(json.dumps({'type': 'command', 'command': 'get_bots'}))
                
            try:
                loop = asyncio.get_running_loop()
            except RuntimeError:
                # No running loop, skip this update
                pass
            else:
                asyncio.run_coroutine_threadsafe(send_requests(), loop)
            
        # Schedule next update
        self.root.after(10000, self.periodic_update)
        
    def update_connection_status(self, message):
        """Update connection status display"""
        timestamp = datetime.now().strftime('%H:%M:%S')
        self.connection_status.insert(tk.END, f"[{timestamp}] {message}\n")
        self.connection_status.see(tk.END)
        
    def log_message(self, message):
        """Log a message"""
        timestamp = datetime.now().strftime('%H:%M:%S')
        log_entry = f"[{timestamp}] {message}\n"
        self.log_display.insert(tk.END, log_entry)
        self.log_display.see(tk.END)
        logger.info(message)
        
    def clear_logs(self):
        """Clear log display"""
        self.log_display.delete(1.0, tk.END)
        
    def export_logs(self):
        """Export logs to file"""
        try:
            filename = f"jumbo_logs_{datetime.now().strftime('%Y%m%d_%H%M%S')}.txt"
            with open(filename, 'w') as f:
                f.write(self.log_display.get(1.0, tk.END))
            self.log_message(f"Logs exported to {filename}")
        except Exception as e:
            self.log_message(f"Failed to export logs: {e}")
            
    def run(self):
        """Run the application"""
        self.log_message("PC MCU Control Center started")
        self.root.protocol("WM_DELETE_WINDOW", self.on_closing)
        self.root.mainloop()
        
    def on_closing(self):
        """Handle application closing"""
        self.running = False
        if self.connected and self.websocket:
            asyncio.run_coroutine_threadsafe(self.websocket.close(), asyncio.get_event_loop())
        self.root.destroy()

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