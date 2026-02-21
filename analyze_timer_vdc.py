#!/usr/bin/env python3
"""
Analyze VDC trace to find timer quad register writes.

Timer uses quad registers 0x40-0x5F (4 quads x 8 registers each).
According to BIOS, timer data is copied to VDC starting at 0x42.
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
    print("Analyzing timer quad register writes...\n")
    
    writes = parse_vdc_trace('vdc_trace.log')
    print(f"Total VDC writes: {len(writes)}\n")
    
    # Quad registers: 0x40-0x5F (4 quads)
    # Each quad has 8 registers:
    #   +0: X position
    #   +1: Y position  
    #   +2-+7: Character data (6 bytes)
    
    # Timer starts at 0x42 according to BIOS (0x200: MOV A,##0x42)
    # So it writes 8 bytes starting at 0x42
    
    timer_regs = set(range(0x42, 0x4A))  # 0x42-0x49 (8 bytes)
    
    print("=== WRITES TO TIMER QUAD REGISTERS (0x42-0x49) ===\n")
    
    timer_writes = []
    for idx, (line_num, reg, val) in enumerate(writes):
        if reg in timer_regs:
            timer_writes.append((idx, line_num, reg, val))
    
    print(f"Found {len(timer_writes)} writes to timer quad registers\n")
    
    # Group into batches (consecutive writes within 20 indices)
    batches = []
    current_batch = []
    last_idx = -100
    
    for idx, line_num, reg, val in timer_writes:
        if idx - last_idx > 20:
            if current_batch:
                batches.append(current_batch)
            current_batch = []
        current_batch.append((idx, line_num, reg, val))
        last_idx = idx
    
    if current_batch:
        batches.append(current_batch)
    
    print(f"Grouped into {len(batches)} batches\n")
    
    for batch_num, batch in enumerate(batches, 1):
        print(f"--- Batch {batch_num} ---")
        print(f"VDC write indices: {batch[0][0]} to {batch[-1][0]}")
        print(f"Approximate frame: {batch[0][0] / len(writes) * 70:.1f}")
        print("Writes:")
        for idx, line_num, reg, val in batch:
            print(f"  #{idx}: reg=0x{reg:02X} val=0x{val:02X} ({val:3d} = {val:08b}b)")
        print()

if __name__ == '__main__':
    main()
