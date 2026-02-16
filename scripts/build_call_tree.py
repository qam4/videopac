#!/usr/bin/env python3
import sys
import re
import argparse

def load_labels(bios_path, rom_path):
    labels = {}
    
    # Load from BIOS - format: "0x000: 84 00  JMP restart"
    # Labels are on lines before the address, like "cold_boot:"
    try:
        with open(bios_path, 'r') as f:
            current_label = None
            for line in f:
                # Check for label (ends with :, no leading whitespace, not hex address)
                if line and not line[0].isspace() and ':' in line and not line.startswith('0x'):
                    label_match = re.match(r'^(\w+):', line)
                    if label_match:
                        current_label = label_match.group(1)
                # Check for address line
                elif line.startswith('0x'):
                    addr_match = re.match(r'^(0x[0-9a-fA-F]+):', line)
                    if addr_match and current_label:
                        addr = addr_match.group(1).lower()
                        labels[addr] = 'bios:' + current_label
                        current_label = None
                    
                    # Also extract labels from CALL/JMP operands that reference ROM addresses
                    # Format: "0x2f0: 84 08  JMP end_of_select_game"
                    if addr_match:
                        operand_match = re.search(r'(?:CALL|JMP)\s+([a-z_][a-z0-9_]+)', line)
                        if operand_match:
                            label_name = operand_match.group(1)
                            # Extract the target address from the opcode
                            opcode_match = re.search(r':\s+([0-9a-fA-F]{2})\s+([0-9a-fA-F]{2})', line)
                            if opcode_match:
                                # For JMP/CALL, second byte is low nibble, first byte's low nibble is high
                                byte1 = int(opcode_match.group(1), 16)
                                byte2 = int(opcode_match.group(2), 16)
                                # 8048 addressing: opcode is 0x84 (JMP) or 0x34 (CALL)
                                # Address is: (byte1 & 0x0F) << 8 | byte2
                                target_addr = ((byte1 & 0x0F) << 8) | byte2
                                target_addr_str = f'0x{target_addr:x}'
                                # Add with priority - BIOS labels for ROM addresses are preferred
                                labels[target_addr_str] = 'rom:' + label_name
                
                # Keep the label if we see a comment or blank line
                elif current_label and (line.startswith(';') or line.strip() == ''):
                    # Keep current_label for next address line
                    pass
                else:
                    # Reset label if we see something else
                    if current_label and not line.startswith(';') and line.strip() != '':
                        current_label = None
    except Exception as e:
        print(f"Error loading BIOS labels: {e}")
    
    # Load from game - format: "0400: [ 44 C3 ] JMP selectgame"
    # Also format: "vsync:" on its own line, then "0600: [ 55 ] STRT T"
    # NOTE: ROM addresses wrap after 0x7FF back to 0x0000 (which is actually 0x800 in ROM)
    try:
        # First pass: build wrap detection map
        wrap_map = rom_address_to_absolute(rom_path)
        
        with open(rom_path, 'r') as f:
            current_label = None
            
            for line_num, line in enumerate(f, 1):
                # Check for standalone label (like "vsync:")
                if line and not line[0].isspace() and line.strip().endswith(':') and not line.strip()[0].isdigit():
                    label_match = re.match(r'^([a-z_][a-z0-9_]*):', line.strip())
                    if label_match:
                        current_label = label_match.group(1)
                        continue
                
                # Check for address line
                addr_match = re.match(r'^([0-9A-Fa-f]{4}):', line)
                if addr_match:
                    addr_hex = addr_match.group(1).lower()
                    after_wrap = wrap_map.get(line_num, False)
                    addr = translate_rom_address(addr_hex, after_wrap)
                    
                    # If we have a pending label from previous line, use it
                    if current_label:
                        labels[addr] = 'rom:' + current_label
                        current_label = None
                    
                    # Also check for inline label in operand (but NOT for CALL/JMP instructions)
                    # because those are targets, not labels for the current address
                    match = re.match(r'^[0-9A-Fa-f]{4}:\s*\[.*?\]\s*(\w+)\s+(\w+)', line)
                    if match:
                        instr = match.group(1)
                        label = match.group(2)
                        # Skip CALL and JMP instructions - their operands are targets, not labels
                        # Skip numeric labels, register names, and only use if not already labeled
                        # Valid labels start with lowercase letter and are at least 2 chars
                        if (instr not in ['CALL', 'JMP'] and
                            not label[0].isdigit() and 
                            not label[0].isupper() and 
                            len(label) >= 2 and 
                            addr not in labels):
                            labels[addr] = 'rom:' + label
                            labels[addr] = 'rom:' + label
    except Exception as e:
        print(f"Error loading game labels: {e}")
    
    # Add MB1 address variants for ROM labels
    # When SEL MB1 is active, CALL/JMP addresses have bit 11 set (add 0x800)
    # The trace shows raw PC addresses (e.g., 0x976), but labels are stored with ROM base offset
    # So we need to add labels for both:
    # 1. Raw PC address with MB1: 0x176 + 0x800 = 0x976
    # 2. Translated address with MB1: (0x176 + 0x400) + 0x800 = 0xd76 (already added above)
    mb1_labels = {}
    for addr_str, label in labels.items():
        if label.startswith('rom:'):
            # Parse address
            addr_int = int(addr_str, 16)
            # If address is in ROM range (0x400-0xBFF or 0xC00-0x13FF after translation)
            # Add raw PC variant by subtracting ROM base (0x400)
            if 0x400 <= addr_int <= 0xBFF:
                # This is a ROM address before MB1 translation
                # Raw PC would be: addr_int - 0x400
                # With MB1: (addr_int - 0x400) + 0x800
                raw_pc_mb1 = addr_int - 0x400 + 0x800
                mb1_addr_str = f'0x{raw_pc_mb1:x}'
                mb1_labels[mb1_addr_str] = label
            elif 0xC00 <= addr_int <= 0x13FF:
                # This is already an MB1-translated address
                # Raw PC would be: addr_int - 0x400 - 0x800 + 0x800 = addr_int - 0x400
                raw_pc = addr_int - 0x400
                raw_pc_str = f'0x{raw_pc:x}'
                if raw_pc_str not in labels:
                    mb1_labels[raw_pc_str] = label
    
    # Merge MB1 labels into main labels dictionary
    labels.update(mb1_labels)
    
    return labels

