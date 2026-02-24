#!/usr/bin/env python3
import re
import sys

def parse_o2em_line(line):
    parts = line.strip().split()
    if len(parts) < 5 or parts[0] == '#':
        return None
    return {'pc': parts[0], 'op': parts[1], 'p1': parts[2], 'acc': parts[3], 'bank': parts[4]}

def parse_our_line(line):
    match = re.search(r'0x([0-9a-f]+):\s+([0-9a-f]+)\s+[0-9a-f]+\s+\|\s+A=([0-9a-f]+)\s+PSW=[0-9a-f]+\s+P1=([0-9a-f]+)', line)
    if not match:
        return None
    pc, op, acc, p1 = match.groups()
    return {'pc': pc, 'op': op, 'p1': p1, 'acc': acc}

with open('o2em_trace.log', 'r') as f1, open('trace_cpu.log', 'r') as f2:
    o2em_lines = f1.readlines()
    our_lines = f2.readlines()
    
    # Skip header line in o2em trace
    o2em_start = 1
    our_start = 0
    
    max_instructions = min(len(o2em_lines) - o2em_start, len(our_lines) - our_start)
    print(f'Analyzing {max_instructions} instructions...')
    print()
    
    divergences = []
    convergences = []
    last_state = None  # 'match' or 'diverge'
    
    for i in range(max_instructions):
        o2em = parse_o2em_line(o2em_lines[i + o2em_start])
        ours = parse_our_line(our_lines[i + our_start])
        
        if o2em and ours:
            matches = (o2em['pc'] == ours['pc'] and o2em['op'] == ours['op'])
            
            if not matches:
                if last_state != 'diverge':
                    divergences.append(i)
                last_state = 'diverge'
            else:
                if last_state == 'diverge':
                    convergences.append(i)
                last_state = 'match'
    
    print(f'Found {len(divergences)} divergence points')
    print(f'Found {len(convergences)} convergence points')
    print()
    
    if len(divergences) > 0:
        print('Divergence and convergence pattern:')
        for j in range(min(20, len(divergences))):
            div = divergences[j]
            o2em = parse_o2em_line(o2em_lines[div + o2em_start])
            ours = parse_our_line(our_lines[div + our_start])
            
            conv_str = ''
            if j < len(convergences):
                conv = convergences[j]
                duration = conv - div
                conv_str = f' -> converges at {conv} (after {duration} instructions)'
            
            print(f'{div:6d}: o2em PC={o2em["pc"]} OP={o2em["op"]} | ours PC={ours["pc"]} OP={ours["op"]}{conv_str}')
