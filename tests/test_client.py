#!/usr/bin/env python3
"""
MyTelegram Stage 3 Test Client
Test client for protobuf protocol communication
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


class ProtocolClient:
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

            print(
                f"Sent packet: version={packet.version}, payload type={packet.WhichOneof('payload')}")
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

            print(
                f"Received packet: version={packet.version}, sequence={packet.sequence}")
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


def test_echo_request(client: ProtocolClient, message: str, sequence: int = 1):
    """Test echo request/response"""
    print(f"\n=== Testing Echo: '{message}' ===")

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
        print(f"✅ Success: Got expected echo response")
        return True
    elif response.HasField('error'):
        error = response.error
        print(
            f"❌ Error Response: code={error.error_code}, message='{error.message}'")
        return False
    else:
        print(f"❌ Unexpected response type: {response.WhichOneof('payload')}")
        return False


def test_invalid_version(client: ProtocolClient):
    """Test invalid protocol version handling"""
    print(f"\n=== Testing Invalid Version ===")

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
        print(f"✅ Success: Got expected error response for invalid version")
        return True
    else:
        print(
            f"❌ Expected error response, got: {response.WhichOneof('payload')}")
        return False


def main():
    print("MyTelegram Stage 3 - Protobuf Protocol Test Client")
    print("=" * 50)

    client = ProtocolClient()

    # Connect to server
    if not client.connect():
        return 1

    try:
        # Test cases
        success_count = 0
        total_tests = 0

        # Test 1: Basic echo
        total_tests += 1
        if test_echo_request(client, "Hello MyTelegram!", 1):
            success_count += 1

        # Test 2: Empty message
        total_tests += 1
        if test_echo_request(client, "", 2):
            success_count += 1

        # Test 3: Long message
        total_tests += 1
        long_message = "A" * 1000
        if test_echo_request(client, long_message, 3):
            success_count += 1

        # Test 4: Unicode message
        total_tests += 1
        if test_echo_request(client, "你好世界! 🌟", 4):
            success_count += 1

        # Test 5: Invalid protocol version
        total_tests += 1
        if test_invalid_version(client):
            success_count += 1

        # Results
        print(f"\n" + "=" * 50)
        print(f"Test Results: {success_count}/{total_tests} tests passed")

        if success_count == total_tests:
            print("🎉 All tests passed!")
            return 0
        else:
            print(f"❌ {total_tests - success_count} tests failed")
            return 1

    finally:
        client.disconnect()


if __name__ == '__main__':
    sys.exit(main())
