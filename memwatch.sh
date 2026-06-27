#!/bin/bash
# Run once. Drops you into a recorded shell. Work normally. Type 'exit' to stop.
RAW=~/.mem_c_raw
mkdir -p "$RAW"
TS="$RAW/session-$(date +%Y%m%d-%H%M%S).log"
echo "recording -> $TS   (work normally; 'exit' to stop)"
script -q -f "$TS"
echo "stopped: $TS  ->  run ./wrapup.py to distill."
