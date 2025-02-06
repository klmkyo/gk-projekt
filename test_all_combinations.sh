#!/bin/bash

set -e

if ! command -v parallel &> /dev/null; then
    echo "GNU parallel is not installed. Please install it first."
    echo "On macOS: brew install parallel"
    echo "On Ubuntu/Debian: apt-get install parallel"
    exit 1
fi

mkdir -p test-runs

IMAGES=("obrazek1.bmp" "obrazek2.bmp" "obrazek3.bmp" "obrazek4.bmp" "obrazek5.bmp" "obrazek6.bmp" "obrazek7.bmp" "obrazek8.bmp" "obrazek9.bmp")

TYPES=("rgb555" "rgb888" "ycbcr")
FILTERS=("none" "average")
COMPRESSIONS=("none" "dct" "dct_chroma" "rle")

get_size_kb() {
    local size_bytes=$(stat -f%z "$1")
    echo $(( size_bytes / 1024 ))
}

get_percentage() {
    echo "scale=1; 100 * $1 / $2" | bc
}

rm -f SM2024-Projekt.out

clang++ -o SM2024-Projekt.out SM2024-Projekt.cpp SM2024-Pliki.cpp SM2024-Zmienne.cpp -O2 -std=c++17 $(pkg-config --cflags --libs sdl2)

sanitize() {
    echo "$1" | tr ' ' '_' | tr '/' '_'
}

