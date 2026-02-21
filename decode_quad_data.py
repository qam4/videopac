#!/usr/bin/env python3
"""
Decode quad character data to understand timer display.

According to BIOS docs, quad characters are 4x4 pixel patterns.
Each quad has 8 bytes of character data in registers 0x42-0x49 (quad 0).

The timer uses quads to display digits. Each digit is made of character patterns.
"""

import re

def decode_batch(batch_writes):
    """Decode a batch of quad writes."""
    # Collect all register values
    regs = {}
    for idx, reg, val in batch_writes:
        regs[reg] = val
    
    print("Register values:")
    for reg in sorted(regs.keys()):
        print(f"  0x{reg:02X} = 0x{regs[reg]:02X} ({regs[reg]:08b})")
    
    # These look like they might be character pattern data
    # The high values (0xD8, 0xE8, 0xF8) suggest they're bit patterns
    
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
    print("Decoding quad character data...\n")
    
    vdc_writes = parse_vdc_trace('vdc_trace.log')
    
    # Find quad register writes
    quad_regs = {0x42, 0x43, 0x4A, 0x4B, 0x52, 0x53, 0x56, 0x57}
    
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
            current_batch.append((idx, reg, val))
            last_idx = idx
    
    if current_batch:
        batches.append(current_batch)
    
    for batch_num, batch in enumerate(batches, 1):
        print(f"=== Batch {batch_num} (frame ~{batch[0][0] / len(vdc_writes) * 70:.1f}) ===\n")
        decode_batch(batch)
        print()

if __name__ == '__main__':
    main()
