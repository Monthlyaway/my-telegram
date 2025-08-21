#!/usr/bin/env python3
"""
MyTelegram Stage 5 Test Client - User System
Test client for user registration and login functionality
"""

import socket
import struct
import sys
import time
import unittest
from typing import Optional

# Import generated protobuf files (run protoc first)
try:
    from protos import messages_pb2
except ImportError:
    print("Error: messages_pb2 not found. Please run:")
    print("protoc --python_out=tests protos/messages.proto")
    sys.exit(1)


class UserSystemClient:
    def __init__(self, host='localhost', port=8080):
        self.host = host
        self.port = port
        self.socket = None

    def connect(self):
        """Connect to server"""
        try:
            self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.socket.connect((self.host, self.port))
            print(f"Connected to {self.host}:{self.port}")
            return True
        except Exception as e:
            print(f"Connection failed: {e}")
            return False

    def disconnect(self):
        """Disconnect from server"""
        if self.socket:
            self.socket.close()
            self.socket = None
            print("Disconnected")

    def send_frame(self, data: bytes) -> bool:
        """Send frame with length prefix"""
        try:
            # Send length prefix (4 bytes, big-endian)
            length = len(data)
            length_bytes = struct.pack('>I', length)
            
            self.socket.sendall(length_bytes + data)
            return True
        except Exception as e:
            print(f"Send failed: {e}")
            return False

    def receive_frame(self) -> Optional[bytes]:
        """Receive frame with length prefix"""
        try:
            # Read length prefix (4 bytes, big-endian)
            length_data = b''
            while len(length_data) < 4:
                chunk = self.socket.recv(4 - len(length_data))
                if not chunk:
                    return None
                length_data += chunk
            
            length = struct.unpack('>I', length_data)[0]
            
            # Read payload
            payload = b''
            while len(payload) < length:
                chunk = self.socket.recv(length - len(payload))
                if not chunk:
                    return None
                payload += chunk
            
            return payload
        except Exception as e:
            print(f"Receive failed: {e}")
            return None

    def register_user(self, username: str, password: str, sequence: int = 1) -> Optional[messages_pb2.Packet]:
        """Register a new user"""
        # Create register request
        packet = messages_pb2.Packet()
        packet.version = 1
        packet.sequence = sequence
        packet.register_request.username = username
        packet.register_request.password = password
        
        # Serialize and send
        data = packet.SerializeToString()
        if not self.send_frame(data):
            return None
        
        # Receive response
        response_data = self.receive_frame()
        if not response_data:
            return None
        
        # Parse response
        response_packet = messages_pb2.Packet()
        response_packet.ParseFromString(response_data)
        return response_packet

    def login_user(self, username: str, password: str, sequence: int = 2) -> Optional[messages_pb2.Packet]:
        """Login a user"""
        # Create login request
        packet = messages_pb2.Packet()
        packet.version = 1
        packet.sequence = sequence
        packet.login_request.username = username
        packet.login_request.password = password
        
        # Serialize and send
        data = packet.SerializeToString()
        if not self.send_frame(data):
            return None
        
        # Receive response
        response_data = self.receive_frame()
        if not response_data:
            return None
        
        # Parse response
        response_packet = messages_pb2.Packet()
        response_packet.ParseFromString(response_data)
        return response_packet


