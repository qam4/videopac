#!/usr/bin/env python3
"""Compare o2em trace with our emulator trace to find divergence"""

import sys

def load_trace(filename):
    """Load trace file, skipping comments"""
    trace = []
    with open(filename, 'r') as f:
        for line in f:
            line = line.strip()
            if not line or line.startswith('#'):
                continue
            
            # Check if this is o2em format (simple: PC OP P1 ACC BANK or PC P1 ACC BANK)
            parts = line.split()
            if len(parts) >= 5 and not line.startswith('['):
                # New format with opcode: PC OP P1 ACC BANK
                try:
                    pc = int(parts[0], 16)
                    op = int(parts[1], 16)
                    p1 = int(parts[2], 16)
                    acc = int(parts[3], 16)
                    bank = int(parts[4])
                    trace.append((pc, op, p1, acc, bank))
                except ValueError:
                    continue
            elif len(parts) >= 4 and not line.startswith('['):
                # Old format without opcode: PC P1 ACC BANK
                try:
                    pc = int(parts[0], 16)
                    p1 = int(parts[1], 16)
                    acc = int(parts[2], 16)
                    bank = int(parts[3])
                    trace.append((pc, 0, p1, acc, bank))  # op=0 as placeholder
                except ValueError:
                    continue
            # Check if this is our format: [F:0 C:0] 0x000: 84 00 | A=00 PSW=00 P1=ff ...
            elif line.startswith('['):
                try:
                    # Extract PC (after "] " and before ":")
                    pc_start = line.index('] ') + 2
                    pc_end = line.index(':', pc_start)
                    pc_str = line[pc_start:pc_end].strip()
                    if pc_str.startswith('0x'):
                        pc = int(pc_str, 16)
                    else:
                        pc = int(pc_str, 16)
                    
                    # Extract opcode (first byte after ":")
                    op_start = pc_end + 2
                    op_end = line.index(' ', op_start)
                    op = int(line[op_start:op_end], 16)
                    
                    # Extract P1 (after "P1=" and before next space)
                    p1_start = line.index('P1=') + 3
                    p1_end = line.index(' ', p1_start)
                    p1 = int(line[p1_start:p1_end], 16)
                    
                    # Extract ACC (after "A=" and before next space)
                    a_start = line.index('A=') + 2
                    a_end = line.index(' ', a_start)
                    acc = int(line[a_start:a_end], 16)
                    
                    # Determine bank from P1 (P10 and P11 bits, inverted)
                    bank = (~p1) & 0x03
                    
                    trace.append((pc, op, p1, acc, bank))
                except (ValueError, IndexError):
                    continue
    return trace

