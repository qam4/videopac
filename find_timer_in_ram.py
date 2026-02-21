#!/usr/bin/env python3
"""
Find timer value "02:00" in RAM by searching the instruction trace.

According to BIOS docs:
- Timer is stored in BCD format
- display_2_digit_bcd_characters is called to display it
- The routine prepares 8 bytes in RAM[0x7D-0x7A] (backwards)
- RAM[0x7E] = 0x42 (VDC target address)
- RAM[0x7F] = 0x08 (byte count)

We need to find where RAM[0x7E] is set to 0x42 and RAM[0x7F] is set to 0x08.
"""

import re

def search_trace_for_pattern(filename, pattern, max_matches=50):
    """Search trace log for a pattern."""
    matches = []
    
    with open(filename, 'r') as f:
        for line_num, line in enumerate(f):
            if re.search(pattern, line, re.IGNORECASE):
                matches.append((line_num, line.strip()))
                if len(matches) >= max_matches:
                    break
    
    return matches

def main():
    print("Searching for timer-related RAM operations in trace...\n")
    
    # Look for writes to RAM[0x7E] = 0x42 (VDC target address for quads)
    print("=== Looking for RAM[0x7E] = 0x42 (copy target address) ===\n")
    matches = search_trace_for_pattern('trace.log', r'RAM\[0x7E\].*0x42', max_matches=20)
    
    if matches:
        for line_num, line in matches[:10]:
            print(f"Line {line_num}: {line}")
    else:
        print("No matches found")
    
    # Look for writes to RAM[0x7F] = 0x08 (byte count)
    print("\n=== Looking for RAM[0x7F] = 0x08 (copy byte count) ===\n")
    matches = search_trace_for_pattern('trace.log', r'RAM\[0x7F\].*0x08', max_matches=20)
    
    if matches:
        for line_num, line in matches[:10]:
            print(f"Line {line_num}: {line}")
    else:
        print("No matches found")
    
    # Look for calls to display_2_digit_bcd_characters (0x17C)
    print("\n=== Looking for calls to display_2_digit_bcd_characters (0x17C) ===\n")
    matches = search_trace_for_pattern('trace.log', r'CALL.*0x17C', max_matches=20)
    
    if matches:
        for line_num, line in matches[:10]:
            print(f"Line {line_num}: {line}")
    else:
        print("No matches found")
    
    # Look for calls to up_down_counter (0x1B0)
    print("\n=== Looking for calls to up_down_counter (0x1B0) ===\n")
    matches = search_trace_for_pattern('trace.log', r'CALL.*0x1B0', max_matches=20)
    
    if matches:
        for line_num, line in matches[:10]:
            print(f"Line {line_num}: {line}")
    else:
        print("No matches found")
    
    # Look for RAM[0x3F] bit 7 being set (copy mode trigger)
    print("\n=== Looking for RAM[0x3F] with bit 7 set (copy mode) ===\n")
    matches = search_trace_for_pattern('trace.log', r'RAM\[0x3F\].*0x[89A-F][0-9A-F]', max_matches=30)
    
    if matches:
        print(f"Found {len(matches)} matches, showing first 10:")
        for line_num, line in matches[:10]:
            print(f"Line {line_num}: {line}")
    else:
        print("No matches found")

if __name__ == '__main__':
    main()
