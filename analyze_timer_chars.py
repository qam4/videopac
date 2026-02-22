#!/usr/bin/env python3
"""
Analyze timer display using single characters (0x10-0x3F), not quads.
Timer shows "02:00" which is 5 characters, so must use single chars.
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
    print("Analyzing timer display using single characters...\n")
    
    vdc_writes = parse_vdc_trace('vdc_trace.log')
    total_writes = len(vdc_writes)
    print(f"Total VDC writes: {total_writes}\n")
    
    # Single character registers: 0x10-0x3F (12 characters, 4 bytes each)
    # Character 0: 0x10-0x13
    # Character 1: 0x14-0x17
    # Character 2: 0x18-0x1B
    # Character 3: 0x1C-0x1F
    # Character 4: 0x20-0x23
    # Character 5: 0x24-0x27
    # Character 6: 0x28-0x2B
    # Character 7: 0x2C-0x2F
    # Character 8: 0x30-0x33
    # Character 9: 0x34-0x37
    # Character 10: 0x38-0x3B
    # Character 11: 0x3C-0x3F
    
    print("=== Single Character Register Writes (0x10-0x3F) ===\n")
    
    char_writes = []
    for idx, (line_num, reg, val) in enumerate(vdc_writes):
        if 0x10 <= reg <= 0x3F:
            estimated_frame = (idx / total_writes) * 120
            char_num = (reg - 0x10) // 4
            byte_num = (reg - 0x10) % 4
            byte_names = ["Y-pos", "X-pos", "Shape", "Color"]
            char_writes.append((idx, estimated_frame, reg, val, char_num, byte_num))
    
    print(f"Found {len(char_writes)} writes to single character registers\n")
    
    # Group by frame ranges
    print("=== Writes by Frame Range ===\n")
    
    frame_ranges = [
        (0, 10, "Initialization"),
        (10, 20, "Title Screen"),
        (20, 30, "Game Start"),
        (50, 60, "Frame 56 Area"),
    ]
    
    for start, end, label in frame_ranges:
        range_writes = [w for w in char_writes if start <= w[1] < end]
        if range_writes:
            print(f"\n{label} (Frames {start}-{end}): {len(range_writes)} writes")
            for idx, frame, reg, val, char_num, byte_num in range_writes[:20]:
                byte_names = ["Y-pos", "X-pos", "Shape", "Color"]
                print(f"  Write #{idx:4d} (~frame {frame:5.1f}): Char {char_num:2d} {byte_names[byte_num]:6s} = 0x{val:02X} ({val:3d})")
            if len(range_writes) > 20:
                print(f"  ... and {len(range_writes) - 20} more")
    
    # Look for patterns that might be "02:00"
    print("\n\n=== Looking for Timer Pattern ===\n")
    print("Timer '02:00' would need 5 consecutive characters")
    print("Looking for character shape writes that might be digits 0, 2, :, 0, 0\n")
    
    # Group writes by character number
    by_char = {}
    for idx, frame, reg, val, char_num, byte_num in char_writes:
        if char_num not in by_char:
            by_char[char_num] = []
        by_char[char_num].append((idx, frame, byte_num, val))
    
    for char_num in sorted(by_char.keys()):
        writes = by_char[char_num]
        print(f"\nCharacter {char_num} (0x{0x10 + char_num*4:02X}-0x{0x10 + char_num*4 + 3:02X}): {len(writes)} writes")
        for idx, frame, byte_num, val in writes[:10]:
            byte_names = ["Y-pos", "X-pos", "Shape", "Color"]
            print(f"  Frame {frame:5.1f}: {byte_names[byte_num]:6s} = 0x{val:02X}")

if __name__ == '__main__':
    main()
