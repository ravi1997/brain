import socket
import time
import os
import subprocess

def test_connection(port, auth_line=None):
    try:
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s.settimeout(2)
        s.connect(('localhost', port))
        
        if auth_line:
            s.sendall(auth_line.encode() + b'\n')
            response = s.recv(1024).decode()
            print(f"Auth response: {response.strip()}")
            return "AUTH_OK" in response
        else:
            # Send something to see if we get disconnected
            s.sendall(b"Hello\n")
            time.sleep(0.5)
            response = s.recv(1024).decode()
            if "AUTH_FAILED" in response:
                print("Connection rejected: Auth required")
                return False
            print(f"Response: {response.strip()}")
            return True
    except Exception as e:
        print(f"Error: {e}")
        return False
    finally:
        s.close()

if __name__ == "__main__":
    print("--- Test 1: No Token (Disabled) ---")
    # Start server without BRAIN_TOKEN
    env = os.environ.copy()
    if "BRAIN_TOKEN" in env: del env["BRAIN_TOKEN"]
    
    executable = "/home/ravi/workspace/brain/build/brain_replica"
    server_proc = subprocess.Popen([executable], env=env, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    time.sleep(2) # Wait for start
    
    success = test_connection(9005) # Chat port
    print(f"No-auth success: {success}")
    
    server_proc.terminate()
    stdout, stderr = server_proc.communicate()
    print("Server output (Test 1):")
    print(stdout.decode())

    print("\n--- Test 2: With Token ---")
    env["BRAIN_TOKEN"] = "secret123"
    server_proc = subprocess.Popen([executable], env=env, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    time.sleep(2)
    
    print("Testing without info...")
    success_none = test_connection(9005)
    print(f"Forbidden success: {success_none}")
    
    print("Testing with wrong token...")
    success_wrong = test_connection(9005, "AUTH wrong")
    print(f"Wrong token success: {success_wrong}")
    
    print("Testing with correct token...")
    success_correct = test_connection(9005, "AUTH secret123")
    print(f"Correct token success: {success_correct}")
    
    server_proc.terminate()
    stdout, stderr = server_proc.communicate()
    print("Server output (Test 2):")
    print(stdout.decode())
