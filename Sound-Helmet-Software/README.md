Sound Helmet Software
=====================

Software prototypes for an ESP32-based spatial audio helmet. The project includes:

- ESP32 sketches for TAS5805M amplifier setup, IMU-based rotation tracking, UDP audio buffering, and TCP test servers.
- PC-side clients for sending structured TCP test data, controlling ESP32 outputs, and streaming local audio files.
- Python and Node.js utilities for capturing, plotting, and forwarding BlackHole audio streams.

Repository Notes
----------------

- Wi-Fi credentials are represented with placeholder values in source files.
- Generated binaries, local media files, editor settings, and dependency folders are ignored by `.gitignore`.
- Arduino sketch folders retain matching `.ino` filenames so they open correctly in the Arduino IDE.
