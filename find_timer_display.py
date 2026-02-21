#!/usr/bin/env python3
"""
Find timer display "02:02" in VDC trace.

The timer is displayed using quads. According to BIOS docs:
- display_2_digit_bcd_characters prepares 8 bytes in RAM (0x7D-0x7A)
- These bytes are copied to VDC quad registers during VBLANK
- Target VDC address is in RAM[0x7E] = 0x42 (quad character registers)
- Byte count is in RAM[0x7F] = 0x08

So we're looking for 8 consecutive writes to registers starting at 0x42.
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

def find_consecutive_writes(writes, start_reg, count):
    """Find sequences of consecutive register writes."""
    sequences = []
    
    for i in range(len(writes) - count + 1):
        # Check if we have 'count' consecutive writes starting at start_reg
        seq = []
        expected_reg = start_reg
        
        for j in range(count):
            if i + j >= len(writes):
                break
            line_num, reg, val = writes[i + j]
            if reg == expected_reg:
                seq.append((line_num, reg, val))
                expected_reg += 1
            else:
                break
        
        if len(seq) == count:
            sequences.append((i, seq))
    
    return sequences

def main():
    print("Finding timer display in VDC trace...\n")
    
    vdc_writes = parse_vdc_trace('vdc_trace.log')
    print(f"Total VDC writes: {len(vdc_writes)}\n")
    
    # Look for 8 consecutive writes starting at 0x42
    # This is the pattern used by the BIOS copy routine
    print("=== Looking for 8 consecutive writes to 0x42-0x49 ===\n")
    
    sequences = find_consecutive_writes(vdc_writes, 0x42, 8)
    
    print(f"Found {len(sequences)} sequences\n")
    
    for seq_num, (idx, seq) in enumerate(sequences, 1):
        print(f"--- Sequence {seq_num} (write index {idx}, ~frame {idx / len(vdc_writes) * 70:.1f}) ---")
        for line_num, reg, val in seq:
            print(f"  0x{reg:02X} = 0x{val:02X} ({val:3d}) {val:08b}")
        print()
    
    # Also look for any writes to 0x42-0x49 (quad character registers)
    print("\n=== All writes to quad character registers (0x42-0x49) ===\n")
    
    quad_char_regs = set(range(0x42, 0x4A))
    quad_writes = [(i, line_num, reg, val) for i, (line_num, reg, val) in enumerate(vdc_writes) 
                   if reg in quad_char_regs]
    
    print(f"Found {len(quad_writes)} writes to quad character registers\n")
    
    # Group by proximity
    if quad_writes:
        print("Grouped by proximity:")
        groups = []
        current_group = [quad_writes[0]]
        
        for i in range(1, len(quad_writes)):
            prev_idx = quad_writes[i-1][0]
            curr_idx = quad_writes[i][0]
            
            if curr_idx - prev_idx < 20:  # Within 20 writes
                current_group.append(quad_writes[i])
            else:
                groups.append(current_group)
                current_group = [quad_writes[i]]
        
        groups.append(current_group)
        
        for group_num, group in enumerate(groups, 1):
            first_idx = group[0][0]
            print(f"\nGroup {group_num} (write index {first_idx}, ~frame {first_idx / len(vdc_writes) * 70:.1f}):")
            for idx, line_num, reg, val in group:
                print(f"  0x{reg:02X} = 0x{val:02X} ({val:3d}) {val:08b}")

if __name__ == '__main__':
    main()