def find_frame_boundaries(trace_path):
    """Find the starting cycle for each frame"""
    frame_starts = {}
    with open(trace_path, 'r') as f:
        for line in f:
            match = re.match(r'\[F:(\d+)\s+C:(\d+)\]', line)
            if match:
                frame = int(match.group(1))
                cycle = int(match.group(2))
                if frame not in frame_starts:
                    frame_starts[frame] = cycle
    return frame_starts

def rom_address_to_absolute(rom_path):
    """
    Convert ROM disassembly address to absolute memory address.
    
    ROM is mapped at 0x400 in memory.
    The disassembly file has addresses that wrap after 0x7FF back to 0x0000.
    
    Examples:
    - "0400" -> 0x800 (0x400 ROM offset + 0x400 base = 0x800)
    - "0176" (before wrap) -> 0x576 (0x176 + 0x400)
    - "0176" (after wrap) -> 0xd76 (0x176 + 0x800 + 0x400)
    
    We detect the wrap by checking if we've seen address 0x7FF yet.
    """
    # Build a map of line number to whether we're after the wrap
    wrap_map = {}
    seen_7ff = False
    
    with open(rom_path, 'r') as f:
        for line_num, line in enumerate(f, 1):
            addr_match = re.match(r'^([0-9A-Fa-f]{4}):', line)
            if addr_match:
                addr_hex = addr_match.group(1).lower()
                addr_int = int(addr_hex, 16)
                
                if addr_int == 0x7ff:
                    seen_7ff = True
                
                wrap_map[line_num] = seen_7ff
    
    return wrap_map

