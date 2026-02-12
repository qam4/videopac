rm -rf screenshots/*
rm -f trace.log

./build/videopac --bios "roms/Philips C52 BIOS (19xx)(Philips)(FR).bin" "$@" --headless --debug --screenshot 1 --frames 10 --press-key 1 5
./convert_screenshots.sh