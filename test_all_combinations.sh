#!/bin/bash

set -e

# Check if parallel is installed
if ! command -v parallel &> /dev/null; then
    echo "GNU parallel is not installed. Please install it first."
    echo "On macOS: brew install parallel"
    echo "On Ubuntu/Debian: apt-get install parallel"
    exit 1
fi

# Create output directory
mkdir -p test-runs

# Input file
INPUT="obrazek6.bmp"

# Arrays of possible values for each parameter
TYPES=("rgb555" "rgb888" "ycbcr")
FILTERS=("none" "average")
COMPRESSIONS=("none" "dct" "dct_chroma" "rle")

# Function to get file size in kB
get_size_kb() {
    local size_bytes=$(stat -f%z "$1")
    echo $(( size_bytes / 1024 ))
}

# Function to calculate percentage of original size
get_percentage() {
    echo "scale=1; 100 * $1 / $2" | bc
}

# remove the output file if it exists
rm -f SM2024-Projekt.out

# Compile the program first
clang++ -o SM2024-Projekt.out SM2024-Projekt.cpp SM2024-Pliki.cpp SM2024-Zmienne.cpp -O2 -std=c++17 $(pkg-config --cflags --libs sdl2)

# Function to convert spaces and special characters to underscores
sanitize() {
    echo "$1" | tr ' ' '_' | tr '/' '_'
}

