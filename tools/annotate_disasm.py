#!/usr/bin/env python3
"""
Annotate disassembly with comments from o2romsrc.txt

Usage: python3 annotate_disasm.py <disasm_file> <o2romsrc_file> > output.txt
"""

import sys
import re

def parse_o2romsrc(filename):
    """Parse o2romsrc.txt and extract comments by address"""
    comments = {}
    block_comments = {}
    
    with open(filename, 'r') as f:
        lines = f.readlines()
    
    current_block = []
    
    for line in lines:
        # Match address lines like "0000 : 84 00     jmp 400"
        addr_match = re.match(r'^([0-9a-fA-F]{4})\s*:\s*([0-9a-fA-F]{2}(?:\s+[0-9a-fA-F]{2})?)\s+(.+?)(?:\s*;(.+))?$', line)
        
        if addr_match:
            addr = int(addr_match.group(1), 16)
            inline_comment = addr_match.group(4)
            
            # Store block comment if we have one
            if current_block:
                block_comments[addr] = '\n'.join(current_block)
                current_block = []
            
            # Store inline comment
            if inline_comment:
                comments[addr] = inline_comment.strip()
        
        # Match block comment lines (start with ;)
        elif line.strip().startswith(';'):
            current_block.append(line.rstrip())
        
        # Empty line does NOT reset block comment - keep accumulating
        # This allows multi-paragraph block comments to be captured
    
    return comments, block_comments

def annotate_disasm(disasm_file, o2romsrc_file):
    """Merge disassembly with comments from o2romsrc"""
    comments, block_comments = parse_o2romsrc(o2romsrc_file)
    
    with open(disasm_file, 'r') as f:
        lines = f.readlines()
    
    for line in lines:
        # Check if this is an address line
        addr_match = re.match(r'^0x([0-9a-fA-F]{3}):', line)
        
        if addr_match:
            addr = int(addr_match.group(1), 16)
            
            # Print block comment before the instruction
            if addr in block_comments:
                print(block_comments[addr])
            
            # Print the instruction line
            print(line.rstrip(), end='')
            
            # Add inline comment if available
            if addr in comments:
                # Pad to column 50 for alignment
                padding = max(1, 50 - len(line.rstrip()))
                print(' ' * padding + '; ' + comments[addr])
            else:
                print()
        else:
            # Check if this is a label line (starts with loc_ or is a known label)
            if line.strip().startswith('loc_') or (line.strip() and line.strip().endswith(':') and not line.strip().startswith('0x')):
                print()  # Add blank line before label
            # Print labels and other lines as-is
            print(line.rstrip())

if __name__ == '__main__':
    if len(sys.argv) != 3:
        print("Usage: python3 annotate_disasm.py <disasm_file> <o2romsrc_file>", file=sys.stderr)
        sys.exit(1)
    
    annotate_disasm(sys.argv[1], sys.argv[2])
