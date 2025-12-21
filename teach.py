import socket
import time
import argparse

def teach(host='localhost', port=9005, file_path='teach.txt'):
    print(f"Connecting to Brain at {host}:{port}...")
    try:
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s.connect((host, port))
        
        with open(file_path, 'r') as f:
            lines = f.readlines()
            
        print(f"Teaching {len(lines)} items...")
        for line in lines:
            line = line.strip()
            if not line or line.startswith('#'): continue
            
            print(f"> {line}")
            s.sendall((line + "\n").encode())
            
            # Wait for brain to process/digest
            time.sleep(1.0) 
            
            # Read acknowledgement (optional, mostly to clear buffer)
            try:
                s.settimeout(1.0)
                data = s.recv(1024)
                print(f"< {data.decode().strip()}")
            except socket.timeout:
                pass
                
        print("Done.")
        s.close()
    except Exception as e:
        print(f"Error: {e}")

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument('--file', default='teach.txt', help='File to load')
    args = parser.parse_args()
    
    teach(file_path=args.file)
