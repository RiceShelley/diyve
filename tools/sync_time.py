#!/usr/bin/env python3
"""
Send the current PC time to the diyve dive computer over UART.
Waits for the device to signal it is ready (SYNC?) then sends the time packet.

Usage: python3 sync_time.py <serial_port> [baud_rate]
Example: python3 sync_time.py /dev/ttyACM0
"""

import sys
import time
import datetime

try:
    import serial
except ImportError:
    print("Error: pyserial not installed. Run: pip install pyserial")
    sys.exit(1)

port = sys.argv[1] if len(sys.argv) > 1 else "/dev/ttyACM0"
baud = int(sys.argv[2]) if len(sys.argv) > 2 else 115200
wait_timeout = 30  # seconds to wait for SYNC? signal

print(f"Connecting to {port} at {baud} baud...")
try:
    with serial.Serial(port, baud, timeout=1) as ser:
        print("Waiting for device to signal ready (SYNC?)...")
        deadline = time.time() + wait_timeout
        synced = False

        while time.time() < deadline:
            line = ser.readline().decode(errors="replace").strip()
            if line:
                print(f"Device: {line}")
            if line == "SYNC?":
                # Send current time immediately after receiving the signal
                now = datetime.datetime.now()
                dotw = now.isoweekday()  # 1=Monday, 7=Sunday, matches DS3231
                packet = f"TIME:{now.strftime('%Y-%m-%dT%H:%M:%S')}:{dotw}\n"
                ser.write(packet.encode())
                ser.flush()
                print(f"Sent: {packet.strip()}")

                # Read confirmation
                response = ser.readline().decode(errors="replace").strip()
                if response:
                    print(f"Device: {response}")
                synced = True
                break

        if not synced:
            print("Timed out waiting for SYNC? signal — is the device connected?")
            sys.exit(1)

except serial.SerialException as e:
    print(f"Error opening {port}: {e}")
    sys.exit(1)
