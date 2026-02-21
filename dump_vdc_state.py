#!/usr/bin/env python3
"""
Dump VDC quad register state from screenshots by analyzing the timer display.
"""

from PIL import Image
import sys

def analyze_frame(frame_num):
    """Analyze a frame's timer display."""
    filename = f"screenshots/frame_{frame_num:06d}.png"
    try:
        img = Image.open(filename)
        # Timer is at top-right, approximately x=200-240, y=10-20
        # Extract that region
        timer_region = img.crop((200, 10, 240, 25))
        timer_region.save(f"timer_frame_{frame_num}.png")
        print(f"Frame {frame_num}: Saved timer region to timer_frame_{frame_num}.png")
    except FileNotFoundError:
        print(f"Frame {frame_num}: File not found")
    except Exception as e:
        print(f"Frame {frame_num}: Error - {e}")

if __name__ == "__main__":
    # Analyze frames around the corruption point
    for frame in [54, 55, 56, 57, 58]:
        analyze_frame(frame)
