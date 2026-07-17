#!/usr/bin/env python3
"""
STM32 <-> USB ACM Serial Bridge (Fixed 921600 baud)
STM32 physical UART: /dev/ttyHS1
USB Gadget virtual serial: /dev/ttyGS0
"""

import serial
import threading
import time
import sys
import signal

# ============ Configuration ============
STM32_PORT = '/dev/ttyHS1'      # Physical UART between Debian and STM32
GADGET_PORT = '/dev/ttyGS0'     # USB Gadget ACM virtual serial port
BAUDRATE = 921600               # Baudrate
# =======================================

class SerialBridge:
    def __init__(self):
        self.running = True
        
        # Register signal handlers
        signal.signal(signal.SIGINT, self.signal_handler)
        signal.signal(signal.SIGTERM, self.signal_handler)

        # Open STM32 physical UART
        try:
            self.stm32 = serial.Serial(
                port=STM32_PORT,
                baudrate=BAUDRATE,
                bytesize=serial.EIGHTBITS,
                parity=serial.PARITY_NONE,
                stopbits=serial.STOPBITS_ONE,
                timeout=0.01
            )
            print(f"[OK] Opened {STM32_PORT} @ {BAUDRATE} baud")
        except serial.SerialException as e:
            print(f"[ERROR] Cannot open {STM32_PORT}: {e}")
            self.list_available_ports()
            sys.exit(1)
        
        # Open USB Gadget ACM serial port
        try:
            self.gadget = serial.Serial(
                port=GADGET_PORT,
                baudrate=BAUDRATE,
                bytesize=serial.EIGHTBITS,
                parity=serial.PARITY_NONE,
                stopbits=serial.STOPBITS_ONE,
                timeout=0.01
            )
            print(f"[OK] Opened {GADGET_PORT} @ {BAUDRATE} baud")
        except serial.SerialException as e:
            print(f"[ERROR] Cannot open {GADGET_PORT}: {e}")
            sys.exit(1)
        
        # Statistics
        self.stm32_to_pc_bytes = 0
        self.pc_to_stm32_bytes = 0
        self.last_stats_time = time.time()

    def signal_handler(self, sig, frame):
        """Handle exit signal"""
        print("\n[*] Received exit signal...")
        self.running = False

    def list_available_ports(self):
        """List available serial ports"""
        import glob
        print("\nAvailable serial ports:")
        for port in glob.glob('/dev/tty*'):
            try:
                s = serial.Serial(port)
                s.close()
                print(f"  {port}")
            except:
                pass

    def bridge_forward(self, src, dst, name):
        """Unidirectional transparent data forwarding"""
        while self.running:
            try:
                if src.in_waiting:
                    data = src.read(src.in_waiting)
                    # Forward directly, no protocol parsing
                    dst.write(data)
                    dst.flush()
                    
                    # Update statistics
                    if name == "STM32>>PC":
                        self.stm32_to_pc_bytes += len(data)
                    else:
                        self.pc_to_stm32_bytes += len(data)
            except serial.SerialException as e:
                print(f"[ERROR] {name} serial error: {e}")
                time.sleep(0.1)
            except Exception as e:
                print(f"[ERROR] {name}: {e}")
                time.sleep(0.1)
            
            time.sleep(0.0001)  # 100us delay to avoid high CPU usage
    
    def stats_printer(self):
        """Periodically print statistics"""
        while self.running:
            time.sleep(5)
            elapsed = time.time() - self.last_stats_time
            if elapsed > 0:
                stm32_rate = self.stm32_to_pc_bytes / elapsed
                pc_rate = self.pc_to_stm32_bytes / elapsed
                print(f"\n[STATS] STM32->PC: {self.stm32_to_pc_bytes} bytes ({stm32_rate:.0f} B/s) | "
                      f"PC->STM32: {self.pc_to_stm32_bytes} bytes ({pc_rate:.0f} B/s)")
                self.stm32_to_pc_bytes = 0
                self.pc_to_stm32_bytes = 0
                self.last_stats_time = time.time()
    
    def run(self):
        print(f"\n{'='*50}")
        print(f"  STM32 Serial Bridge")
        print(f"  {STM32_PORT} <--> {GADGET_PORT}")
        print(f"  Fixed Baudrate: {BAUDRATE}")
        print(f"{'='*50}")
        print("[*] Bridge running. Press Ctrl+C to stop.\n")
        
        # Create forwarding threads
        t1 = threading.Thread(
            target=self.bridge_forward, 
            args=(self.stm32, self.gadget, "STM32>>PC"), 
            daemon=True
        )
        t2 = threading.Thread(
            target=self.bridge_forward, 
            args=(self.gadget, self.stm32, "PC>>STM32"), 
            daemon=True
        )
        t3 = threading.Thread(target=self.stats_printer, daemon=True)
        
        t1.start()
        t2.start()
        t3.start()
        
        try:
            while self.running:
                time.sleep(0.5)
        except KeyboardInterrupt:
            print("\n[*] Keyboard interrupt...")
            self.running = False
        
        # Wait for threads to finish
        t1.join(timeout=2)
        t2.join(timeout=2)
        t3.join(timeout=2)
        
        # Close serial ports
        self.stm32.close()
        self.gadget.close()
        print("[*] Serial ports closed")
        print("[*] Bridge stopped")

if __name__ == "__main__":
    # Check if pyserial is installed
    try:
        import serial
    except ImportError:
        print("pyserial not installed. Installing...")
        import subprocess
        subprocess.check_call([sys.executable, "-m", "pip", "install", "pyserial"])
        import serial
    
    bridge = SerialBridge()
    bridge.run()