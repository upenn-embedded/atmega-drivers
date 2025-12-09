import socket
import threading

# Server configuration
HOST = '0.0.0.0'  # Listen on all network interfaces
PORT = 7040

def handle_client(conn, addr):
    """Handle individual client connections"""
    print(f"New connection from {addr}")
    
    try:
        # Receive data from client
        data = conn.recv(1024)
        if data:
            message = data.decode('utf-8').strip()
            print(f"Received from {addr}: {message}")
            
            # Send response back to client
            response = f"Message received: {message}"
            conn.sendall(response.encode('utf-8') + b'\n')
            print(f"Sent response to {addr}")
    
    except Exception as e:
        print(f"Error handling client {addr}: {e}")
    
    finally:
        conn.close()
        print(f"Connection closed: {addr}")

def start_server():
    """Start the socket server"""
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as server_socket:
        # Allow reuse of address
        server_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        
        # Bind to host and port
        server_socket.bind((HOST, PORT))
        server_socket.listen(5)
        
        print(f"Server listening on {HOST}:{PORT}")
        print("Waiting for ESP32 connections...")
        
        try:
            while True:
                # Accept incoming connections
                conn, addr = server_socket.accept()
                
                # Handle each client in a separate thread
                client_thread = threading.Thread(target=handle_client, args=(conn, addr))
                client_thread.daemon = True
                client_thread.start()
        
        except KeyboardInterrupt:
            print("\nServer shutting down...")

if __name__ == "__main__":
    print("Python Socket Server for ESP32")
    print("=" * 40)
    
    # Get local IP address
    hostname = socket.gethostname()
    local_ip = socket.gethostbyname(hostname)
    print(f"Your computer's IP address: {local_ip}")
    print(f"Use this IP in your ESP32 code!")
    print("=" * 40)
    
    start_server()
