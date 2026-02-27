#!/usr/bin/env python3
"""Show the permanent divergence between HL and SDL traces."""
import sys
sys.path.insert(0, 'scripts')
from compare_hl_sdl import load_trace, find_all_divergences, print_context

hl = load_trace('trace_cpu_hl.log')
sdl = load_trace('trace_cpu_sdl.log')
divs = find_all_divergences(hl, sdl)

# Show last divergence (the permanent one)
start, end, length, reconv = divs[-1]
status = "reconverged" if reconv else "PERMANENT"
print(f"Last divergence: instr {start:,}-{end:,} ({length:,} long) [{status}]")
print(f"  HL frame: {hl[start]['frame']}, SDL frame: {sdl[start]['frame']}")
print(f"  HL cycle: {hl[start]['cycle']}, SDL cycle: {sdl[start]['cycle']}")
print()
print("Context around start of permanent divergence:")
print_context(hl, sdl, start, before=5, after=15)
