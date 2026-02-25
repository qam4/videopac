#!/usr/bin/env python3
"""
Comprehensive trace comparison tool for o2em vs our emulator.
Combines divergence detection, reconvergence analysis, and frame-specific checks.
"""

import sys
import argparse


def load_trace(filename):
    """Load trace file, skipping comments"""
    trace = []
    with open(filename, 'r') as f:
        for line in f:
            line = line.strip()
            if not line or line.startswith('#'):
                continue

            # Check if this is o2em format (PC OP P1 ACC BANK)
            parts = line.split()
            if len(parts) >= 5 and not line.startswith('['):
                try:
                    pc = int(parts[0], 16)
                    op = int(parts[1], 16)
                    p1 = int(parts[2], 16)
                    acc = int(parts[3], 16)
                    bank = int(parts[4])
                    trace.append((pc, op, p1, acc, bank))
                except ValueError:
                    continue
            # Check if this is our format: [F:0 C:0] 0x000: 84 00 | A=00...
            elif line.startswith('['):
                try:
                    # Extract PC
                    pc_start = line.index('] ') + 2
                    pc_end = line.index(':', pc_start)
                    pc_str = line[pc_start:pc_end].strip()
                    pc = int(pc_str, 16)

                    # Extract opcode
                    op_start = pc_end + 2
                    op_end = line.index(' ', op_start)
                    op = int(line[op_start:op_end], 16)

                    # Extract P1
                    p1_start = line.index('P1=') + 3
                    p1_end = line.index(' ', p1_start)
                    p1 = int(line[p1_start:p1_end], 16)

                    # Extract ACC
                    a_start = line.index('A=') + 2
                    a_end = line.index(' ', a_start)
                    acc = int(line[a_start:a_end], 16)

                    # Determine bank from P1
                    bank = (~p1) & 0x03

                    trace.append((pc, op, p1, acc, bank))
                except (ValueError, IndexError):
                    continue
    return trace


def find_divergences(o2em_trace, our_trace):
    """Find all divergence and convergence points"""
    min_len = min(len(o2em_trace), len(our_trace))
    divergences = []
    in_divergence = False
    divergence_start = 0

    for i in range(min_len):
        o2em_pc, o2em_op, o2em_p1, o2em_acc, o2em_bank = o2em_trace[i]
        our_pc, our_op, our_p1, our_acc, our_bank = our_trace[i]

        matches = (o2em_pc == our_pc and o2em_op == our_op and
                   o2em_p1 == our_p1 and o2em_acc == our_acc and
                   o2em_bank == our_bank)

        if not matches and not in_divergence:
            # Start of divergence
            in_divergence = True
            divergence_start = i
        elif matches and in_divergence:
            # End of divergence - reconverged
            divergence_len = i - divergence_start
            divergences.append((divergence_start, i, divergence_len, True))
            in_divergence = False

    # Check if still diverged at end
    if in_divergence:
        divergence_len = min_len - divergence_start
        divergences.append((divergence_start, min_len, divergence_len, False))

    return divergences


def print_instruction(idx, o2em_data, our_data, label=""):
    """Print instruction comparison"""
    o_pc, o_op, o_p1, o_acc, o_bank = o2em_data
    u_pc, u_op, u_p1, u_acc, u_bank = our_data

    matches = (o_pc == u_pc and o_op == u_op and o_p1 == u_p1 and
               o_acc == u_acc and o_bank == u_bank)
    status = "OK" if matches else "XX"

    print(f"  {idx:7d} {status}: o2em: PC={o_pc:03X} OP={o_op:02X} "
          f"P1={o_p1:02X} ACC={o_acc:02X} BANK={o_bank} {label}")
    print(f"          {status}:  ours: PC={u_pc:03X} OP={u_op:02X} "
          f"P1={u_p1:02X} ACC={u_acc:02X} BANK={u_bank}")


def show_first_divergence(o2em_trace, our_trace):
    """Show detailed view of first divergence"""
    min_len = min(len(o2em_trace), len(our_trace))

    for i in range(min_len):
        o2em_data = o2em_trace[i]
        our_data = our_trace[i]

        if o2em_data != our_data:
            print("\n" + "="*70)
            print(f"FIRST DIVERGENCE AT INSTRUCTION {i}")
            print("="*70)

            print("\nContext (5 instructions before):")
            for j in range(max(0, i-5), i):
                print_instruction(j, o2em_trace[j], our_trace[j])

            print("\n>>> DIVERGENCE POINT:")
            print_instruction(i, o2em_data, our_data)

            o_pc, o_op, o_p1, o_acc, o_bank = o2em_data
            u_pc, u_op, u_p1, u_acc, u_bank = our_data

            print("\nDifferences:")
            if o_pc != u_pc:
                print(f"  PC:   o2em={o_pc:03X} ({o_pc:4d})  "
                      f"ours={u_pc:03X} ({u_pc:4d})  diff={u_pc-o_pc:+d}")
            if o_op != u_op:
                print(f"  OP:   o2em={o_op:02X}  ours={u_op:02X}  "
                      f"<-- DIFFERENT INSTRUCTION!")
            if o_p1 != u_p1:
                print(f"  P1:   o2em={o_p1:02X} (0b{o_p1:08b})  "
                      f"ours={u_p1:02X} (0b{u_p1:08b})")
            if o_acc != u_acc:
                print(f"  ACC:  o2em={o_acc:02X} ({o_acc:3d})  "
                      f"ours={u_acc:02X} ({u_acc:3d})  diff={u_acc-o_acc:+d}")
            if o_bank != u_bank:
                print(f"  BANK: o2em={o_bank}  ours={u_bank}")

            print("\nNext 5 instructions:")
            for j in range(i+1, min(i+6, min_len)):
                print_instruction(j, o2em_trace[j], our_trace[j])

            return True

    return False