# Clear output directory
rm -rf test-runs/*

# Get original file size
original_size=$(get_size_kb "$INPUT")
echo "Original file size: ${original_size}kB"
echo "----------------------------------------"

# Function to process one combination
process_combination() {
    local type=$1
    local filter=$2
    local compression=$3
    
    # Skip invalid combinations
    if [ "$compression" = "dct_chroma" ] && [ "$type" != "ycbcr" ]; then
        echo "Skipping invalid combination: DCT+Chroma can only be used with YCbCr format"
        echo "----------------------------------------"
        return
    fi
    
    # Create descriptive names
    local nf_name="test-runs/$(sanitize "${type}")_${filter}_${compression}.nf"
    local bmp_name="test-runs/$(sanitize "${type}")_${filter}_${compression}.bmp"
    local png_name="test-runs/$(sanitize "${type}")_${filter}_${compression}.png"
    
    echo "Testing combination: type=$type, filter=$filter, compression=$compression"
    
    # Convert BMP to NF
    ./SM2024-Projekt.out convert -i "$INPUT" -o "$nf_name" \
                               -t "$type" -f "$filter" -c "$compression"
    
    # Convert NF back to BMP
    ./SM2024-Projekt.out convert -i "$nf_name" -o "$bmp_name"
    
    # Convert to PNG using imagemagick
    convert "$bmp_name" "$png_name"
    
    # Get file sizes
    local nf_size=$(get_size_kb "$nf_name")
    local bmp_size=$(get_size_kb "$bmp_name")
    local png_size=$(get_size_kb "$png_name")
    
    # Calculate percentages
    local nf_percent=$(get_percentage $nf_size $original_size)
    
    # Display file info with sizes
    echo "Created files:"
    echo "  NF:  $nf_name (${nf_size}kB, ${nf_percent}% of original)"
    echo "  BMP: $bmp_name (${bmp_size}kB)"
    echo "  PNG: $png_name (${png_size}kB)"
    echo "----------------------------------------"
}
export -f process_combination
export -f sanitize
export -f get_size_kb
export -f get_percentage
export INPUT
export original_size

# Generate all combinations and process them in parallel
parallel --bar process_combination {1} {2} {3} ::: "${TYPES[@]}" ::: "${FILTERS[@]}" ::: "${COMPRESSIONS[@]}"

# Print summary
echo "All conversions completed. Results are in the test-runs directory."
echo -e "\nDetailed file sizes:"
printf "%-50s %10s %15s\n" "FILENAME" "SIZE(kB)" "% OF ORIG"
echo "--------------------------------------------------------------------------------"
for f in test-runs/*.{nf,bmp,png}; do
    if [ -f "$f" ]; then
        size=$(get_size_kb "$f")
        percent=$(get_percentage $size $original_size)
        printf "%-50s %10d %14.1f%%\n" "$(basename "$f")" "$size" "$percent"
    fi
done

# Sort results by compression ratio for the summary
echo -e "\nCompression Results (sorted by effectiveness):"
printf "%-30s %-10s %-10s %15s\n" "SETTINGS" "SIZE(kB)" "SAVED(kB)" "% OF ORIG"
echo "--------------------------------------------------------------------------------"
for f in test-runs/*.nf; do
    if [ -f "$f" ]; then
        # Extract settings from filename
        filename=$(basename "$f" .nf)
        size=$(get_size_kb "$f")
        saved=$((original_size - size))
        percent=$(get_percentage $size $original_size)
        printf "%-30s %10d %10d %14.1f%%\n" "$filename" "$size" "$saved" "$percent"
    fi
done | sort -k4n

# Create an HTML preview page with file sizes
cat > test-runs/preview.html << EOF
<!DOCTYPE html>
<html>
<head>
    <title>Image Conversion Results</title>
    <style>
        body { font-family: Arial, sans-serif; margin: 20px; }
        .image-container { margin-bottom: 30px; border: 1px solid #ddd; padding: 15px; border-radius: 5px; }
        img { 
            max-width: 100%; 
            height: auto; 
            image-rendering: pixelated;
            image-rendering: -moz-crisp-edges;
            image-rendering: crisp-edges;
        }
        h2 { color: #333; margin-top: 0; }
        .details { color: #666; margin-bottom: 10px; }
        .file-info { font-family: monospace; background: #f5f5f5; padding: 10px; border-radius: 3px; }
        .size-good { color: #2a9d8f; }
        .size-medium { color: #e9c46a; }
        .size-bad { color: #e76f51; }
        .grid { display: grid; grid-template-columns: repeat(auto-fit, minmax(300px, 1fr)); gap: 20px; }
    </style>
</head>
<body>
    <h1>Image Conversion Results</h1>
    <p>Original file: $INPUT (${original_size}kB)</p>
    <div class="grid">
EOF

# Add each image to the HTML file
for type in "${TYPES[@]}"; do
    for filter in "${FILTERS[@]}"; do
        for compression in "${COMPRESSIONS[@]}"; do
            # Skip invalid combinations
            if [ "$compression" = "dct_chroma" ] && [ "$type" != "ycbcr" ]; then
                continue
            fi
            
            nf_name="test-runs/$(sanitize "${type}")_${filter}_${compression}.nf"
            png_name="$(sanitize "${type}")_${filter}_${compression}.png"
            
            nf_size=$(get_size_kb "$nf_name")
            nf_percent=$(get_percentage $nf_size $original_size)
            saved_kb=$((original_size - nf_size))
            
            # Determine color class based on compression percentage
            if (( $(echo "$nf_percent < 40" | bc -l) )); then
                size_class="size-good"
            elif (( $(echo "$nf_percent < 70" | bc -l) )); then
                size_class="size-medium"
            else
                size_class="size-bad"
            fi
            
            cat >> test-runs/preview.html << EOF
    <div class="image-container">
        <h2>$type - $filter - $compression</h2>
        <div class="details">
            Settings: Type=$type, Filter=$filter, Compression=$compression
        </div>
        <div class="file-info">
            NF size: <span class="${size_class}">${nf_size}kB (${nf_percent}% of original)</span><br>
            Saved: ${saved_kb}kB
        </div>
        <img src="$png_name" alt="$png_name">
    </div>
EOF
        done
    done
done

echo "</div>" >> test-runs/preview.html

# Add comparison tables
cat >> test-runs/preview.html << EOF
    <h2>Compression Ratio Comparison Tables</h2>
    
    <h3>No Filter</h3>
    <table style="border-collapse: collapse; width: 100%; margin-bottom: 30px;">
        <tr style="background-color: #f5f5f5;">
            <th style="border: 1px solid #ddd; padding: 8px; text-align: left;">Type</th>
            <th style="border: 1px solid #ddd; padding: 8px; text-align: center;">No Compression</th>
            <th style="border: 1px solid #ddd; padding: 8px; text-align: center;">DCT</th>
            <th style="border: 1px solid #ddd; padding: 8px; text-align: center;">DCT+Chroma</th>
            <th style="border: 1px solid #ddd; padding: 8px; text-align: center;">RLE</th>
        </tr>
EOF

# Fill the "No Filter" table
for type in "${TYPES[@]}"; do
    echo "        <tr>" >> test-runs/preview.html
    echo "            <td style=\"border: 1px solid #ddd; padding: 8px;\">$type</td>" >> test-runs/preview.html
    for compression in "none" "dct" "dct_chroma" "rle"; do
        if [ "$compression" = "dct_chroma" ] && [ "$type" != "ycbcr" ]; then
            echo "            <td style=\"border: 1px solid #ddd; padding: 8px; text-align: center; color: #999;\">N/A</td>" >> test-runs/preview.html
            continue
        fi
        nf_name="test-runs/$(sanitize "${type}")_none_${compression}.nf"
        if [ -f "$nf_name" ]; then
            nf_size=$(get_size_kb "$nf_name")
            nf_percent=$(get_percentage $nf_size $original_size)
            # Color coding based on compression ratio
            if (( $(echo "$nf_percent < 40" | bc -l) )); then
                color="#2a9d8f"
            elif (( $(echo "$nf_percent < 70" | bc -l) )); then
                color="#e9c46a"
            else
                color="#e76f51"
            fi
            echo "            <td style=\"border: 1px solid #ddd; padding: 8px; text-align: center; color: ${color}\">${nf_percent}%</td>" >> test-runs/preview.html
        else
            echo "            <td style=\"border: 1px solid #ddd; padding: 8px; text-align: center;\">N/A</td>" >> test-runs/preview.html
        fi
    done
    echo "        </tr>" >> test-runs/preview.html
done

cat >> test-runs/preview.html << EOF
    </table>

    <h3>Average Filter</h3>
    <table style="border-collapse: collapse; width: 100%; margin-bottom: 30px;">
        <tr style="background-color: #f5f5f5;">
            <th style="border: 1px solid #ddd; padding: 8px; text-align: left;">Type</th>
            <th style="border: 1px solid #ddd; padding: 8px; text-align: center;">No Compression</th>
            <th style="border: 1px solid #ddd; padding: 8px; text-align: center;">DCT</th>
            <th style="border: 1px solid #ddd; padding: 8px; text-align: center;">DCT+Chroma</th>
            <th style="border: 1px solid #ddd; padding: 8px; text-align: center;">RLE</th>
        </tr>
EOF

# Fill the "Average Filter" table
for type in "${TYPES[@]}"; do
    echo "        <tr>" >> test-runs/preview.html
    echo "            <td style=\"border: 1px solid #ddd; padding: 8px;\">$type</td>" >> test-runs/preview.html
    for compression in "none" "dct" "dct_chroma" "rle"; do
        if [ "$compression" = "dct_chroma" ] && [ "$type" != "ycbcr" ]; then
            echo "            <td style=\"border: 1px solid #ddd; padding: 8px; text-align: center; color: #999;\">N/A</td>" >> test-runs/preview.html
            continue
        fi
        nf_name="test-runs/$(sanitize "${type}")_average_${compression}.nf"
        if [ -f "$nf_name" ]; then
            nf_size=$(get_size_kb "$nf_name")
            nf_percent=$(get_percentage $nf_size $original_size)
            # Color coding based on compression ratio
            if (( $(echo "$nf_percent < 40" | bc -l) )); then
                color="#2a9d8f"
            elif (( $(echo "$nf_percent < 70" | bc -l) )); then
                color="#e9c46a"
            else
                color="#e76f51"
            fi
            echo "            <td style=\"border: 1px solid #ddd; padding: 8px; text-align: center; color: ${color}\">${nf_percent}%</td>" >> test-runs/preview.html
        else
            echo "            <td style=\"border: 1px solid #ddd; padding: 8px; text-align: center;\">N/A</td>" >> test-runs/preview.html
        fi
    done
    echo "        </tr>" >> test-runs/preview.html
done

# Close the HTML file
echo "</table></body></html>" >> test-runs/preview.html

echo -e "\nCreated preview.html - open it in a browser to compare all images"
echo "You can open it with: open test-runs/preview.html" 