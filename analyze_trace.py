#!/usr/bin/env python3
"""
Analyze trace log to find VDC character RAM writes around frame 56
"""

import re
import sys

def parse_trace_line(line):
    """Parse a trace log line"""
    # [F:56 C:3331018] 0x183: 90 c8 | A=08 PSW=10 P1=af P2=f0 RB1 F1=1
    match = re.match(r'\[F:(\d+) C:(\d+)\] 0x([0-9a-f]+): ([0-9a-f]+) ([0-9a-f]+) \| A=([0-9a-f]+) PSW=([0-9a-f]+) P1=([0-9a-f]+) P2=([0-9a-f]+) (RB[01]) F1=([01])', line)
    if not match:
        return None
    
    return {
        'frame': int(match.group(1)),
        'cycle': int(match.group(2)),
        'pc': int(match.group(3), 16),
        'opcode': int(match.group(4), 16),
        'operand': int(match.group(5), 16),
        'a': int(match.group(6), 16),
        'psw': int(match.group(7), 16),
        'p1': int(match.group(8), 16),
        'p2': int(match.group(9), 16),
        'rb': match.group(10),
        'f1': int(match.group(11))
    }

def is_vdc_enabled(p1):
    """Check if VDC is enabled (P13=0, active low)"""
    return (p1 & 0x08) == 0

def is_movx_write(opcode):
    """Check if opcode is MOVX @Rr,A (0x90-0x91)"""
    return opcode in [0x90, 0x91]

def main():
    print("Analyzing trace log for VDC writes around frame 56...")
    print()
    
    # Track register values
    r0 = None
    r1 = None
    
    with open('trace.log', 'r') as f:
        for line_num, line in enumerate(f, 1):
            parsed = parse_trace_line(line.strip())
            if not parsed:
                continue
            
            frame = parsed['frame']
            
            # Only look at frames 55-58
            if frame < 55 or frame > 58:
                continue
            
            opcode = parsed['opcode']
            pc = parsed['pc']
            a = parsed['a']
            p1 = parsed['p1']
            
            # Track MOV R0,#data (0xb8) and MOV R1,#data (0xb9)
            if opcode == 0xb8:
                r0 = parsed['operand']
            elif opcode == 0xb9:
                r1 = parsed['operand']
            # Track INC @Rr (0x10-0x11)
            elif opcode == 0x10 and r0 is not None:
                r0 = (r0 + 1) & 0xFF
            elif opcode == 0x11 and r1 is not None:
                r1 = (r1 + 1) & 0xFF
            
            # Look for MOVX @R0,A or MOVX @R1,A
            if is_movx_write(opcode) and is_vdc_enabled(p1):
                reg_name = "R0" if opcode == 0x90 else "R1"
                reg_val = r0 if opcode == 0x90 else r1
                
                if reg_val is not None:
                    # Show all VDC writes, especially quads (0x40-0x7F)
                    area = "CHAR" if reg_val <= 0x3F else "QUAD" if 0x40 <= reg_val <= 0x7F else "COLOR" if 0x80 <= reg_val <= 0xBF else "CTRL"
                    if 0x30 <= a <= 0x39:  # Digits 0-9
                        char_display = f"'{chr(a)}'"
                    else:
                        char_display = f"0x{a:02x}"
                    
                    # Focus on quad writes (timer likely uses quads)
                    if 0x40 <= reg_val <= 0x7F:
                        print(f"[F:{frame:3d} Line:{line_num:6d}] PC=0x{pc:03x} MOVX @{reg_name},A  addr=0x{reg_val:02x} val={char_display:6s} [{area}]")

if __name__ == "__main__":
    main()
