#!/usr/bin/env python3
"""
Compare headless vs SDL CPU traces to find divergences.
Both traces use our format: [F:0 C:0] 0x000: 84 00 | A=00 PSW=00 P1=BF P2=FF RB0
"""

import sys
import argparse
import re


def parse_line(line):
    """Parse a trace line in our format. Returns dict or None."""
    line = line.strip()
    if not line or not line.startswith('['):
        return None
    try:
        # Full format: [F:0 C:123] 0x000: 84 00 | A=00 PSW=00 P1=BF P2=FF RB0 F1=0 | R0=...
        m = re.match(
            r'\[F:(\d+)\s+C:(\d+)\]\s+0x([0-9a-fA-F]+):\s+([0-9a-fA-F]+)\s+[0-9a-fA-F]+\s+\|\s+'
            r'A=([0-9a-fA-F]+)\s+PSW=([0-9a-fA-F]+)\s+P1=([0-9a-fA-F]+)\s+P2=([0-9a-fA-F]+)\s+'
            r'RB(\d)\s+F1=(\d)',
            line
        )
        if not m:
            return None
        return {
            'frame': int(m.group(1)),
            'cycle': int(m.group(2)),
            'pc': int(m.group(3), 16),
            'op': int(m.group(4), 16),
            'a': int(m.group(5), 16),
            'psw': int(m.group(6), 16),
            'p1': int(m.group(7), 16),
            'p2': int(m.group(8), 16),
            'rb': int(m.group(9)),
            'f1': int(m.group(10)),
            'raw': line,
        }
    except (ValueError, IndexError):
        return None


def load_trace(filename, max_lines=None):
    """Load trace file, return list of parsed entries."""
    entries = []
    with open(filename, 'r') as f:
        for line in f:
            parsed = parse_line(line)
            if parsed:
                entries.append(parsed)
                if max_lines and len(entries) >= max_lines:
                    break
    return entries


def compare_field(a, b, field):
    """Compare a field, return diff string or None."""
    va, vb = a[field], b[field]
    if va == vb:
        return None
    if isinstance(va, int):
        return f"{field.upper()}: hl={va:02X} sdl={vb:02X}"
    return f"{field.upper()}: hl={va} sdl={vb}"


def find_first_divergence(hl, sdl):
    """Find first instruction where traces differ."""
    min_len = min(len(hl), len(sdl))
    fields = ['pc', 'op', 'a', 'psw', 'p1', 'p2', 'rb', 'f1']

    for i in range(min_len):
        diffs = []
        for f in fields:
            d = compare_field(hl[i], sdl[i], f)
            if d:
                diffs.append(d)
        if diffs:
            return i, diffs
    return None, None


def print_context(hl, sdl, idx, before=5, after=10):
    """Print trace context around a divergence point."""
    min_len = min(len(hl), len(sdl))
    start = max(0, idx - before)
    end = min(idx + after + 1, min_len)

    for i in range(start, end):
        marker = ">>>" if i == idx else "   "
        h, s = hl[i], sdl[i]
        match = "OK" if h['pc'] == s['pc'] and h['op'] == s['op'] and h['a'] == s['a'] and h['p1'] == s['p1'] and h['p2'] == s['p2'] and h['f1'] == s['f1'] else "XX"

        print(f"{marker} {i:7d} [{match}]  HL: [F:{h['frame']:3d} C:{h['cycle']:8d}] "
              f"PC={h['pc']:03X} OP={h['op']:02X} A={h['a']:02X} P1={h['p1']:02X} P2={h['p2']:02X} RB{h['rb']} F1={h['f1']}")
        print(f"    {' ':7s}        SDL: [F:{s['frame']:3d} C:{s['cycle']:8d}] "
              f"PC={s['pc']:03X} OP={s['op']:02X} A={s['a']:02X} P1={s['p1']:02X} P2={s['p2']:02X} RB{s['rb']} F1={s['f1']}")


def find_all_divergences(hl, sdl):
    """Find all divergence regions (start, end, reconverged?)."""
    min_len = min(len(hl), len(sdl))
    fields = ['pc', 'op', 'a', 'psw', 'p1', 'p2', 'rb', 'f1']
    divergences = []
    in_div = False
    div_start = 0

    for i in range(min_len):
        matches = all(hl[i][f] == sdl[i][f] for f in fields)
        if not matches and not in_div:
            in_div = True
            div_start = i
        elif matches and in_div:
            divergences.append((div_start, i, i - div_start, True))
            in_div = False

    if in_div:
        divergences.append((div_start, min_len, min_len - div_start, False))

    return divergences


def main():
    parser = argparse.ArgumentParser(description='Compare headless vs SDL CPU traces')
    parser.add_argument('hl_trace', help='Headless trace file')
    parser.add_argument('sdl_trace', help='SDL trace file')
    parser.add_argument('--all', action='store_true', help='Show all divergences summary')
    parser.add_argument('--max', type=int, default=None, help='Max lines to load')
    parser.add_argument('--context', type=int, default=5, help='Context lines before divergence')

    args = parser.parse_args()

    print(f"Loading headless trace: {args.hl_trace}")
    hl = load_trace(args.hl_trace, args.max)
    print(f"  Loaded {len(hl):,} instructions")

    print(f"Loading SDL trace: {args.sdl_trace}")
    sdl = load_trace(args.sdl_trace, args.max)
    print(f"  Loaded {len(sdl):,} instructions")

    idx, diffs = find_first_divergence(hl, sdl)
    if idx is None:
        print(f"\n✓ Traces match perfectly for {min(len(hl), len(sdl)):,} instructions!")
        return

    print(f"\n{'='*70}")
    print(f"FIRST DIVERGENCE AT INSTRUCTION {idx:,}")
    print(f"  Frame: {hl[idx]['frame']} (HL) / {sdl[idx]['frame']} (SDL)")
    print(f"  Cycle: {hl[idx]['cycle']} (HL) / {sdl[idx]['cycle']} (SDL)")
    print(f"  Differences: {', '.join(diffs)}")
    print(f"{'='*70}")
    print_context(hl, sdl, idx, before=args.context, after=10)

    if args.all:
        divergences = find_all_divergences(hl, sdl)
        print(f"\n{'='*70}")
        print(f"DIVERGENCE SUMMARY")
        print(f"{'='*70}")
        print(f"Total: {len(divergences)}")
        reconverged = sum(1 for d in divergences if d[3])
        permanent = len(divergences) - reconverged
        print(f"Reconverged: {reconverged}")
        print(f"Permanent: {permanent}")

        for i, (start, end, length, reconv) in enumerate(divergences[:30]):
            status = "✓" if reconv else "✗ PERMANENT"
            hl_frame = hl[start]['frame'] if start < len(hl) else '?'
            print(f"  #{i+1}: instr {start:,}-{end:,} ({length:,} long) frame ~{hl_frame} {status}")

        if permanent:
            print(f"\n⚠ PERMANENT DIVERGENCE - this is a real bug!")
        else:
            print(f"\n✓ All divergences reconverge (timing only)")


if __name__ == '__main__':
    main()
