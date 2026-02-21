#!/usr/bin/env python3
"""
Videopac Emulator Launcher
Unified cross-platform script for running the emulator in different modes.
"""

import argparse
import os
import platform
import shutil
import subprocess
import sys
from pathlib import Path


def get_executable_path():
    """Determine the emulator executable path based on platform."""
    system = platform.system()
    
    if system == "Windows":
        # Try dev-mingw first (current preset), then build root, then ci-win64
        paths = [
            Path("build/dev-mingw/videopac.exe"),
            Path("build/videopac.exe"),
            Path("build/ci-win64/Release/videopac.exe"),
        ]
    else:
        # Linux/macOS
        paths = [
            Path("build/videopac"),
            Path("build/Release/videopac"),
        ]
    
    for path in paths:
        if path.exists():
            return str(path)
    
    print(f"Error: Could not find emulator executable. Tried:", file=sys.stderr)
    for path in paths:
        print(f"  - {path}", file=sys.stderr)
    sys.exit(1)


def clean_trace_log():
    """Remove old trace log file."""
    if Path("trace.log").exists():
        try:
            Path("trace.log").unlink()
        except PermissionError:
            # File is locked by another process, skip deletion
            print("Warning: trace.log is locked by another process, skipping cleanup", file=sys.stderr)


def setup_screenshots_dir():
    """Clean and create screenshots directory."""
    screenshots_dir = Path("screenshots")
    if screenshots_dir.exists():
        try:
            shutil.rmtree(screenshots_dir)
        except PermissionError:
            # Files are locked, just skip cleanup
            print("Warning: screenshots directory is locked, skipping cleanup", file=sys.stderr)
            return
    screenshots_dir.mkdir(exist_ok=True)


def convert_screenshots():
    """Convert screenshots using the conversion script."""
    convert_script = Path("convert_screenshots.sh")
    if convert_script.exists():
        print("Converting screenshots...")
        try:
            subprocess.run(["bash", str(convert_script)], check=True)
        except subprocess.CalledProcessError as e:
            print(f"Warning: Screenshot conversion failed: {e}", file=sys.stderr)
        except FileNotFoundError:
            print("Warning: bash not found, skipping screenshot conversion", file=sys.stderr)


def run_sdl_mode(exe_path, bios_path, rom_path, extra_args):
    """Run emulator in SDL mode with debugger."""
    print("Running in SDL mode...")
    print()
    print("Note: For Course de Voitures ROM:")
    print("  - Press '1' for Game 1: Course de Voitures (road racing)")
    print("  - Press '2' for Game 2: Autodrome (top-down circuit racing)")
    print()
    print("IMPORTANT: Do NOT press any arrow keys after selecting the game!")
    print("We are testing if the road moves without input.")
    print()
    
    cmd = [
        exe_path,
        "--debug",
        "--trace",
        "--bios", bios_path,
        rom_path
    ]
    
    # Add any extra arguments (breakpoints, watch expressions, region, etc.)
    cmd.extend(extra_args)
    
    subprocess.run(cmd)


def run_headless_mode(exe_path, bios_path, rom_path, extra_args, no_input=False):
    """Run emulator in headless mode with screenshot capture."""
    print("Running in HEADLESS mode...")
    if no_input:
        print("Note: NO INPUT MODE - testing without joystick")
    else:
        print("Note: Pressing '1' at frame 5 to select game")
        print("Note: Pressing '1' at frame 12 to select level")
        print("Note: Pressing UP at frame 20 for 95 frames")
        print("Note: Will dump VDC state at frames 10, 14, 20, 22, 24, 26, 28, 30, 60, 100, 118")
    print()
    
    setup_screenshots_dir()
    
    if no_input:
        # No input mode - only basic setup, no key/joystick presses
        cmd = [
            exe_path,
            "--headless",
            "--screenshot", "1",
            "--frames", "200",
            "--debug",
            "--trace",
            "--bios", bios_path,
            rom_path
        ]
    else:
        # Normal mode with input - 120 frames with '1' pressed twice, UP at frame 20 for 95 frames
        cmd = [
            exe_path,
            "--headless",
            "--screenshot", "1",
            "--frames", "120",
            "--press-key", "1", "5", "5",   # Press '1' at frame 5 for 5 frames (title screen)
            "--press-key", "1", "12", "5",  # Press '1' at frame 12 for 5 frames (select game)
            "--press-joystick", "2", "0", "20", "95",  # Press joystick 2 UP at frame 20 for 95 frames
            "--debug",
            "--trace",
            "--bios", bios_path,
            rom_path
        ]
    
    # Add any extra arguments (breakpoints, watch expressions, region, etc.)
    cmd.extend(extra_args)
    
    print("Running emulator (output will appear below)...")
    print("Command:", " ".join(cmd))
    print("=" * 60)
    result = subprocess.run(cmd)
    print("=" * 60)
    print(f"Emulator exited with code: {result.returncode}")
    print()
    
    convert_screenshots()


