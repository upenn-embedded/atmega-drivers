import socket
import threading
import pyautogui
import time
import serial

# --- Configuration ---
HOST = '0.0.0.0'
PORT = 7040

# Serial port for ESP32 (fan control) - UPDATE THIS TO YOUR PORT
ESP32_SERIAL_PORT = '/dev/cu.usbmodem21401'  # Change to your ESP32 port
ESP32_BAUD_RATE = 115200

# Global serial connection
esp32_serial = None

# Speed up PyAutoGUI to not pause
pyautogui.PAUSE = 0 

PULSE_INTERVAL = 0.15  # Total duration of one pulse cycle
DUTY_CYCLE = 0.6       # Percentage of time the key is pressed during a cycle (0.5 = 50%)

# --- Global State ---
# Hand possibilities: 'A', 'a', 'D', 'd', 'N'
# Feet possibilities: 'W', 'w', 'N', 's', 'S'
hand_command = 'N'
feet_command = 'N'
last_feet_sent = None  # Track last command sent to ESP32

def init_esp32_serial():
    """Initialize serial connection to ESP32 for fan control"""
    global esp32_serial
    try:
        esp32_serial = serial.Serial(ESP32_SERIAL_PORT, ESP32_BAUD_RATE, timeout=1)
        print(f"Connected to ESP32 on {ESP32_SERIAL_PORT}")
        return True
    except serial.SerialException as e:
        print(f"Warning: Could not connect to ESP32: {e}")
        print("Fan control will be disabled.")
        return False

def send_to_esp32(feet_cmd):
    """Send feet command to ESP32 for fan control"""
    global esp32_serial, last_feet_sent
    
    # Only send if command changed
    if feet_cmd == last_feet_sent:
        return
    
    # Map feet command to fan speed character
    # 'W' (fast forward) -> 'W' (high fan speed)
    # 'w' (slow forward) -> 'w' (low fan speed)  
    # 'S' (fast backward) -> 's' (medium fan speed)
    # 's' (slow backward) -> 's' (medium fan speed)
    # 'N' (neutral) -> 'x' (stop fan)
    fan_char = feet_cmd
    if feet_cmd == 'N':
        fan_char = 'x'  # Stop fan when neutral
    elif feet_cmd == 'S':
        fan_char = 's'  # Map fast backward to medium
    
    if esp32_serial and esp32_serial.is_open:
        try:
            esp32_serial.write(fan_char.encode())
            last_feet_sent = feet_cmd
            print(f"Sent to ESP32: {fan_char} (feet: {feet_cmd})")
        except serial.SerialException as e:
            print(f"Error sending to ESP32: {e}")

