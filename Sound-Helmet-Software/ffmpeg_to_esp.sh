#!/bin/bash

ffmpeg -f avfoundation -i ":BlackHole 64ch" \
  -filter_complex "[0:a]pan=stereo|c0=c0|c1=c1[aout]" \
  -map "[aout]" -ac 2 -ar 44100 -sample_fmt u8 \
  -flush_packets 0 -fflags +nobuffer \
  -f u8 "udp://172.21.124.30:12345?pkt_size=128"
