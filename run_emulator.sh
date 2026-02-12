#!/bin/bash
# Videopac Emulator Launcher for DCV

export DISPLAY=:0
export SDL_RENDER_DRIVER=software
export SDL_AUDIODRIVER=dummy  # Disable audio to avoid ALSA errors

./build/videopac --bios "roms/Philips C52 BIOS (19xx)(Philips)(FR).bin" "roms/Satellite Attack (1981)(Philips)(EU).bin" "$@" --debug
