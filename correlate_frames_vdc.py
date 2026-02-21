#!/usr/bin/env python3
"""
Correlate VDC writes with frame numbers from instruction trace.
"""

import re

def parse_trace_frames(filename):
    """Extract frame boundaries from instruction trace."""
    frame_lines = {}
    current_frame = None
    
    with open(filename, 'r') as f:
        for line_num, line in enumerate(f):
            match = re.match(r'\[F:(\d+)', line)
            if match:
                frame = int(match.group(1))
                if frame not in frame_lines:
                    frame_lines[frame] = line_num
                current_frame = frame
    
    return frame_lines

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
    print("Correlating VDC writes with frames...\n")
    
    # Parse VDC trace
    vdc_writes = parse_vdc_trace('vdc_trace.log')
    total_writes = len(vdc_writes)
    print(f"Total VDC writes: {total_writes}\n")
    
    # Estimate frame for each VDC write (assuming 70 frames total)
    # This is approximate since we don't have exact frame markers in VDC trace
    
    quad_char_regs = set(range(0x42, 0x4A))
    
    print("=== VDC writes to quad character registers (0x42-0x49) ===\n")
    
    for idx, (line_num, reg, val) in enumerate(vdc_writes):
        if reg in quad_char_regs:
            estimated_frame = (idx / total_writes) * 70
            print(f"Write #{idx:4d} (~frame {estimated_frame:5.1f}): reg=0x{reg:02X} val=0x{val:02X} ({val:3d}) {val:08b}")
    
    # Focus on writes around frame 56 (estimated index range)
    frame_56_start = int((56 / 70) * total_writes)
    frame_56_end = int((57 / 70) * total_writes)
    
    print(f"\n=== ALL VDC writes around frame 56 (indices {frame_56_start}-{frame_56_end}) ===\n")
    
    for idx in range(max(0, frame_56_start - 20), min(total_writes, frame_56_end + 20)):
        line_num, reg, val = vdc_writes[idx]
        estimated_frame = (idx / total_writes) * 70
        marker = " <-- QUAD CHAR" if reg in quad_char_regs else ""
        print(f"Write #{idx:4d} (~frame {estimated_frame:5.1f}): reg=0x{reg:02X} val=0x{val:02X}{marker}")

if __name__ == '__main__':
    main()
