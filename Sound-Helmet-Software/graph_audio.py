import numpy as np
import sounddevice as sd
import matplotlib.pyplot as plt


DEVICE_NAME = "BlackHole"
SAMPLE_RATE = 44100
DURATION_SECONDS = 2
CHANNELS = 2


def find_audio_device(name):
    for index, device in enumerate(sd.query_devices()):
        if name.lower() in device["name"].lower() and device["max_input_channels"] > 0:
            print(f"Using input device {index}: {device['name']}")
            return index
    raise RuntimeError(f"{name} input device not found")


device_id = find_audio_device(DEVICE_NAME)

print("Recording from BlackHole...")
recording = sd.rec(
    int(SAMPLE_RATE * DURATION_SECONDS),
    samplerate=SAMPLE_RATE,
    channels=CHANNELS,
    dtype="float32",
    device=device_id,
)
sd.wait()

time_axis = np.linspace(0, DURATION_SECONDS, int(SAMPLE_RATE * DURATION_SECONDS))

plt.figure(figsize=(10, 4))
plt.plot(time_axis, recording[:, 0], label="Left channel")
plt.plot(time_axis, recording[:, 1], label="Right channel", alpha=0.7)
plt.title("BlackHole Audio Waveform")
plt.xlabel("Time (s)")
plt.ylabel("Amplitude")
plt.legend()
plt.grid(True)
plt.tight_layout()
plt.show()
