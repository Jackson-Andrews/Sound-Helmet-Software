import socket
import struct
import time

import sounddevice as sd


RATE = 44100
CHUNK = 512
DEST_IP = "172.21.124.30"
DEST_PORT = 12345


def find_blackhole_device():
    for index, device in enumerate(sd.query_devices()):
        if "BlackHole" in device["name"] and device["max_input_channels"] >= 2:
            print(f"Using input device {index}: {device['name']}")
            return index
    raise RuntimeError("No suitable input device with 'BlackHole' found")


device_index = find_blackhole_device()
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sequence = 0
start_time = time.time()


def callback(indata, frames, time_info, status):
    global sequence

    if status:
        print("Input stream status:", status)

    if not indata.any():
        print("Received an all-zero audio chunk")
        return

    data = (indata * 127 + 128).clip(0, 255).astype("uint8")
    payload = data.tobytes()
    timestamp_ms = int((time.time() - start_time) * 1000)

    header = struct.pack(">HI", sequence, timestamp_ms)
    packet = header + payload
    sock.sendto(packet, (DEST_IP, DEST_PORT))

    print(f"Sent UDP packet: {len(packet)} bytes")
    print(f"Header -> seq: {sequence}, timestamp_ms: {timestamp_ms}")
    print(f"Payload first 16 bytes: {list(payload[:16])}")

    sequence = (sequence + 1) % 65536


with sd.InputStream(
    device=device_index,
    channels=2,
    samplerate=RATE,
    blocksize=CHUNK,
    callback=callback,
):
    print("Streaming audio to ESP32. Press Ctrl+C to stop.")
    while True:
        time.sleep(1)
