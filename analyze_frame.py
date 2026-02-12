#!/usr/bin/env python3
"""Analyze a PPM frame to see what's being rendered"""
import sys

def analyze_ppm(filename):
    with open(filename, 'rb') as f:
        # Read PPM header
        magic = f.readline().decode().strip()
        if magic != 'P6':
            print(f"Not a P6 PPM file: {magic}")
            return
        
        # Skip comments
        line = f.readline().decode().strip()
        while line.startswith('#'):
            line = f.readline().decode().strip()
        
        # Read dimensions
        width, height = map(int, line.split())
        maxval = int(f.readline().decode().strip())
        
        print(f"Frame: {filename}")
        print(f"Dimensions: {width}x{height}")
        print(f"Max value: {maxval}")
        
        # Read pixel data
        pixels = []
        for y in range(height):
            row = []
            for x in range(width):
                r = ord(f.read(1))
                g = ord(f.read(1))
                b = ord(f.read(1))
                row.append((r, g, b))
            pixels.append(row)
        
        # Count non-black pixels
        non_black = 0
        colors_used = set()
        for row in pixels:
            for r, g, b in row:
                if r != 0 or g != 0 or b != 0:
                    non_black += 1
                    colors_used.add((r, g, b))
        
        print(f"Non-black pixels: {non_black} ({100*non_black/(width*height):.1f}%)")
        print(f"Colors used: {len(colors_used)}")
        for color in sorted(colors_used):
            print(f"  RGB{color}")
        
        # Show a simple ASCII representation of the top portion
        print("\nTop 40 lines (. = black, # = non-black):")
        for y in range(min(40, height)):
            line = ""
            for x in range(0, width, 2):  # Sample every 2 pixels
                r, g, b = pixels[y][x]
                if r == 0 and g == 0 and b == 0:
                    line += "."
                else:
                    line += "#"
            print(line)

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: python3 analyze_frame.py <frame.ppm>")
        sys.exit(1)
    
    analyze_ppm(sys.argv[1])