rm -rf test-runs/*

process_combination() {
    local input_image=$1
    local type=$2
    local filter=$3
    local compression=$4
    
    local image_name=$(basename "$input_image" .bmp)
    
    mkdir -p "test-runs/${image_name}"
    
    if [ "$compression" = "dct_chroma" ] && [ "$type" != "ycbcr" ]; then
        echo "Skipping invalid combination for ${image_name}: DCT+Chroma can only be used with YCbCr format"
        echo "----------------------------------------"
        return
    fi
    
    local nf_name="test-runs/${image_name}/$(sanitize "${type}")_${filter}_${compression}.nf"
    local bmp_name="test-runs/${image_name}/$(sanitize "${type}")_${filter}_${compression}.bmp"
    local png_name="test-runs/${image_name}/$(sanitize "${type}")_${filter}_${compression}.png"
    
    echo "Testing combination for ${image_name}: type=$type, filter=$filter, compression=$compression"
    
    ./SM2024-Projekt.out convert -i "$input_image" -o "$nf_name" \
                               -t "$type" -f "$filter" -c "$compression"
    
    ./SM2024-Projekt.out convert -i "$nf_name" -o "$bmp_name"
    
    convert "$bmp_name" "$png_name"
    
    local original_size=$(get_size_kb "$input_image")
    local nf_size=$(get_size_kb "$nf_name")
    local bmp_size=$(get_size_kb "$bmp_name")
    local png_size=$(get_size_kb "$png_name")
    
    local nf_percent=$(get_percentage $nf_size $original_size)
    
    echo "Created files for ${image_name}:"
    echo "  NF:  $nf_name (${nf_size}kB, ${nf_percent}% of original)"
    echo "  BMP: $bmp_name (${bmp_size}kB)"
    echo "  PNG: $png_name (${png_size}kB)"
    echo "----------------------------------------"
}
export -f process_combination
export -f sanitize
export -f get_size_kb
export -f get_percentage

for image in "${IMAGES[@]}"; do
    if [ -f "$image" ]; then
        echo "Processing $image..."
        parallel --bar process_combination "$image" {1} {2} {3} ::: "${TYPES[@]}" ::: "${FILTERS[@]}" ::: "${COMPRESSIONS[@]}"
    else
        echo "Warning: $image not found, skipping..."
    fi
done

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
        .tab-buttons { margin-bottom: 20px; }
        .tab-button {
            padding: 10px 20px;
            border: none;
            background: #f0f0f0;
            cursor: pointer;
            border-radius: 5px;
            margin-right: 5px;
        }
        .tab-button.active {
            background: #2a9d8f;
            color: white;
        }
        .tab-content {
            display: none;
        }
        .tab-content.active {
            display: block;
        }
    </style>
    <script>
        function showTab(imageId) {
            // Hide all tabs
            document.querySelectorAll('.tab-content').forEach(tab => {
                tab.classList.remove('active');
            });
            document.querySelectorAll('.tab-button').forEach(button => {
                button.classList.remove('active');
            });
            
            // Show selected tab
            document.getElementById('tab-' + imageId).classList.add('active');
            document.getElementById('button-' + imageId).classList.add('active');
        }
    </script>
</head>
<body>
    <h1>Image Conversion Results</h1>
    
    <div class="tab-buttons">
EOF

for image in "${IMAGES[@]}"; do
    if [ -f "$image" ]; then
        image_name=$(basename "$image" .bmp)
        cat >> test-runs/preview.html << EOF
        <button id="button-${image_name}" class="tab-button" onclick="showTab('${image_name}')">${image_name}</button>
EOF
    fi
done

echo '</div><div class="tabs-content">' >> test-runs/preview.html

for image in "${IMAGES[@]}"; do
    if [ -f "$image" ]; then
        image_name=$(basename "$image" .bmp)
        original_size=$(get_size_kb "$image")
        
        cat >> test-runs/preview.html << EOF
        <div id="tab-${image_name}" class="tab-content">
            <h2>${image_name}</h2>
            <p>Original file: ${image} (${original_size}kB)</p>
            
            <h3>Compression Ratio Comparison Tables</h3>
            
            <h4>No Filter</h4>
            <table style="border-collapse: collapse; width: 100%; margin-bottom: 30px;">
                <tr style="background-color: #f5f5f5;">
                    <th style="border: 1px solid #ddd; padding: 8px; text-align: left;">Type</th>
                    <th style="border: 1px solid #ddd; padding: 8px; text-align: center;">No Compression</th>
                    <th style="border: 1px solid #ddd; padding: 8px; text-align: center;">DCT</th>
                    <th style="border: 1px solid #ddd; padding: 8px; text-align: center;">DCT+Chroma</th>
                    <th style="border: 1px solid #ddd; padding: 8px; text-align: center;">RLE</th>
                </tr>
EOF

        for type in "${TYPES[@]}"; do
            echo "        <tr>" >> test-runs/preview.html
            echo "            <td style=\"border: 1px solid #ddd; padding: 8px;\">$type</td>" >> test-runs/preview.html
            for compression in "none" "dct" "dct_chroma" "rle"; do
                if [ "$compression" = "dct_chroma" ] && [ "$type" != "ycbcr" ]; then
                    echo "            <td style=\"border: 1px solid #ddd; padding: 8px; text-align: center; color: #999;\">N/A</td>" >> test-runs/preview.html
                    continue
                fi
                nf_name="test-runs/${image_name}/$(sanitize "${type}")_none_${compression}.nf"
                if [ -f "$nf_name" ]; then
                    nf_size=$(get_size_kb "$nf_name")
                    nf_percent=$(get_percentage $nf_size $original_size)
                    if (( $(echo "$nf_percent < 100" | bc -l) )); then
                        color="#20BB5B"
                    elif (( $(echo "$nf_percent >= 99 && $nf_percent <= 101" | bc -l) )); then
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

            <h4>Average Filter</h4>
            <table style="border-collapse: collapse; width: 100%; margin-bottom: 30px;">
                <tr style="background-color: #f5f5f5;">
                    <th style="border: 1px solid #ddd; padding: 8px; text-align: left;">Type</th>
                    <th style="border: 1px solid #ddd; padding: 8px; text-align: center;">No Compression</th>
                    <th style="border: 1px solid #ddd; padding: 8px; text-align: center;">DCT</th>
                    <th style="border: 1px solid #ddd; padding: 8px; text-align: center;">DCT+Chroma</th>
                    <th style="border: 1px solid #ddd; padding: 8px; text-align: center;">RLE</th>
                </tr>
EOF

        for type in "${TYPES[@]}"; do
            echo "        <tr>" >> test-runs/preview.html
            echo "            <td style=\"border: 1px solid #ddd; padding: 8px;\">$type</td>" >> test-runs/preview.html
            for compression in "none" "dct" "dct_chroma" "rle"; do
                if [ "$compression" = "dct_chroma" ] && [ "$type" != "ycbcr" ]; then
                    echo "            <td style=\"border: 1px solid #ddd; padding: 8px; text-align: center; color: #999;\">N/A</td>" >> test-runs/preview.html
                    continue
                fi
                nf_name="test-runs/${image_name}/$(sanitize "${type}")_average_${compression}.nf"
                if [ -f "$nf_name" ]; then
                    nf_size=$(get_size_kb "$nf_name")
                    nf_percent=$(get_percentage $nf_size $original_size)
                    if (( $(echo "$nf_percent < 100" | bc -l) )); then
                        color="#20BB5B"
                    elif (( $(echo "$nf_percent >= 99 && $nf_percent <= 101" | bc -l) )); then
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

            <h3>Image Comparisons</h3>
            <div class="grid">
EOF

        for type in "${TYPES[@]}"; do
            for filter in "${FILTERS[@]}"; do
                for compression in "${COMPRESSIONS[@]}"; do
                    if [ "$compression" = "dct_chroma" ] && [ "$type" != "ycbcr" ]; then
                        continue
                    fi
                    
                    nf_name="test-runs/${image_name}/$(sanitize "${type}")_${filter}_${compression}.nf"
                    png_name="${image_name}/$(sanitize "${type}")_${filter}_${compression}.png"
                    
                    if [ -f "$nf_name" ]; then
                        nf_size=$(get_size_kb "$nf_name")
                        nf_percent=$(get_percentage $nf_size $original_size)
                        saved_kb=$((original_size - nf_size))
                        
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
                    fi
                done
            done
        done

        echo "</div></div>" >> test-runs/preview.html
    fi
done

cat >> test-runs/preview.html << EOF
    </div>
    <script>
        // Show first tab by default
        document.querySelector('.tab-button').click();
    </script>
</body>
</html>
EOF

echo -e "\nCreated preview.html - open it in a browser to compare all images"
echo "You can open it with: open test-runs/preview.html" 