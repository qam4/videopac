#!/usr/bin/env python3
"""
Correlate instruction trace with VDC trace to find frame numbers.
"""

import re

def parse_trace_log(filename, max_lines=None):
    """Parse instruction trace to find frame boundaries."""
    frames = []
    current_frame = []
    frame_num = 0
    
    pattern = r'Frame:\s*(\d+)'
    
    with open(filename, 'r') as f:
        for i, line in enumerate(f):
            if max_lines and i >= max_lines:
                break
            
            match = re.search(pattern, line)
            if match:
                if current_frame:
                    frames.append((frame_num, current_frame))
                frame_num = int(match.group(1))
                current_frame = [i]
            else:
                if current_frame:
                    current_frame.append(i)
    
    if current_frame:
        frames.append((frame_num, current_frame))
    
    return frames

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
    print("Correlating instruction trace with VDC trace...\n")
    
    # Parse VDC trace
    vdc_writes = parse_vdc_trace('vdc_trace.log')
    print(f"Total VDC writes: {len(vdc_writes)}\n")
    
    # Find quad register writes
    quad_regs = {0x42, 0x43, 0x4A, 0x4B, 0x52, 0x53, 0x56, 0x57}
    
    print("=== QUAD REGISTER WRITES BY POSITION ===\n")
    
    # Group by batches
    batches = []
    current_batch = []
    last_idx = -10
    
    for idx, (line_num, reg, val) in enumerate(vdc_writes):
        if reg in quad_regs:
            if idx - last_idx > 10:
                if current_batch:
                    batches.append(current_batch)
                current_batch = []
            current_batch.append((idx, line_num, reg, val))
            last_idx = idx
    
    if current_batch:
        batches.append(current_batch)
    
    print(f"Found {len(batches)} batches of quad writes\n")
    
    for batch_num, batch in enumerate(batches, 1):
        print(f"--- Batch {batch_num} ---")
        print(f"VDC write indices: {batch[0][0]} to {batch[-1][0]}")
        print(f"Approximate position: {batch[0][0] / len(vdc_writes) * 70:.1f} frames into execution")
        print("Writes:")
        for idx, line_num, reg, val in batch:
            print(f"  #{idx}: reg=0x{reg:02X} val=0x{val:02X}")
        print()

if __name__ == '__main__':
    main()