def run_dcv_mode(exe_path, bios_path, rom_path, extra_args):
    """Run emulator in DCV mode (remote desktop optimized)."""
    print("Running in DCV mode (remote desktop)...")
    
    # Set environment variables for remote desktop
    env = os.environ.copy()
    env["DISPLAY"] = ":0"
    env["SDL_RENDER_DRIVER"] = "software"
    env["SDL_AUDIODRIVER"] = "dummy"  # Disable audio to avoid errors
    
    cmd = [
        exe_path,
        "--debug",
        "--bios", bios_path,
        rom_path
    ]
    
    # Add any extra arguments (breakpoints, watch expressions, region, etc.)
    cmd.extend(extra_args)
    
    subprocess.run(cmd, env=env)
    cmd.extend(extra_args)
    
    subprocess.run(cmd, env=env)


def main():
    parser = argparse.ArgumentParser(
        description="Videopac Emulator Launcher",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  %(prog)s                                    # SDL mode with default ROM
  %(prog)s sdl                                # SDL mode with default ROM
  %(prog)s headless                           # Headless mode with default ROM
  %(prog)s dcv                                # DCV mode with default ROM
  %(prog)s sdl "roms/game.bin"                # SDL mode with custom ROM
  %(prog)s headless "roms/game.bin"           # Headless mode with custom ROM
  
  # With debugger options:
  %(prog)s sdl --watch "RAM[0x30]!=0"         # Watch expression
  %(prog)s sdl --break 0x495                  # Breakpoint at address
  %(prog)s sdl --break 0x495 --condition "A==0xFF"  # Conditional breakpoint
        """
    )
    
    parser.add_argument(
        "mode",
        nargs="?",
        default="sdl",
        choices=["sdl", "headless", "hl", "dcv"],
        help="Emulator mode: sdl (default), headless/hl, or dcv"
    )
    
    parser.add_argument(
        "rom",
        nargs="?",
        default="roms/Course de Voitures + Autodrome + Cryptogramme (1980)(Philips)(FR).bin",
        # default="roms/Satellite Attack (1981)(Philips)(EU).bin",
        help="Path to ROM file (default: Course de Voitures / Autodrome)"
    )
    
    parser.add_argument(
        "--bios",
        default="roms/Philips C52 BIOS (19xx)(Philips)(FR).bin",
        help="Path to BIOS file (default: French C52 BIOS)"
    )
    
    parser.add_argument(
        "--region",
        default=None,
        choices=["usa", "europe", "france"],
        help="Region setting (default: usa if not specified)"
    )
    
    parser.add_argument(
        "--no-input",
        action="store_true",
        help="Headless mode: run without any input (for baseline testing)"
    )
    
    # Debugger options
    parser.add_argument(
        "--watch",
        action="append",
        dest="watch_expressions",
        help="Watch expression (e.g., 'RAM[0x30]!=0')"
    )
    
    parser.add_argument(
        "--break",
        action="append",
        dest="breakpoints",
        help="Breakpoint address (e.g., 0x495)"
    )
    
    parser.add_argument(
        "--condition",
        action="append",
        dest="conditions",
        help="Condition for previous breakpoint (e.g., 'A==0xFF')"
    )
    
    args = parser.parse_args()
    
    # Get executable path
    exe_path = get_executable_path()
    
    # Clean up old files
    clean_trace_log()
    
    # Normalize mode
    mode = args.mode.lower()
    if mode == "hl":
        mode = "headless"
    
    # Build extra arguments for debugger
    extra_args = []
    
    # Add region if specified (otherwise use emulator default)
    if args.region:
        extra_args.extend(["--region", args.region])
    
    # Add watch expressions
    if args.watch_expressions:
        for watch in args.watch_expressions:
            extra_args.extend(["--watch", watch])
    
    # Add breakpoints with optional conditions
    if args.breakpoints:
        for i, bp in enumerate(args.breakpoints):
            extra_args.extend(["--break", bp])
            # If there's a corresponding condition, add it
            if args.conditions and i < len(args.conditions):
                extra_args.extend(["--condition", args.conditions[i]])
    
    # Run appropriate mode
    if mode == "headless":
        run_headless_mode(exe_path, args.bios, args.rom, extra_args, no_input=args.no_input)
    elif mode == "dcv":
        run_dcv_mode(exe_path, args.bios, args.rom, extra_args)
    else:  # sdl
        run_sdl_mode(exe_path, args.bios, args.rom, extra_args)


if __name__ == "__main__":
    main()
