#!/usr/bin/env python3
"""
Annotate trace log with disassembled instructions.

Reads a trace log with raw instruction bytes and adds disassembled
instruction mnemonics by looking them up in BIOS/ROM disassembly files.
"""

import sys
import re
import argparse


def load_disassembly(bios_path, rom_path):
    """Load disassembly from BIOS and ROM files into dict: addr -> (label, instruction)"""
    disasm = {}
    
    # Process both BIOS and ROM files with the same logic
    for file_path in [bios_path, rom_path]:
        try:
            # Try UTF-8 first, then UTF-16 if that fails
            encodings = ['utf-8', 'utf-16']
            content = None
            for encoding in encodings:
                try:
                    with open(file_path, 'r', encoding=encoding) as f:
                        content = f.readlines()
                    break
                except (UnicodeDecodeError, UnicodeError):
                    continue
            
            if content is None:
                print(f"Warning: Could not decode file: {file_path}", file=sys.stderr)
                continue
            
            current_label = None
            for line in content:
                # Check for label line (ends with colon, no leading whitespace)
                label_match = re.match(r'^(\w+):\s*$', line.strip())
                if label_match:
                    current_label = label_match.group(1)
                    continue
                
                # Format: "0x400: 44 c3  JMP bios:select_game"
                # Match address, hex bytes, and instruction
                match = re.match(r'^(0x[0-9A-Fa-f]+):\s+[0-9a-fA-F\s]+\s+(.+?)(?:\s*$)', line)
                if match:
                    addr = match.group(1).lower()
                    instr = match.group(2).strip()
                    disasm[addr] = (current_label, instr)
                    current_label = None  # Label only applies to first instruction
        except FileNotFoundError:
            print(f"Warning: Disassembly file not found: {file_path}", file=sys.stderr)
    
    return disasm


def annotate_trace(trace_path, output_path, disasm):
    """Annotate trace log with disassembled instructions and labels"""
    with open(trace_path, 'r') as infile, open(output_path, 'w') as outfile:
        for line in infile:
            # Format: "[F:0 C:0] 0x000: 84 00 | A=00 PSW=00 P1=ff P2=ff RB0 F1=0"
            match = re.match(r'^(\[F:\d+ C:\d+\] )(0x[0-9a-f]+)(: [0-9a-f]+ [0-9a-f]*) (\| .+)$', line)
            if match:
                prefix = match.group(1)
                addr = match.group(2)
                bytes_part = match.group(3)
                registers = match.group(4)
                
                # Look up instruction and label
                label, instr = disasm.get(addr, (None, '???'))
                
                # Format output with optional label prefix
                if label:
                    outfile.write(f'{prefix}{addr}{bytes_part} {label}: {instr} {registers}\n')
                else:
                    outfile.write(f'{prefix}{addr}{bytes_part} {instr} {registers}\n')
            else:
                # Pass through unchanged
                outfile.write(line)


def main():
    parser = argparse.ArgumentParser(
        description='Annotate trace log with disassembled instructions',
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  %(prog)s trace.log trace_annotated.log
  %(prog)s --bios doc/french_bios_annotated.txt --rom racing_game_disasm.txt trace.log trace_annotated.log
        """
    )
    
    parser.add_argument(
        'input',
        help='Input trace log file'
    )
    
    parser.add_argument(
        'output',
        help='Output annotated trace log file'
    )
    
    parser.add_argument(
        '--bios',
        default='doc/french_bios_annotated.txt',
        help='Path to BIOS disassembly (default: doc/french_bios_annotated.txt)'
    )
    
    parser.add_argument(
        '--rom',
        default='racing_game_disasm.txt',
        help='Path to ROM disassembly (default: racing_game_disasm.txt)'
    )
    
    args = parser.parse_args()
    
    print(f"Loading disassembly from {args.bios} and {args.rom}...")
    disasm = load_disassembly(args.bios, args.rom)
    print(f"Loaded {len(disasm)} instructions")
    
    print(f"Annotating {args.input} -> {args.output}...")
    annotate_trace(args.input, args.output, disasm)
    print("Done!")


if __name__ == '__main__':
    main()
