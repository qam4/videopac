#!/usr/bin/env python3
import sys
import re

# Load labels from annotated files
def load_labels():
    labels = {}
    
    # Load from BIOS
    try:
        with open('doc/french_bios_annotated.txt', 'r') as f:
            for line in f:
                match = re.match(r'^([0-9A-Fa-f]{4}):\s*(\w+)', line)
                if match:
                    addr = '0x' + match.group(1).lower()
                    label = match.group(2)
                    labels[addr] = label
    except:
        pass
    
    # Load from game disassembly
    try:
        with open('doc/satellite-attack-disassembly.txt', 'r') as f:
            for line in f:
                match = re.match(r'^([0-9A-Fa-f]{4}):\s*\[\s*(\w+)', line)
                if match:
                    addr = '0x' + match.group(1).lower()
                    label = match.group(2)
                    labels[addr] = label
    except:
        pass
    
    return labels

def analyze_frame(filename, frame_num, labels):
    """Analyze a specific frame and build call tree"""
    
    call_tree = []
    indent_level = 0
    
    with open(filename, 'r') as f:
        in_frame = False
        for line in f:
            # Check if we're in the target frame
            match = re.match(r'\[F:(\d+)\s+C:(\d+)\]\s+(0x[0-9a-f]+):\s+([0-9a-f]+\s+[0-9a-f]+)\s+(\w+)', line)
            if not match:
                continue
                
            current_frame = int(match.group(1))
            cycle = match.group(2)
            addr = match.group(3)
            instr = match.group(5)
            
            if current_frame == frame_num:
                in_frame = True
            elif in_frame:
                break
            else:
                continue
            
            # Get label for address
            addr_label = labels.get(addr, addr)
            
            # Track CALL instructions
            if instr == 'CALL':
                # Extract target from line
                target_match = re.search(r'CALL (0x[0-9a-f]+)', line)
                if target_match:
                    target = target_match.group(1)
                    target_label = labels.get(target, target)
                    call_tree.append(f"{'  ' * indent_level}[C:{cycle}] {addr_label}: CALL {target_label}")
                    indent_level += 1
            
            # Track RET instructions
            elif instr in ['RET', 'RETR']:
                indent_level = max(0, indent_level - 1)
                call_tree.append(f"{'  ' * indent_level}[C:{cycle}] {addr_label}: {instr}")
            
            # Track significant JMP instructions
            elif instr == 'JMP':
                target_match = re.search(r'JMP (0x[0-9a-f]+)', line)
                if target_match:
                    target = target_match.group(1)
                    try:
                        if abs(int(target, 16) - int(addr, 16)) > 0x30:
                            target_label = labels.get(target, target)
                            call_tree.append(f"{'  ' * indent_level}[C:{cycle}] {addr_label}: JMP {target_label}")
                    except:
                        pass
    
    return call_tree

if __name__ == '__main__':
    if len(sys.argv) < 2:
        print("Usage: analyze_frame.py <frame_number> [<frame_number2> ...]")
        sys.exit(1)
    
    labels = load_labels()
    print(f"Loaded {len(labels)} labels\n")
    
    for frame_arg in sys.argv[1:]:
        frame_num = int(frame_arg)
        print(f"=== Frame {frame_num} Call Tree ===\n")
        
        tree = analyze_frame('trace.log', frame_num, labels)
        for line in tree[:200]:
            print(line)
        
        if len(tree) > 200:
            print(f"\n... ({len(tree) - 200} more calls)")
        
        print(f"\nTotal calls/returns/jumps: {len(tree)}\n")
