#!/bin/bash

# Create output directory
mkdir -p test-runs

# Input file
INPUT="obrazek1.bmp"

# Arrays of possible values for each parameter
TYPES=("rgb555" "rgb888" "ycbcr")
FILTERS=("none" "average")
COMPRESSIONS=("none" "dct" "dct_chroma")

# Compile the program first
clang++ -o SM2024-Projekt.out SM2024-Projekt.cpp SM2024-Funkcje.cpp SM2024-MedianCut.cpp \
        SM2024-Paleta.cpp SM2024-Pliki.cpp SM2024-Zmienne.cpp SM2024-Gui.cpp \
        SM2024-Konwersje.cpp SM2024-Kompresja.cpp -O2 -std=c++17 $(pkg-config --cflags --libs sdl2)

# Function to convert spaces and special characters to underscores
sanitize() {
    echo "$1" | tr ' ' '_' | tr '/' '_'
}

# Clear output directory
rm -rf test-runs/*

# Test all combinations
for type in "${TYPES[@]}"; do
    for filter in "${FILTERS[@]}"; do
        for compression in "${COMPRESSIONS[@]}"; do
            # Create descriptive names
            nf_name="test-runs/$(sanitize "${type}")_${filter}_${compression}.nf"
            bmp_name="test-runs/$(sanitize "${type}")_${filter}_${compression}.bmp"
            png_name="test-runs/$(sanitize "${type}")_${filter}_${compression}.png"
            
            echo "Testing combination: type=$type, filter=$filter, compression=$compression"
            
            # Convert BMP to NF
            ./SM2024-Projekt.out convert -i "$INPUT" -o "$nf_name" \
                                       -t "$type" -f "$filter" -c "$compression"
            
            # Convert NF back to BMP
            ./SM2024-Projekt.out convert -i "$nf_name" -o "$bmp_name"
            
            # Convert BMP to PNG for easy preview
            sips -s format png "$bmp_name" --out "$png_name" >/dev/null 2>&1
            
            # Display file info
            echo "Created: $nf_name"
            echo "Created: $bmp_name"
            echo "Created: $png_name"
            echo "----------------------------------------"
        done
    done
done

# Print summary
echo "All conversions completed. Results are in the test-runs directory."
echo -e "\nFile sizes:"
ls -lh test-runs/

# Create an HTML preview page
cat > test-runs/preview.html << EOF
<!DOCTYPE html>
<html>
<head>
    <title>Image Conversion Results</title>
    <style>
        body { font-family: Arial, sans-serif; margin: 20px; }
        .image-container { margin-bottom: 30px; }
        img { max-width: 100%; height: auto; }
        h2 { color: #333; }
        .details { color: #666; margin-bottom: 10px; }
    </style>
</head>
<body>
    <h1>Image Conversion Results</h1>
EOF

# Add each image to the HTML file
for type in "${TYPES[@]}"; do
    for filter in "${FILTERS[@]}"; do
        for compression in "${COMPRESSIONS[@]}"; do
            png_name="$(sanitize "${type}")_${filter}_${compression}.png"
            cat >> test-runs/preview.html << EOF
    <div class="image-container">
        <h2>$type - $filter - $compression</h2>
        <div class="details">
            Original: $INPUT<br>
            Settings: Type=$type, Filter=$filter, Compression=$compression
        </div>
        <img src="$png_name" alt="$png_name">
    </div>
EOF
        done
    done
done

# Close the HTML file
echo "</body></html>" >> test-runs/preview.html

echo -e "\nCreated preview.html - open it in a browser to compare all images"
echo "You can open it with: open test-runs/preview.html" 