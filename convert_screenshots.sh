#!/bin/bash
# Convert PPM screenshots to PNG for easier viewing

cd screenshots

for ppm in *.ppm; do
    if [ -f "$ppm" ]; then
        png="${ppm%.ppm}.png"
        echo "Converting $ppm to $png..."
        convert "$ppm" "$png"
    fi
done

echo "Conversion complete!"
ls -lh *.png