def translate_rom_address(addr_hex, after_wrap):
    """
    Translate a ROM address to absolute memory address.
    
    The ROM disassembly uses addresses that are already absolute (0x400-0xBFF).
    But after 0x7FF, it wraps back to 0x0000, which represents 0xC00-0xFFF.
    
    Args:
        addr_hex: Address as hex string (e.g., "0400")
        after_wrap: Boolean indicating if this address is after the 0x7FF wrap
    
    Returns:
        Absolute memory address as string (e.g., "0x400")
    """
    addr_int = int(addr_hex, 16)
    
    # After wrap, addresses 0x000-0x3FF are actually 0xC00-0xFFF in memory
    if after_wrap and addr_int < 0x400:
        addr_int += 0xC00
    
    return f'0x{addr_int:x}'

def check_frame_ending_loop(frame_num, frame_starts, trace_path):
    """Check if a frame ends with a loop back-edge, return loop info if so"""
    frame_start_cycle = frame_starts.get(frame_num, 0)
    next_frame_start = frame_starts.get(frame_num + 1, float('inf'))
    
    last_instr = None
    with open(trace_path, 'r') as f:
        in_frame = False
        for line in f:
            # Match annotated trace: [F:0 C:0] 0x000: 84 00 JMP restart | A=00...
            match = re.match(r'\[F:(\d+)\s+C:(\d+)\]\s+(0x[0-9a-f]+):\s+([0-9a-f]+)\s+([0-9a-f]*)\s+(\S+)', line)
            if not match:
                continue
            
            current_frame = int(match.group(1))
            addr = match.group(3)
            instr = match.group(6)
            
            if current_frame == frame_num:
                in_frame = True
                last_instr = {'addr': addr, 'instr': instr, 'line': line}
            elif in_frame:
                break
    
    # Check if last instruction was a loop back-edge
    # All conditional and unconditional jump instructions
    jump_instrs = ['JMP', 'JNZ', 'JZ', 'DJNZ', 'JNC', 'JC', 'JF0', 'JF1', 
                   'JT0', 'JNT0', 'JT1', 'JNT1', 'JTF', 'JNI',
                   'JB0', 'JB1', 'JB2', 'JB3', 'JB4', 'JB5', 'JB6', 'JB7']
    if last_instr and last_instr['instr'] in jump_instrs:
        # Build regex pattern for all jump instructions
        pattern = r'(?:' + '|'.join(jump_instrs) + r')\s+(?:R\d+,)?(0x[0-9a-f]+)'
        target_match = re.search(pattern, last_instr['line'])
        if target_match:
            target = target_match.group(1)
            try:
                addr_int = int(last_instr['addr'], 16)
                target_int = int(target, 16)
                # Check if it's a backward jump (loop)
                if target_int < addr_int and abs(target_int - addr_int) < 0x30:
                    return {'from_addr': last_instr['addr'], 'to_addr': target}
            except:
                pass
    
    return None

