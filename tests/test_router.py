#!/usr/bin/env python3
"""
MyTelegram Stage 4 Test Client - Message Router System
Test client for message routing and session management
"""

import socket
import struct
import sys
import time
from typing import Optional

# Import generated protobuf files (run protoc first)
try:
    from protos import messages_pb2
except ImportError:
    print("Error: messages_pb2 not found. Please run:")
    print("protoc --python_out=tests protos/messages.proto")
    sys.exit(1)


class RouterTestClient:
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
            print(f"Failed to connect: {e}")
            return False

    def disconnect(self):
        """Disconnect from server"""
        if self.socket:
            self.socket.close()
            self.socket = None
            print("Disconnected")

    def send_packet(self, packet: messages_pb2.Packet) -> bool:
        """Send protobuf packet using frame protocol"""
        try:
            # Serialize packet to bytes
            packet_data = packet.SerializeToString()

            # Create frame: [4-byte length][protobuf data]
            frame_length = len(packet_data)
            # Network byte order (big-endian)
            frame_header = struct.pack('!I', frame_length)

            # Send frame
            frame = frame_header + packet_data
            self.socket.sendall(frame)

            print(f"Sent packet: version={packet.version}, sequence={packet.sequence}, "
                  f"payload type={packet.WhichOneof('payload')}")
            return True
        except Exception as e:
            print(f"Failed to send packet: {e}")
            return False

    def receive_packet(self, timeout=5.0) -> Optional[messages_pb2.Packet]:
        """Receive protobuf packet from server"""
        try:
            self.socket.settimeout(timeout)

            # Read frame header (4 bytes)
            header_data = self._receive_exactly(4)
            if not header_data:
                return None

            # Parse frame length
            frame_length = struct.unpack('!I', header_data)[0]
            print(f"Expecting frame of {frame_length} bytes")

            # Read frame data
            packet_data = self._receive_exactly(frame_length)
            if not packet_data:
                return None

            # Parse protobuf packet
            packet = messages_pb2.Packet()
            packet.ParseFromString(packet_data)

            print(f"Received packet: version={packet.version}, sequence={packet.sequence}, "
                  f"payload type={packet.WhichOneof('payload')}")
            return packet

        except socket.timeout:
            print("Timeout waiting for response")
            return None
        except Exception as e:
            print(f"Failed to receive packet: {e}")
            return None

    def _receive_exactly(self, num_bytes: int) -> Optional[bytes]:
        """Receive exactly num_bytes from socket"""
        data = b''
        while len(data) < num_bytes:
            chunk = self.socket.recv(num_bytes - len(data))
            if not chunk:
                print("Connection closed by server")
                return None
            data += chunk
        return data


def test_echo_routing(client: RouterTestClient, message: str, sequence: int = 1):
    """Test echo message routing through MessageRouter"""
    print(f"\n=== Testing Echo Routing: '{message}' ===")

    # Create echo request packet
    packet = messages_pb2.Packet()
    packet.version = 1
    packet.sequence = sequence
    packet.echo_request.content = message

    # Send packet
    if not client.send_packet(packet):
        return False

    # Receive response
    response = client.receive_packet()
    if not response:
        return False

    if response.HasField('echo_response'):
        echo_response = response.echo_response
        print(f"Echo Response: '{echo_response.content}'")
        if echo_response.content == message:
            print("✅ Success: Echo routing works correctly")
            return True
        else:
            print(
                f"❌ Content mismatch: expected '{message}', got '{echo_response.content}'")
            return False
    elif response.HasField('error'):
        error = response.error
        print(
            f"❌ Error Response: code={error.error_code}, message='{error.message}'")
        return False
    else:
        print(f"❌ Unexpected response type: {response.WhichOneof('payload')}")
        return False


