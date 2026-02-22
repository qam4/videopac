#!/usr/bin/env python3
"""
Analyze grid system writes (0xC0-0xFF) to find timer display.
Grid is the background character system - likely where "02:00" is displayed.
"""

import re

def parse_vdc_trace(filename):
    """Parse VDC trace log."""
    writes = []
    pattern = r'\[VDC\] write_register\(0x([0-9a-f]+), 0x([0-9a-f]+)\)'
    
    with open(filename, 'r') as f:
        for line_num, line in enumerate(f):
            match = re.search(pattern, line)
            if match:
                reg = int(match.group(1), 16)
                val = int(match.group(2), 16)
                writes.append((line_num, reg, val))
    
    return writes

def main():
    print("Analyzing grid system writes (0xC0-0xFF)...\n")
    
    vdc_writes = parse_vdc_trace('vdc_trace.log')
    total_writes = len(vdc_writes)
    print(f"Total VDC writes: {total_writes}\n")
    
    # Grid registers:
    # 0xC0-0xDF: Horizontal grid lines (32 bytes, 12 rows x 10 columns)
    # 0xE0-0xE9: Vertical grid lines (10 bytes)
    # But also character RAM might be here
    
    print("=== Grid Register Writes (0xC0-0xFF) ===\n")
    
    grid_writes = []
    for idx, (line_num, reg, val) in enumerate(vdc_writes):
        if 0xC0 <= reg <= 0xFF:
            estimated_frame = (idx / total_writes) * 120
            grid_writes.append((idx, estimated_frame, reg, val))
    
    print(f"Found {len(grid_writes)} writes to grid registers\n")
    
    if len(grid_writes) == 0:
        print("NO writes to grid registers found!")
        print("Timer must be using a different display method.\n")
        return
    
    # Group by frame ranges
    print("=== Writes by Frame Range ===\n")
    
    frame_ranges = [
        (0, 10, "Initialization"),
        (10, 20, "Title Screen"),
        (20, 30, "Game Start"),
        (50, 60, "Frame 56 Area"),
    ]
    
    for start, end, label in frame_ranges:
        range_writes = [w for w in grid_writes if start <= w[1] < end]
        if range_writes:
            print(f"\n{label} (Frames {start}-{end}): {len(range_writes)} writes")
            for idx, frame, reg, val in range_writes[:30]:
                print(f"  Write #{idx:4d} (~frame {frame:5.1f}): reg=0x{reg:02X} val=0x{val:02X} ({val:3d}) {val:08b}")
            if len(range_writes) > 30:
                print(f"  ... and {len(range_writes) - 30} more")
    
    # Show all grid writes
    print("\n\n=== All Grid Writes ===\n")
    for idx, frame, reg, val in grid_writes[:100]:
        print(f"Write #{idx:4d} (~frame {frame:5.1f}): reg=0x{reg:02X} val=0x{val:02X} ({val:3d}) {val:08b}")
    if len(grid_writes) > 100:
        print(f"... and {len(grid_writes) - 100} more")

if __name__ == '__main__':
    main()