def build_call_tree(frame_num, labels, frame_starts, trace_path):
    call_stack = []
    events = []
    loop_regions = []  # Track loop regions: (start_cycle, end_cycle, from_addr, to_addr, iterations)
    current_loop = None  # Track current loop: {from_addr, to_addr, start_cycle, start_abs_cycle, iterations}
    
    frame_start_cycle = frame_starts.get(frame_num, 0)
    next_frame_start = frame_starts.get(frame_num + 1, float('inf'))
    frame_end_cycle = next_frame_start - frame_start_cycle - 1  # Last cycle of this frame (relative)
    
    # Check if previous frame ended with a loop - if so, we might be continuing it
    prev_frame_loop = None
    if frame_num > 0:
        prev_frame_loop = check_frame_ending_loop(frame_num - 1, frame_starts, trace_path)
    
    with open(trace_path, 'r') as f:
        in_frame = False
        prev_addr = None
        prev_cycle = None
        prev_abs_cycle = None
        loop_closed_at_boundary = False
        first_instruction_addr = None
        
        for line in f:
            # Match annotated trace: [F:0 C:0] 0x000: 84 00 JMP restart | A=00...
            # Capture the full instruction text including operand/label
            match = re.match(r'\[F:(\d+)\s+C:(\d+)\]\s+(0x[0-9a-f]+):\s+([0-9a-f]+)\s+([0-9a-f]*)\s+(.+?)\s*\|', line)
            if not match:
                continue
            
            current_frame = int(match.group(1))
            abs_cycle = int(match.group(2))
            addr = match.group(3)
            instr_full = match.group(6).strip()  # Full instruction like "CALL reset" or "JMP bios:select_game"
            
            # Extract just the mnemonic
            instr_parts = instr_full.split(None, 1)
            instr = instr_parts[0] if instr_parts else instr_full
            
            if current_frame == frame_num:
                in_frame = True
                # Record first instruction address
                if first_instruction_addr is None:
                    first_instruction_addr = addr
                    # Check if we're continuing a loop from previous frame
                    if prev_frame_loop and addr == prev_frame_loop['to_addr']:
                        # We're continuing the loop - mark it as starting at cycle 0
                        current_loop = {
                            'from_addr': prev_frame_loop['from_addr'],
                            'to_addr': prev_frame_loop['to_addr'],
                            'start_cycle': 0,
                            'start_abs_cycle': frame_start_cycle,
                            'iterations': 0  # Will be incremented when we see the first back-edge
                        }
            elif in_frame:
                # We've moved to the next frame - close any open loop at frame boundary
                if current_loop and prev_cycle is not None:
                    loop_start_rel = max(0, current_loop['start_abs_cycle'] - frame_start_cycle)
                    # End the loop at the frame boundary
                    if prev_cycle > loop_start_rel:
                        loop_regions.append((
                            loop_start_rel,
                            prev_cycle,
                            current_loop['from_addr'],
                            current_loop['to_addr'],
                            current_loop['iterations']
                        ))
                    loop_closed_at_boundary = True
                break
            else:
                continue
            
            # Frame-relative cycle
            cycle = abs_cycle - frame_start_cycle
            
            # Check for interrupt (jump to 0x003 or 0x009)
            if addr in ['0x3', '0x9'] and prev_addr and prev_addr not in ['0x3', '0x9']:
                events.append(('INTERRUPT', cycle, addr, '', len(call_stack)))
            
            addr_label = labels.get(addr, addr)
            
            # Detect loop exit: if we were in a loop and now we're not at the loop addresses
            if current_loop and prev_cycle is not None:
                loop_from = current_loop['from_addr']
                loop_to = current_loop['to_addr']
                try:
                    addr_int = int(addr, 16)
                    from_int = int(loop_from, 16)
                    to_int = int(loop_to, 16)
                    # Check if we've exited the loop region
                    if addr_int < to_int or addr_int > from_int:
                        # Loop ended
                        loop_start_rel = max(0, current_loop['start_abs_cycle'] - frame_start_cycle)
                        if prev_cycle > loop_start_rel:  # Only add if loop has duration in this frame
                            loop_regions.append((
                                loop_start_rel,
                                prev_cycle,
                                loop_from,
                                loop_to,
                                current_loop['iterations']
                            ))
                        current_loop = None
                except:
                    pass
            
            if instr == 'CALL':
                # Extract target label from instruction text (e.g., "CALL reset" or "CALL bios:display_off")
                if len(instr_parts) > 1:
                    target_label = instr_parts[1]
                else:
                    # Fallback: calculate from bytes
                    byte0 = match.group(4)
                    byte1 = match.group(5)
                    if byte1:
                        try:
                            opcode = int(byte0, 16)
                            low_byte = int(byte1, 16)
                            target_int = ((opcode & 0xE0) << 3) | low_byte
                            target = f'0x{target_int:x}'
                            target_label = labels.get(target, target)
                        except:
                            target_label = '???'
                    else:
                        target_label = '???'
                
                call_stack.append(addr)
                events.append(('CALL', cycle, addr_label, target_label, len(call_stack) - 1))
            elif instr in ['RET', 'RETR']:
                if call_stack:
                    from_addr = call_stack.pop()
                    from_label = labels.get(from_addr, from_addr)
                    events.append(('RET', cycle, addr_label, from_label, len(call_stack)))
            
            elif instr in ['JMP', 'JNZ', 'JZ', 'DJNZ', 'JNC', 'JC', 'JF0', 'JF1',
                           'JT0', 'JNT0', 'JT1', 'JNT1', 'JTF', 'JNI',
                           'JB0', 'JB1', 'JB2', 'JB3', 'JB4', 'JB5', 'JB6', 'JB7']:
                # Extract target address from instruction bytes
                byte0 = match.group(4)
                byte1 = match.group(5)
                if byte1:  # Jump instructions are 2-byte
                    try:
                        opcode = int(byte0, 16)
                        low_byte = int(byte1, 16)
                        # 8048 JMP: target = ((opcode & 0xE0) << 3) | low_byte
                        # Conditional jumps use different encoding
                        if instr == 'JMP':
                            target_int = ((opcode & 0xE0) << 3) | low_byte
                        elif instr == 'DJNZ':
                            # DJNZ: relative jump, byte1 is signed offset
                            target_int = (int(addr, 16) + 2 + (low_byte if low_byte < 128 else low_byte - 256)) & 0xFFF
                        else:
                            # Other conditional jumps: page-relative
                            target_int = (int(addr, 16) & 0xF00) | low_byte
                        
                        target = f'0x{target_int:x}'
                        addr_int = int(addr, 16)
                        
                        # Check if it's a backward jump (potential loop)
                        if target_int < addr_int and abs(target_int - addr_int) < 0x30:
                            # This is a loop back-edge
                            if current_loop and current_loop['from_addr'] == addr and current_loop['to_addr'] == target:
                                # Same loop, increment counter
                                current_loop['iterations'] += 1
                            else:
                                # New loop started
                                if current_loop and prev_cycle is not None:
                                    # Save previous loop
                                    loop_start_rel = max(0, current_loop['start_abs_cycle'] - frame_start_cycle)
                                    if prev_cycle > loop_start_rel:  # Only add if loop has duration in this frame
                                        loop_regions.append((
                                            loop_start_rel,
                                            prev_cycle,
                                            current_loop['from_addr'],
                                            current_loop['to_addr'],
                                            current_loop['iterations']
                                        ))
                                current_loop = {
                                    'from_addr': addr,
                                    'to_addr': target,
                                    'start_cycle': cycle,
                                    'start_abs_cycle': abs_cycle,
                                    'iterations': 1
                                }
                        elif instr == 'JMP':
                            # Significant jump (only for unconditional JMP), record it
                            target_label = labels.get(target, target)
                            events.append(('JMP', cycle, addr_label, target_label, len(call_stack)))
                    except:
                        pass
            
            prev_addr = addr
            prev_cycle = cycle
            prev_abs_cycle = abs_cycle
        
        # Close any open loop at end of frame (only if not already closed at boundary)
        if current_loop and prev_cycle is not None and not loop_closed_at_boundary:
            loop_start_rel = max(0, current_loop['start_abs_cycle'] - frame_start_cycle)
            if prev_cycle > loop_start_rel:  # Only add if loop has duration in this frame
                loop_regions.append((
                    loop_start_rel,
                    prev_cycle,
                    current_loop['from_addr'],
                    current_loop['to_addr'],
                    current_loop['iterations']
                ))
    
    return events, loop_regions, len(call_stack)

