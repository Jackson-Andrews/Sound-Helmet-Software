import numpy as np
import sounddevice as sd
import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation


RATE = 44100
CHUNK = 256


def find_blackhole_device():
    for index, device in enumerate(sd.query_devices()):
        if "BlackHole" in device["name"] and device["max_input_channels"] >= 2:
            print(f"Using input device {index}: {device['name']}")
            return index
    raise RuntimeError("No suitable input device with 'BlackHole' found")


device_index = find_blackhole_device()

fig, ax = plt.subplots()
x = np.arange(CHUNK)
y = np.zeros(CHUNK)
line_left, = ax.plot(x, y, label="Left", color="magenta")
line_right, = ax.plot(x, y, label="Right", color="cyan")
ax.set_ylim(0, 255)
ax.set_xlim(0, CHUNK)
ax.legend()
ax.set_title("Live Audio Input from BlackHole")
ax.set_xlabel("Sample")
ax.set_ylabel("Amplitude (uint8)")

latest_frame = np.zeros((CHUNK, 2), dtype=np.uint8)


def audio_callback(indata, frames, time_info, status):
    if status:
        print("Input stream status:", status)
    latest_frame[:] = (indata * 127 + 128).clip(0, 255).astype("uint8")


def update_plot(frame):
    line_left.set_ydata(latest_frame[:, 0])
    line_right.set_ydata(latest_frame[:, 1])
    return line_left, line_right


with sd.InputStream(
    device=device_index,
    channels=2,
    samplerate=RATE,
    blocksize=CHUNK,
    callback=audio_callback,
):
    animation = FuncAnimation(
        fig,
        update_plot,
        interval=1000 * CHUNK // RATE,
        blit=True,
    )
    print("Streaming audio into graph. Close the window to stop.")
    plt.show()