def main():
    if len(sys.argv) != 3:
        print("Usage: python compare_traces.py <o2em_trace.log> <our_trace.log>")
        sys.exit(1)
    
    o2em_file = sys.argv[1]
    our_file = sys.argv[2]
    
    print(f"Loading o2em trace from: {o2em_file}")
    o2em_trace = load_trace(o2em_file)
    print(f"  Loaded {len(o2em_trace)} instructions")
    
    print(f"Loading our trace from: {our_file}")
    our_trace = load_trace(our_file)
    print(f"  Loaded {len(our_trace)} instructions")
    
    print("\nFirst 10 instructions from each:")
    print("\no2em:")
    for i in range(min(10, len(o2em_trace))):
        pc, op, p1, acc, bank = o2em_trace[i]
        print(f"  {i:5d}: PC={pc:03X} OP={op:02X} P1={p1:02X} ACC={acc:02X} BANK={bank}")
    
    print("\nOurs:")
    for i in range(min(10, len(our_trace))):
        pc, op, p1, acc, bank = our_trace[i]
        print(f"  {i:5d}: PC={pc:03X} OP={op:02X} P1={p1:02X} ACC={acc:02X} BANK={bank}")
    
    # Find first divergence
    print("\n" + "="*70)
    print("COMPARING TRACES...")
    print("="*70)
    
    min_len = min(len(o2em_trace), len(our_trace))
    divergence_found = False
    
    for i in range(min_len):
        o2em_pc, o2em_op, o2em_p1, o2em_acc, o2em_bank = o2em_trace[i]
        our_pc, our_op, our_p1, our_acc, our_bank = our_trace[i]
        
        if (o2em_pc != our_pc or o2em_op != our_op or o2em_p1 != our_p1 or 
            o2em_acc != our_acc or o2em_bank != our_bank):
            
            print(f"\n*** DIVERGENCE FOUND AT INSTRUCTION {i}!")
            print("\nContext (5 instructions before):")
            for j in range(max(0, i-5), i):
                o_pc, o_op, o_p1, o_acc, o_bank = o2em_trace[j]
                u_pc, u_op, u_p1, u_acc, u_bank = our_trace[j]
                match = "OK" if (o_pc == u_pc and o_op == u_op and o_p1 == u_p1 and 
                               o_acc == u_acc and o_bank == u_bank) else "XX"
                print(f"  {j:5d} {match}: o2em: PC={o_pc:03X} OP={o_op:02X} P1={o_p1:02X} ACC={o_acc:02X} BANK={o_bank}")
                print(f"         {match}:  ours: PC={u_pc:03X} OP={u_op:02X} P1={u_p1:02X} ACC={u_acc:02X} BANK={u_bank}")
            
            print("\n>>> DIVERGENCE POINT:")
            print(f"  {i:5d} XX: o2em: PC={o2em_pc:03X} OP={o2em_op:02X} P1={o2em_p1:02X} ACC={o2em_acc:02X} BANK={o2em_bank}")
            print(f"         XX:  ours: PC={our_pc:03X} OP={our_op:02X} P1={our_p1:02X} ACC={our_acc:02X} BANK={our_bank}")
            
            print("\nDifferences:")
            if o2em_pc != our_pc:
                print(f"  PC:   o2em={o2em_pc:03X} ({o2em_pc:4d})  ours={our_pc:03X} ({our_pc:4d})  diff={our_pc-o2em_pc:+d}")
            if o2em_op != our_op:
                print(f"  OP:   o2em={o2em_op:02X}  ours={our_op:02X}  <-- DIFFERENT INSTRUCTION BYTES!")
            if o2em_p1 != our_p1:
                print(f"  P1:   o2em={o2em_p1:02X} (0b{o2em_p1:08b})  ours={our_p1:02X} (0b{our_p1:08b})")
            if o2em_acc != our_acc:
                print(f"  ACC:  o2em={o2em_acc:02X} ({o2em_acc:3d})  ours={our_acc:02X} ({our_acc:3d})  diff={our_acc-o2em_acc:+d}")
            if o2em_bank != our_bank:
                print(f"  BANK: o2em={o2em_bank}  ours={our_bank}")
            
            print("\nNext 5 instructions:")
            for j in range(i+1, min(i+6, min_len)):
                o_pc, o_op, o_p1, o_acc, o_bank = o2em_trace[j]
                u_pc, u_op, u_p1, u_acc, u_bank = our_trace[j]
                print(f"  {j:5d}  : o2em: PC={o_pc:03X} OP={o_op:02X} P1={o_p1:02X} ACC={o_acc:02X} BANK={o_bank}")
                print(f"         :  ours: PC={u_pc:03X} OP={u_op:02X} P1={u_p1:02X} ACC={u_acc:02X} BANK={u_bank}")
            
            divergence_found = True
            break
    
    if not divergence_found:
        if len(o2em_trace) != len(our_trace):
            print(f"\n!!! Traces match for {min_len} instructions but have different lengths:")
            print(f"  o2em: {len(o2em_trace)} instructions")
            print(f"  ours: {len(our_trace)} instructions")
        else:
            print(f"\n+++ Traces match perfectly! ({len(o2em_trace)} instructions)")

if __name__ == '__main__':
    main()