def merge_events_and_loops(events, loop_regions, labels):
    """Merge events and loop regions into a single timeline"""
    timeline = []
    
    # Add all events
    for event in events:
        event_type, cycle, from_label, to_label, depth = event
        timeline.append((cycle, 'event', event_type, from_label, to_label, depth))
    
    # Add loop regions
    for start_cycle, end_cycle, from_addr, to_addr, iterations in loop_regions:
        from_label = labels.get(from_addr, from_addr)
        to_label = labels.get(to_addr, to_addr)
        timeline.append((start_cycle, 'loop', from_label, to_label, end_cycle, iterations))
    
    # Sort by cycle
    timeline.sort(key=lambda x: x[0])
    
    return timeline

if __name__ == '__main__':
    parser = argparse.ArgumentParser(
        description='Generate call tree from trace log',
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  %(prog)s 0
  %(prog)s 0 1 2 3
  %(prog)s --bios doc/french_bios_annotated.txt --rom doc/satellite-attack-disassembly.txt 0
        """
    )
    
    parser.add_argument(
        'frames',
        metavar='FRAME',
        type=int,
        nargs='+',
        help='Frame number(s) to analyze'
    )
    
    parser.add_argument(
        '--bios',
        default='doc/french_bios_annotated.txt',
        help='Path to annotated BIOS disassembly (default: doc/french_bios_annotated.txt)'
    )
    
    parser.add_argument(
        '--rom',
        default='doc/satellite-attack-disassembly.txt',
        help='Path to annotated ROM disassembly (default: doc/satellite-attack-disassembly.txt)'
    )
    
    parser.add_argument(
        '--trace',
        default='trace.log',
        help='Path to trace log file (default: trace.log)'
    )
    
    args = parser.parse_args()
    
    labels = load_labels(args.bios, args.rom)
    frame_starts = find_frame_boundaries(args.trace)
    print(f"Loaded {len(labels)} labels\n")
    
    prev_frame_depth = 0  # Track call stack depth across frames
    
    for frame_num in args.frames:
        print(f"=== Frame {frame_num} Call Tree ===\n")
        
        events, loop_regions, final_depth = build_call_tree(frame_num, labels, frame_starts, args.trace)
        timeline = merge_events_and_loops(events, loop_regions, labels)
        
        # Start with the depth from the previous frame
        current_depth = prev_frame_depth
        
        for item in timeline[:200]:
            if item[1] == 'event':
                cycle, _, event_type, from_label, to_label, depth = item
                indent = '  ' * depth
                if event_type == 'CALL':
                    print(f"{indent}[{cycle:6d}] {from_label} -> CALL {to_label}")
                    current_depth = depth + 1
                elif event_type == 'RET':
                    print(f"{indent}[{cycle:6d}] {from_label} <- RET")
                    current_depth = depth
                elif event_type == 'JMP':
                    print(f"{indent}[{cycle:6d}] {from_label} -> JMP {to_label}")
                elif event_type == 'INTERRUPT':
                    print(f"{indent}[{cycle:6d}] *** INTERRUPT -> {from_label} ***")
            elif item[1] == 'loop':
                cycle, _, from_label, to_label, end_cycle, iterations = item
                indent = '  ' * current_depth
                duration = end_cycle - cycle
                print(f"{indent}[{cycle:6d}..{end_cycle:6d}] LOOP {from_label} -> {to_label}: {iterations} iterations ({duration} cycles)")
        
        if len(timeline) > 200:
            print(f"\n... ({len(timeline) - 200} more items)")
        
        print(f"\nTotal events: {len(events)}")
        print(f"Total loop regions: {len(loop_regions)}\n")
        
        # Save the final depth for the next frame
        prev_frame_depth = final_depth