def test_unknown_message_type(client: RouterTestClient):
    """Test routing of unknown message types"""
    print(f"\n=== Testing Unknown Message Type Handling ===")

    # Create packet with only version and sequence (no payload)
    packet = messages_pb2.Packet()
    packet.version = 1
    packet.sequence = 999
    # Intentionally not setting any payload

    # Send packet
    if not client.send_packet(packet):
        return False

    # Receive response (should be error)
    response = client.receive_packet()
    if not response:
        return False

    if response.HasField('error'):
        error = response.error
        print(
            f"Error Response: code={error.error_code}, message='{error.message}'")
        print("✅ Success: Unknown message type correctly handled")
        return True
    else:
        print(
            f"❌ Expected error response, got: {response.WhichOneof('payload')}")
        return False


def test_multiple_sessions():
    """Test session management with multiple concurrent connections"""
    print(f"\n=== Testing Multiple Session Management ===")

    clients = []
    success_count = 0

    try:
        # Create 3 concurrent connections
        for i in range(3):
            client = RouterTestClient()
            if client.connect():
                clients.append(client)
                print(f"Client {i+1} connected successfully")
            else:
                print(f"Client {i+1} failed to connect")

        # Test echo with each client
        for i, client in enumerate(clients):
            if test_echo_routing(client, f"Hello from client {i+1}", i+10):
                success_count += 1

        # Close all connections
        for i, client in enumerate(clients):
            client.disconnect()
            print(f"Client {i+1} disconnected")

        print(
            f"Session test: {success_count}/{len(clients)} clients successful")
        return success_count == len(clients)

    except Exception as e:
        print(f"Session management test failed: {e}")
        return False


def test_invalid_protocol_version(client: RouterTestClient):
    """Test invalid protocol version handling"""
    print(f"\n=== Testing Invalid Protocol Version ===")

    # Create packet with invalid version
    packet = messages_pb2.Packet()
    packet.version = 999  # Invalid version
    packet.sequence = 100
    packet.echo_request.content = "test with invalid version"

    # Send packet
    if not client.send_packet(packet):
        return False

    # Receive response (should be error)
    response = client.receive_packet()
    if not response:
        return False

    if response.HasField('error'):
        error = response.error
        print(
            f"Error Response: code={error.error_code}, message='{error.message}'")
        print("✅ Success: Invalid version correctly handled")
        return True
    else:
        print(
            f"❌ Expected error response, got: {response.WhichOneof('payload')}")
        return False


def main():
    print("MyTelegram Stage 4 - Message Router System Test Client")
    print("=" * 60)

    client = RouterTestClient()

    # Connect to server
    if not client.connect():
        return 1

    try:
        # Test cases
        success_count = 0
        total_tests = 0

        # Test 1: Basic echo routing
        total_tests += 1
        if test_echo_routing(client, "Hello MessageRouter!", 1):
            success_count += 1

        # Test 2: Empty message routing
        total_tests += 1
        if test_echo_routing(client, "", 2):
            success_count += 1

        # Test 3: Unicode message routing
        total_tests += 1
        if test_echo_routing(client, "你好路由器! 🚀", 3):
            success_count += 1

        # Test 4: Unknown message type
        total_tests += 1
        if test_unknown_message_type(client):
            success_count += 1

        # Test 5: Invalid protocol version
        total_tests += 1
        if test_invalid_protocol_version(client):
            success_count += 1

    finally:
        client.disconnect()

    # Test 6: Multiple session management (separate test)
    total_tests += 1
    if test_multiple_sessions():
        success_count += 1

    # Results
    print(f"\n" + "=" * 60)
    print(f"Router Test Results: {success_count}/{total_tests} tests passed")

    if success_count == total_tests:
        print("🎉 All router tests passed!")
        print("\n✅ MessageRouter System Verification:")
        print("  • Echo messages correctly routed to EchoHandler")
        print("  • Unknown message types return proper errors")
        print("  • Session management works with multiple clients")
        print("  • Protocol version validation functioning")
        print("  • Error handling and logging operational")
        return 0
    else:
        print(f"❌ {total_tests - success_count} tests failed")
        return 1


if __name__ == '__main__':
    sys.exit(main())
