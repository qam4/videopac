#!/bin/bash
# Convert PPM screenshots to PNG for easier viewing

cd screenshots

# requires pip install Pillow
python -c "import os; from PIL import Image; [Image.open(f).save(f.replace('.ppm', '.png')) for f in os.listdir('.') if f.endswith('.ppm')]"

echo "Conversion complete!"
ls -lh *.png