class TestUserSystem(unittest.TestCase):
    """Test cases for user system functionality"""
    
    def setUp(self):
        """Set up test client"""
        self.client = UserSystemClient()
        self.assertTrue(self.client.connect(), "Failed to connect to server")
    
    def tearDown(self):
        """Clean up"""
        self.client.disconnect()
    
    def test_user_registration_success(self):
        """Test successful user registration"""
        print("\n=== Testing User Registration Success ===")
        
        # Generate unique username
        username = f"testuser_{int(time.time())}"
        password = "testpassword123"
        
        response = self.client.register_user(username, password)
        self.assertIsNotNone(response, "No response received")
        self.assertTrue(response.HasField('register_response'), "Response is not a register response")
        
        register_resp = response.register_response
        print(f"Registration result: {register_resp.success}")
        print(f"Message: {register_resp.message}")
        if register_resp.success:
            print(f"User ID: {register_resp.user_id}")
        
        self.assertTrue(register_resp.success, f"Registration failed: {register_resp.message}")
    
    def test_user_registration_duplicate_username(self):
        """Test registration with duplicate username"""
        print("\n=== Testing Duplicate Username Registration ===")
        
        username = "duplicate_test_user"
        password = "testpassword123"
        
        # First registration should succeed
        response1 = self.client.register_user(username, password, sequence=1)
        self.assertIsNotNone(response1, "No response received for first registration")
        
        # Second registration with same username should fail
        response2 = self.client.register_user(username, password, sequence=2)
        self.assertIsNotNone(response2, "No response received for second registration")
        self.assertTrue(response2.HasField('register_response'), "Response is not a register response")
        
        register_resp2 = response2.register_response
        print(f"Second registration result: {register_resp2.success}")
        print(f"Message: {register_resp2.message}")
        
        self.assertFalse(register_resp2.success, "Second registration should have failed")
        self.assertIn("exists", register_resp2.message.lower(), "Error message should mention username exists")
    
    def test_user_login_success(self):
        """Test successful user login"""
        print("\n=== Testing User Login Success ===")
        
        # First register a user
        username = f"logintest_{int(time.time())}"
        password = "loginpassword123"
        
        reg_response = self.client.register_user(username, password, sequence=1)
        self.assertIsNotNone(reg_response, "Registration failed")
        self.assertTrue(reg_response.register_response.success, "Registration failed")
        
        # Now try to login
        login_response = self.client.login_user(username, password, sequence=2)
        self.assertIsNotNone(login_response, "No login response received")
        self.assertTrue(login_response.HasField('login_response'), "Response is not a login response")
        
        login_resp = login_response.login_response
        print(f"Login result: {login_resp.success}")
        print(f"Message: {login_resp.message}")
        if login_resp.success:
            print(f"User ID: {login_resp.user_id}")
            print(f"Username: {login_resp.username}")
        
        self.assertTrue(login_resp.success, f"Login failed: {login_resp.message}")
        self.assertEqual(login_resp.username, username, "Username mismatch")
    
    def test_user_login_wrong_password(self):
        """Test login with wrong password"""
        print("\n=== Testing Login with Wrong Password ===")
        
        # First register a user
        username = f"wrongpasstest_{int(time.time())}"
        password = "correctpassword123"
        
        reg_response = self.client.register_user(username, password, sequence=1)
        self.assertIsNotNone(reg_response, "Registration failed")
        self.assertTrue(reg_response.register_response.success, "Registration failed")
        
        # Try to login with wrong password
        wrong_password = "wrongpassword123"
        login_response = self.client.login_user(username, wrong_password, sequence=2)
        self.assertIsNotNone(login_response, "No login response received")
        self.assertTrue(login_response.HasField('login_response'), "Response is not a login response")
        
        login_resp = login_response.login_response
        print(f"Login result: {login_resp.success}")
        print(f"Message: {login_resp.message}")
        
        self.assertFalse(login_resp.success, "Login should have failed with wrong password")
        self.assertIn("password", login_resp.message.lower(), "Error message should mention password")
    
    def test_user_login_nonexistent_user(self):
        """Test login with non-existent user"""
        print("\n=== Testing Login with Non-existent User ===")
        
        username = f"nonexistent_{int(time.time())}"
        password = "somepassword123"
        
        login_response = self.client.login_user(username, password)
        self.assertIsNotNone(login_response, "No login response received")
        self.assertTrue(login_response.HasField('login_response'), "Response is not a login response")
        
        login_resp = login_response.login_response
        print(f"Login result: {login_resp.success}")
        print(f"Message: {login_resp.message}")
        
        self.assertFalse(login_resp.success, "Login should have failed for non-existent user")
        self.assertIn("not found", login_resp.message.lower(), "Error message should mention user not found")
    
    def test_invalid_username_registration(self):
        """Test registration with invalid username"""
        print("\n=== Testing Invalid Username Registration ===")
        
        # Test short username
        response = self.client.register_user("ab", "validpassword123")
        self.assertIsNotNone(response, "No response received")
        self.assertTrue(response.HasField('register_response'), "Response is not a register response")
        
        register_resp = response.register_response
        print(f"Short username result: {register_resp.success}")
        print(f"Message: {register_resp.message}")
        
        self.assertFalse(register_resp.success, "Registration should have failed with short username")


def run_interactive_test():
    """Run interactive test"""
    print("=== Interactive User System Test ===")
    
    client = UserSystemClient()
    if not client.connect():
        return
    
    try:
        while True:
            print("\nOptions:")
            print("1. Register user")
            print("2. Login user")
            print("3. Exit")
            
            choice = input("Choose option (1-3): ").strip()
            
            if choice == '1':
                username = input("Username: ").strip()
                password = input("Password: ").strip()
                
                response = client.register_user(username, password)
                if response and response.HasField('register_response'):
                    resp = response.register_response
                    print(f"Registration {'successful' if resp.success else 'failed'}: {resp.message}")
                    if resp.success:
                        print(f"User ID: {resp.user_id}")
                else:
                    print("No valid response received")
            
            elif choice == '2':
                username = input("Username: ").strip()
                password = input("Password: ").strip()
                
                response = client.login_user(username, password)
                if response and response.HasField('login_response'):
                    resp = response.login_response
                    print(f"Login {'successful' if resp.success else 'failed'}: {resp.message}")
                    if resp.success:
                        print(f"User ID: {resp.user_id}")
                        print(f"Username: {resp.username}")
                else:
                    print("No valid response received")
            
            elif choice == '3':
                break
            
            else:
                print("Invalid choice")
    
    finally:
        client.disconnect()


def main():
    if len(sys.argv) > 1 and sys.argv[1] == "interactive":
        run_interactive_test()
    else:
        # Run unit tests
        unittest.main(verbosity=2)


if __name__ == "__main__":
    main()