#!/usr/bin/env python3
"""
Analyze VDC trace to find timer corruption bug.
The timer uses quad registers (0x42, 0x43, 0x4A, 0x4B, 0x52, 0x53, 0x56, 0x57).
"""

import re
from collections import defaultdict

def parse_vdc_trace(filename):
    """Parse VDC trace log and return list of (register, value) tuples."""
    writes = []
    pattern = r'\[VDC\] write_register\(0x([0-9a-f]+), 0x([0-9a-f]+)\)'
    
    with open(filename, 'r') as f:
        for line in f:
            match = re.search(pattern, line)
            if match:
                reg = int(match.group(1), 16)
                val = int(match.group(2), 16)
                writes.append((reg, val))
    
    return writes

def analyze_quad_writes(writes):
    """Analyze writes to quad registers (timer display)."""
    # Quad registers for 4 quads (timer uses quads for display)
    # Each quad has 8 registers: shape, color, x, y, and 4 character bytes
    quad_regs = {
        0x42, 0x43, 0x4A, 0x4B,  # Quad character data
        0x52, 0x53, 0x56, 0x57   # More quad character data
    }
    
    print("=== QUAD REGISTER WRITES (Timer Display) ===\n")
    
    quad_writes = []
    for i, (reg, val) in enumerate(writes):
        if reg in quad_regs:
            quad_writes.append((i, reg, val))
    
    print(f"Total quad register writes: {len(quad_writes)}\n")
    
    # Group writes by batches (consecutive writes are likely one update)
    batches = []
    current_batch = []
    last_idx = -10
    
    for idx, reg, val in quad_writes:
        if idx - last_idx > 10:  # New batch if gap > 10 writes
            if current_batch:
                batches.append(current_batch)
            current_batch = []
        current_batch.append((idx, reg, val))
        last_idx = idx
    
    if current_batch:
        batches.append(current_batch)
    
    print(f"Found {len(batches)} batches of quad writes\n")
    
    # Print each batch
    for batch_num, batch in enumerate(batches, 1):
        print(f"--- Batch {batch_num} (writes {batch[0][0]}-{batch[-1][0]}) ---")
        for idx, reg, val in batch:
            print(f"  Write #{idx}: reg=0x{reg:02X}, val=0x{val:02X}")
        print()
    
    return batches

def find_suspicious_patterns(writes):
    """Find suspicious write patterns that might indicate the bug."""
    print("\n=== SUSPICIOUS PATTERNS ===\n")
    
    # Look for writes to quad registers with unusual values
    for i, (reg, val) in enumerate(writes):
        # Quad character registers should have reasonable character codes
        if reg in {0x42, 0x43, 0x4A, 0x4B, 0x52, 0x53, 0x56, 0x57}:
            # Character codes should be in reasonable ranges
            # 0x00-0x3F are standard characters
            if val > 0x3F:
                print(f"Write #{i}: UNUSUAL VALUE - reg=0x{reg:02X}, val=0x{val:02X}")

def main():
    print("Analyzing VDC trace for timer corruption bug...\n")
    
    writes = parse_vdc_trace('vdc_trace.log')
    print(f"Total VDC writes: {len(writes)}\n")
    
    # Analyze quad register writes (timer display)
    batches = analyze_quad_writes(writes)
    
    # Find suspicious patterns
    find_suspicious_patterns(writes)
    
    # Print summary
    print("\n=== SUMMARY ===")
    print(f"Total VDC writes: {len(writes)}")
    print(f"Quad write batches: {len(batches)}")
    print("\nThe timer corruption likely occurs in one of these quad write batches.")
    print("Look for batches with unusual character codes or incomplete updates.")

if __name__ == '__main__':
    main()
