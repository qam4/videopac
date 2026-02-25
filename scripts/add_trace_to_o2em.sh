#!/bin/bash
# Add CPU and VDC trace logging to o2em

set -e  # Exit on error

cd ~/src/o2em

# Check if we're in the o2em directory
if [ ! -f "cpu.c" ] || [ ! -f "vmachine.c" ]; then
    echo "Error: cpu.c or vmachine.c not found. Please run from ~/src/o2em directory"
    exit 1
fi

echo "Getting fresh files from git..."
git checkout cpu.c vmachine.c

echo "Applying CPU trace patch..."

awk '
BEGIN { in_cpu_exec = 0; added_globals = 0; added_trace = 0 }

# Add globals after last include
/^#include "vpp.h"/ {
    print
    if (!added_globals) {
        print ""
        print "/* TRACE LOGGING */"
        print "static FILE *trace_fp = NULL;"
        print "static int trace_count = 0;"
        added_globals = 1
    }
    next
}

# Detect cpu_exec function
/^void cpu_exec\(/ {
    in_cpu_exec = 1
}

# Add trace code right after "op=ROM(pc++);" - this is where we have valid pc, p1, acc
/op=ROM\(pc\+\+\);/ && in_cpu_exec && !added_trace {
    print
    print ""
    print "\t\t/* TRACE: Log first 50000 instructions */"
    print "\t\tif (!trace_fp) {"
    print "\t\t\ttrace_fp = fopen(\"o2em_trace.log\", \"a\");"
    print "\t\t\tif (trace_fp) {"
    print "\t\t\t\tif (trace_count == 0) {"
    print "\t\t\t\t\tfprintf(trace_fp, \"# O2EM Trace: PC OP P1 ACC BANK\\n\");"
    print "\t\t\t\t}"
    print "\t\t\t\tfprintf(stderr, \"TRACE: Logging to o2em_trace.log (count=%d)\\n\", trace_count);"
    print "\t\t\t} else {"
    print "\t\t\t\tfprintf(stderr, \"TRACE ERROR: Could not open o2em_trace.log\\n\");"
    print "\t\t\t}"
    print "\t\t}"
    print "\t\tif (trace_fp && trace_count < 3000000) {"
    print "\t\t\tint bank = (app_data.bank == 3) ? ((~p1) & 0x03) : 0;"
    print "\t\t\t/* Log PC before increment, opcode, current P1, ACC, and bank */"
    print "\t\t\tfprintf(trace_fp, \"%03X %02X %02X %02X %d\\n\", lastpc, op, p1, acc, bank);"
    print "\t\t\ttrace_count++;"
    print "\t\t\tif (trace_count == 3000000) {"
    print "\t\t\t\tfprintf(stderr, \"TRACE: Completed 3000000 instructions\\n\");"
    print "\t\t\t\tfflush(trace_fp);"
    print "\t\t\t\tfclose(trace_fp);"
    print "\t\t\t\ttrace_fp = NULL;"
    print "\t\t\t}"
    print "\t\t}"
    added_trace = 1
    next
}

{ print }
' cpu.c > cpu.c.traced

mv cpu.c.traced cpu.c

echo "CPU trace code added."
echo ""
echo "Applying VDC trace patch..."

awk '
BEGIN { in_ext_write = 0; added_globals = 0; added_trace = 0; in_main_loop = 0 }

# Add globals after includes
/^#include "config.h"/ {
    print
    if (!added_globals) {
        print ""
        print "/* VDC TRACE: Frame counter and trace file */"
        print "unsigned long long vdc_frame_count = 0;"
        print "static FILE *vdc_trace_fp = NULL;"
        print "static int vdc_trace_count = 0;"
        added_globals = 1
    }
    next
}

# Detect ext_write function
/^void ext_write\(/ {
    in_ext_write = 1
}

# Add trace code at the start of ext_write function, after the opening brace
in_ext_write && /^{/ && !added_trace {
    print
    print "\t/* VDC TRACE: Log first 10000 VDC writes */"
    print "\tif (!vdc_trace_fp) {"
    print "\t\tvdc_trace_fp = fopen(\"o2em_vdc_trace.log\", \"a\");"
    print "\t\tif (vdc_trace_fp) {"
    print "\t\t\tif (vdc_trace_count == 0) {"
    print "\t\t\t\tfprintf(vdc_trace_fp, \"# O2EM VDC Trace: FRAME ADDR VALUE\\n\");"
    print "\t\t\t}"
    print "\t\t} else {"
    print "\t\t\tfprintf(stderr, \"VDC TRACE ERROR: Could not open o2em_vdc_trace.log\\n\");"
    print "\t\t}"
    print "\t}"
    print "\tif (vdc_trace_fp && vdc_trace_count < 10000) {"
    print "\t\t/* Log frame, address, and value */"
    print "\t\tfprintf(vdc_trace_fp, \"%llu 0x%02X 0x%02X\\n\", vdc_frame_count, adr, dat);"
    print "\t\tvdc_trace_count++;"
    print "\t\tif (vdc_trace_count == 10000) {"
    print "\t\t\tfprintf(stderr, \"VDC TRACE: Completed 10000 writes\\n\");"
    print "\t\t\tfflush(vdc_trace_fp);"
    print "\t\t\tfclose(vdc_trace_fp);"
    print "\t\t\tvdc_trace_fp = NULL;"
    print "\t\t}"
    print "\t}"
    added_trace = 1
    next
}

# Track frame count - look for EVBLCLK (end of vertical blank)
/master_count.*==.*EVBLCLK/ {
    print
    print "\t\t\tvdc_frame_count++; /* Track frames for VDC trace */"
    next
}

{ print }
' vmachine.c > vmachine.c.traced

mv vmachine.c.traced vmachine.c

echo "VDC trace code added."
make clean
make

echo ""
echo "Rebuilding o2em..."
make clean
make

echo ""
echo ""
echo "=== CPU and VDC trace logging added successfully! ==="
echo ""
echo "Now create a wrapper script to run o2em with French BIOS:"
echo ""
cat > run_o2em_trace.sh << 'EOF'
#!/bin/bash
# Wrapper script to run O2EM with trace logging

# Set library path for Allegro
export LD_LIBRARY_PATH="/usr/local/lib:$LD_LIBRARY_PATH"

# Set display for remote desktop
export DISPLAY=":0"
export SDL_RENDER_DRIVER="software"
export SDL_AUDIODRIVER="dummy"

# Run O2EM with French BIOS
./o2em -bios="$HOME/src/roms/BIOS/bios_c52.bin" -romdir="$HOME/src/roms/" "$@"
EOF

chmod +x run_o2em_trace.sh

echo "Created: run_o2em_trace.sh"
echo ""
echo "To generate traces, run:"
echo "  ./run_o2em_trace.sh 'Killer Bees (1983)(Philips)(US).bin'"
echo ""
echo "This will create:"
echo "  - o2em_trace.log (first 50000 CPU instructions)"
echo "  - o2em_vdc_trace.log (first 10000 VDC writes)"
echo ""
echo "Press any key to start the game, then close the window after a few seconds."