def show_divergence_summary(divergences, instructions_per_frame=5960):
    """Show summary of all divergences"""
    print("\n" + "="*70)
    print("DIVERGENCE SUMMARY")
    print("="*70)
    print(f"\nTotal divergences: {len(divergences)}")
    reconverged = sum(1 for _, _, _, r in divergences if r)
    print(f"Reconverged: {reconverged}")
    print(f"Permanent: {len(divergences) - reconverged}")

    if len(divergences) == 0:
        print("\n✓ Traces match perfectly!")
        return

    print("\nDivergence details:")
    for idx, (start, end, length, reconverged) in enumerate(divergences[:20]):
        start_frame = start // instructions_per_frame
        end_frame = end // instructions_per_frame
        status = "✓ reconverged" if reconverged else "✗ PERMANENT"

        print(f"\n  #{idx+1}: Instructions {start:,}-{end:,} "
              f"(frames {start_frame}-{end_frame})")
        print(f"       Length: {length:,} instructions  {status}")

    if len(divergences) > 20:
        print(f"\n  ... and {len(divergences) - 20} more divergences")

    # Check for permanent divergence
    permanent = [d for d in divergences if not d[3]]
    if permanent:
        print("\n" + "="*70)
        print("⚠ WARNING: PERMANENT DIVERGENCE DETECTED")
        print("="*70)
        start, end, length, _ = permanent[0]
        print(f"\nDivergence starts at instruction {start:,} "
              f"(frame {start // instructions_per_frame})")
        print("This indicates a real bug, not just timing differences.")
    else:
        print("\n" + "="*70)
        print("✓ ALL DIVERGENCES RECONVERGE")
        print("="*70)
        print("\nAll divergences are timing-related (interrupts, etc.)")


def check_frame_range(o2em_trace, our_trace, frame_num,
                      instructions_per_frame=5960, window=50):
    """Check for divergences around a specific frame"""
    target = frame_num * instructions_per_frame
    min_len = min(len(o2em_trace), len(our_trace))

    print("\n" + "="*70)
    print(f"CHECKING FRAME {frame_num} (instruction ~{target:,})")
    print("="*70)

    if target >= min_len:
        print(f"\n⚠ Frame {frame_num} is beyond trace length "
              f"({min_len:,} instructions)")
        return

    start = max(0, target - window)
    end = min(target + window, min_len)

    diverged = False
    for i in range(start, end):
        if o2em_trace[i] != our_trace[i]:
            diverged = True
            break

    if diverged:
        print(f"\n✗ DIVERGENCE DETECTED around frame {frame_num}")
        print(f"\nShowing instructions {start:,} to {end:,}:")
        for i in range(start, end):
            print_instruction(i, o2em_trace[i], our_trace[i])
    else:
        print(f"\n✓ No divergence around frame {frame_num} "
              f"(checked {start:,}-{end:,})")


def main():
    parser = argparse.ArgumentParser(
        description='Compare o2em and our emulator traces',
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  # Show first divergence
  %(prog)s o2em_trace.log trace_cpu.log

  # Show all divergences summary
  %(prog)s o2em_trace.log trace_cpu.log --all

  # Check specific frame
  %(prog)s o2em_trace.log trace_cpu.log --frame 606

  # Full analysis
  %(prog)s o2em_trace.log trace_cpu.log --all --frame 606
        """)

    parser.add_argument('o2em_trace', help='O2EM trace file')
    parser.add_argument('our_trace', help='Our emulator trace file')
    parser.add_argument('--all', action='store_true',
                        help='Show all divergences summary')
    parser.add_argument('--frame', type=int,
                        help='Check specific frame number')
    parser.add_argument('--ipf', type=int, default=5960,
                        help='Instructions per frame (default: 5960)')

    args = parser.parse_args()

    print("Loading traces...")
    print(f"  o2em: {args.o2em_trace}")
    o2em_trace = load_trace(args.o2em_trace)
    print(f"    Loaded {len(o2em_trace):,} instructions")

    print(f"  ours: {args.our_trace}")
    our_trace = load_trace(args.our_trace)
    print(f"    Loaded {len(our_trace):,} instructions")

    # Always show first divergence unless only --frame specified
    if not args.frame or args.all:
        if not show_first_divergence(o2em_trace, our_trace):
            print("\n✓ No divergences found!")

    # Show all divergences if requested
    if args.all:
        divergences = find_divergences(o2em_trace, our_trace)
        show_divergence_summary(divergences, args.ipf)

    # Check specific frame if requested
    if args.frame:
        check_frame_range(o2em_trace, our_trace, args.frame, args.ipf)


if __name__ == '__main__':
    main()