def control_loop():
    """
    This runs in the background constantly. 
    It checks 'hand_command' and 'feet_command' and presses/releases keys accordingly.
    """
    global hand_command, feet_command
    
    # Track what keys are currently physically held down on the computer
    hand_held_key = None
    feet_held_key = None
    
    print("Control Loop Started")

    while True:
        # --- HAND CONTROL (a/d keys) ---
        hand_target_key = None
        hand_mode = 'release'

        cmd = hand_command
        
        if cmd == 'A':
            hand_target_key = 'a'
            hand_mode = 'hold'
        elif cmd == 'a':
            hand_target_key = 'a'
            hand_mode = 'pulse'
        elif cmd == 'D':
            hand_target_key = 'd'
            hand_mode = 'hold'
        elif cmd == 'd':
            hand_target_key = 'd'
            hand_mode = 'pulse'
        elif cmd == 'N':
            hand_mode = 'release'

        # Execute Hand Logic
        if hand_held_key is not None and hand_held_key != hand_target_key:
            pyautogui.keyUp(hand_held_key)
            print(f"Hand released {hand_held_key}")
            hand_held_key = None

        if hand_mode == 'hold':
            if hand_held_key != hand_target_key:
                pyautogui.keyDown(hand_target_key)
                hand_held_key = hand_target_key
                print(f"Hand HOLDING {hand_target_key} (Fast)")

        elif hand_mode == 'pulse':
            cycle_time = time.time() % PULSE_INTERVAL
            is_on_part_of_cycle = cycle_time < (PULSE_INTERVAL * DUTY_CYCLE)

            if is_on_part_of_cycle:
                if hand_held_key != hand_target_key:
                    pyautogui.keyDown(hand_target_key)
                    hand_held_key = hand_target_key
            else:
                if hand_held_key == hand_target_key:
                    pyautogui.keyUp(hand_target_key)
                    hand_held_key = None

        elif hand_mode == 'release':
            if hand_held_key is not None:
                pyautogui.keyUp(hand_held_key)
                hand_held_key = None
                print("Hand Neutral")

        # --- FEET CONTROL (w/s keys) ---
        feet_target_key = None
        feet_mode = 'release'

        cmd = feet_command
        
        if cmd == 'W':
            feet_target_key = 'w'
            feet_mode = 'hold'
        elif cmd == 'w':
            feet_target_key = 'w'
            feet_mode = 'pulse'
        elif cmd == 'S':
            feet_target_key = 's'
            feet_mode = 'hold'
        elif cmd == 's':
            feet_target_key = 's'
            feet_mode = 'pulse'
        elif cmd == 'N':
            feet_mode = 'release'

        # Execute Feet Logic
        if feet_held_key is not None and feet_held_key != feet_target_key:
            pyautogui.keyUp(feet_held_key)
            print(f"Feet released {feet_held_key}")
            feet_held_key = None

        if feet_mode == 'hold':
            if feet_held_key != feet_target_key:
                pyautogui.keyDown(feet_target_key)
                feet_held_key = feet_target_key
                print(f"Feet HOLDING {feet_target_key} (Fast)")

        elif feet_mode == 'pulse':
            cycle_time = time.time() % PULSE_INTERVAL
            is_on_part_of_cycle = cycle_time < (PULSE_INTERVAL * DUTY_CYCLE)

            if is_on_part_of_cycle:
                if feet_held_key != feet_target_key:
                    pyautogui.keyDown(feet_target_key)
                    feet_held_key = feet_target_key
            else:
                if feet_held_key == feet_target_key:
                    pyautogui.keyUp(feet_target_key)
                    feet_held_key = None

        elif feet_mode == 'release':
            if feet_held_key is not None:
                pyautogui.keyUp(feet_held_key)
                feet_held_key = None
                print("Feet Neutral")

        # Send feet command to ESP32 for fan control
        send_to_esp32(feet_command)

        # Sleep a tiny amount to prevent CPU 100% usage, but fast enough for smooth input
        time.sleep(0.005) 

def handle_client(conn, addr):
    """Network Thread: Only listens for data and updates global state"""
    global hand_command, feet_command
    print(f"New connection from {addr}")

    try:
        while True:
            data = conn.recv(1024)
            if not data: break 
            
            raw_message = data.decode('utf-8', errors='ignore').strip()
            if not raw_message: continue

            # Parse the message format: "hand: X" or "feet: X"
            if raw_message.startswith("hand:"):
                # Extract the character after "hand: "
                value = raw_message[5:].strip()
                if value and value[0] in ['A', 'a', 'D', 'd', 'N']:
                    hand_command = value[0]
            elif raw_message.startswith("feet:"):
                # Extract the character after "feet: "
                value = raw_message[5:].strip()
                if value and value[0] in ['W', 'w', 'N', 's', 'S']:
                    feet_command = value[0]
            
    except Exception as e:
        print(f"Error: {e}")
    finally:
        hand_command = 'N' # Safety: Stop car if connection dies
        feet_command = 'N'
        conn.close()
        print(f"Connection closed: {addr}")

def start_server():
    # Initialize ESP32 serial connection for fan control
    init_esp32_serial()
    
    # Start the Control Loop in a separate thread
    control_thread = threading.Thread(target=control_loop, daemon=True)
    control_thread.start()

    # 2. Start the Network Listener
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        s.bind((HOST, PORT))
        s.listen(5)
        print("="*40)
        print(f"Server Listening on {HOST}:{PORT}")
        print("Waiting for ESP32...")
        print("="*40)
        
        while True:
            conn, addr = s.accept()
            # Handle client in a thread so we can accept reconnects
            t = threading.Thread(target=handle_client, args=(conn, addr), daemon=True)
            t.start()

if __name__ == "__main__":
    start_server()