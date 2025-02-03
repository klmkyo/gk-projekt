#include "SM2024-Pliki.h"
#include "SM2024-Zmienne.h"
#include <SDL.h>
#include <algorithm>
#include <cmath> // <-- For std::cos, std::sqrt
#include <ctime>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <vector> // <-- Needed for vectors

extern const char FILE_SIGNATURE[2];

SDL_Color getPixelSurface(int x, int y, SDL_Surface *surface) {
  SDL_Color color;
  Uint32 col = 0;
  if ((x >= 0) && (x < surface->w) && (y >= 0) && (y < surface->h)) {
    char *pPosition = (char *)surface->pixels;
    pPosition += (surface->pitch * y);
    pPosition += (surface->format->BytesPerPixel * x);
    memcpy(&col, pPosition, surface->format->BytesPerPixel);
    SDL_GetRGB(col, surface->format, &color.r, &color.g, &color.b);
  }
  return color;
}

const char *NOFI_MAGIC = "NOFI";
const Uint8 NFVERSION = 1;

std::vector<Uint8> serializeHeader(NFHeaderUser header) {
  std::vector<Uint8> headerData;
  headerData.reserve(NFHEADER_SIZE);

  // Setting Magic
  for (int i = 0; i < 4; i++) {
    headerData.push_back(NOFI_MAGIC[i]);
  }

  headerData.push_back(NFVERSION);
  headerData.push_back((Uint8)header.type);
  headerData.push_back((Uint8)header.filter);
  headerData.push_back((Uint8)header.compression);

  headerData.push_back(header.width & 0xFF);
  headerData.push_back((header.width >> 8) & 0xFF);

  headerData.push_back(header.height & 0xFF);
  headerData.push_back((header.height >> 8) & 0xFF);

  // Pad to HEADER_SIZE
  while (headerData.size() < NFHEADER_SIZE) {
    headerData.push_back(0);
  }

  return headerData;
}

NFHeader deserializeHeader(std::vector<Uint8> data) {
  NFHeader header;
  for (int i = 0; i < 4; i++) {
    header.magic[i] = data[i];

    if (header.magic[i] != NOFI_MAGIC[i]) {
      throw std::invalid_argument("Invalid NOFI file");
    }
  }

  header.version = data[4];

  if (header.version != NFVERSION) {
    throw std::invalid_argument("Invalid NOFI version. Expected " +
                                std::to_string(NFVERSION) + " but got " +
                                std::to_string(header.version));
  }

  header.type = (ImageType)data[5];
  header.filter = (FilterType)data[6];
  header.compression = (CompressionType)data[7];

  header.width = data[8] | (data[9] << 8);
  header.height = data[10] | (data[11] << 8);

  return header;
}

void saveNFImage(const std::string &filename, NFHeaderUser header,
                 Canvas &image) {
  std::vector<Uint8> bytes;

  // add header
  std::vector<Uint8> headerBytes = serializeHeader(header);
  bytes.insert(bytes.end(), headerBytes.begin(), headerBytes.end());

  // add image data
  std::vector<Uint8> imageData = serializeCanvas(image, header);

  bytes.insert(bytes.end(), imageData.begin(), imageData.end());

  std::ofstream file(filename, std::ios::out | std::ios::binary);
  file.write((char *)bytes.data(), bytes.size());
  file.close();
}

std::pair<NFHeader, Canvas> loadNFImage(const std::string &filename) {
  std::ifstream file(filename, std::ios::in | std::ios::binary);
  std::vector<Uint8> bytes;

  // read the whole file into bytes
  file.seekg(0, std::ios::end);
  bytes.resize(file.tellg());
  file.seekg(0, std::ios::beg);

  file.read((char *)bytes.data(), bytes.size());
  file.close();

  int headerSize = NFHEADER_SIZE;
  std::cout << "Header size: " << headerSize << std::endl;
  std::vector<Uint8> headerBytes(bytes.begin(), bytes.begin() + headerSize);
  NFHeader header = deserializeHeader(headerBytes);

  // image data is the rest of the bytes
  std::vector<Uint8> imageData(bytes.begin() + headerSize, bytes.end());

  Canvas image = deserializeCanvas(imageData, header);
  return {header, image};
}

// -----------------------------------------------------------------------------
// Basic clamp
// -----------------------------------------------------------------------------
template <typename T>
T clamp(const T &value, const T &minVal, const T &maxVal) {
  if (value < minVal)
    return minVal;
  if (value > maxVal)
    return maxVal;
  return value;
}

// -----------------------------------------------------------------------------
// 8×8 Bayer dithering matrix
// -----------------------------------------------------------------------------
const int BAYER_MATRIX_SIZE = 8;
const float BAYER_MATRIX[8][8] = {
    {0, 48, 12, 60, 3, 51, 15, 63}, {32, 16, 44, 28, 35, 19, 47, 31},
    {8, 56, 4, 52, 11, 59, 7, 55},  {40, 24, 36, 20, 43, 27, 39, 23},
    {2, 50, 14, 62, 1, 49, 13, 61}, {34, 18, 46, 30, 33, 17, 45, 29},
    {10, 58, 6, 54, 9, 57, 5, 53},  {42, 26, 38, 22, 41, 25, 37, 21}};

Uint8 applyBayerDithering(int x, int y, Uint8 value) {
  float bayerValue =
      BAYER_MATRIX[y % BAYER_MATRIX_SIZE][x % BAYER_MATRIX_SIZE] / 64.0f;
  bayerValue = (bayerValue - 0.5f) * (8.0f / 255.0f); // for ~5-bit
  float dithered = (value / 255.0f) + bayerValue;
  dithered = std::max(0.0f, std::min(1.0f, dithered));
  return static_cast<Uint8>(dithered * 31.0f + 0.5f);
}

// -----------------------------------------------------------------------------
// NEW: Simple DCT + quantization tables
// -----------------------------------------------------------------------------

// DCT tables (quantization for Y & C). Very simple example:
static const float QY[8][8] = {
    {16, 11, 10, 16, 24, 40, 51, 61},     {12, 12, 14, 19, 26, 58, 60, 55},
    {14, 13, 16, 24, 40, 57, 69, 56},     {14, 17, 22, 29, 51, 87, 80, 62},
    {18, 22, 37, 56, 68, 109, 103, 77},   {24, 35, 55, 64, 81, 104, 113, 92},
    {49, 64, 78, 87, 103, 121, 120, 101}, {72, 92, 95, 98, 112, 100, 103, 99}};

static const float QC[8][8] = {
    {17, 18, 24, 47, 99, 99, 99, 99}, {18, 21, 26, 66, 99, 99, 99, 99},
    {24, 26, 56, 99, 99, 99, 99, 99}, {47, 66, 99, 99, 99, 99, 99, 99},
    {99, 99, 99, 99, 99, 99, 99, 99}, {99, 99, 99, 99, 99, 99, 99, 99},
    {99, 99, 99, 99, 99, 99, 99, 99}, {99, 99, 99, 99, 99, 99, 99, 99}};

// Forward & inverse DCT helpers
static inline float c(int idx) {
  // "alpha" factor
  return (idx == 0) ? (1.0f / std::sqrt(8.0f)) : std::sqrt(2.0f / 8.0f);
}

static void forwardDCT8x8(const float in[8][8], float out[8][8]) {
  for (int v = 0; v < 8; v++) {
    for (int u = 0; u < 8; u++) {
      float sum = 0.f;
      for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 8; x++) {
          sum += in[y][x] *
                 std::cos(((2.f * x + 1.f) * u * (float)M_PI) / 16.f) *
                 std::cos(((2.f * y + 1.f) * v * (float)M_PI) / 16.f);
        }
      }
      out[v][u] = c(u) * c(v) * sum;
    }
  }
}

static void inverseDCT8x8(const float in[8][8], float out[8][8]) {
  for (int y = 0; y < 8; y++) {
    for (int x = 0; x < 8; x++) {
      float sum = 0.f;
      for (int v = 0; v < 8; v++) {
        for (int u = 0; u < 8; u++) {
          sum += c(u) * c(v) * in[v][u] *
                 std::cos(((2.f * x + 1.f) * u * (float)M_PI) / 16.f) *
                 std::cos(((2.f * y + 1.f) * v * (float)M_PI) / 16.f);
        }
      }
      out[y][x] = sum;
    }
  }
}

// -----------------------------------------------------------------------------
// RLE compression functions (unchanged)
// -----------------------------------------------------------------------------
std::vector<Uint8> compressRLE(const std::vector<Uint8> &input) {
  std::vector<Uint8> compressed;
  size_t i = 0;

  while (i < input.size()) {
    Uint8 current = input[i];
    Uint8 count = 1;

    while (i + 1 < input.size() && input[i + 1] == current && count < 255) {
      count++;
      i++;
    }

    // If run length is 1 and next byte is different, store without RLE
    if (count == 1 && i + 1 < input.size() && input[i + 1] != current) {
      compressed.push_back(0); // 0 count means literal byte follows
      compressed.push_back(current);
    } else {
      compressed.push_back(count);
      compressed.push_back(current);
    }

    i++;
  }

  return compressed;
}

std::vector<Uint8> decompressRLE(const std::vector<Uint8> &input) {
  std::vector<Uint8> decompressed;
  size_t i = 0;

  while (i + 1 < input.size()) {
    Uint8 count = input[i++];
    Uint8 value = input[i++];

    if (count == 0) {
      // Literal byte
      decompressed.push_back(value);
    } else {
      // Run of identical bytes
      for (Uint8 j = 0; j < count; j++) {
        decompressed.push_back(value);
      }
    }
  }

  return decompressed;
}

// -----------------------------------------------------------------------------
// serializeCanvas (updated to handle DCT for YCbCr + subsampling)
// -----------------------------------------------------------------------------
std::vector<Uint8> serializeCanvas(Canvas &image, NFHeaderUser header) {
  NFHeader fullHeader;
  fullHeader.type = header.type;
  fullHeader.filter = header.filter;
  fullHeader.compression = header.compression;
  fullHeader.width = header.width;
  fullHeader.height = header.height;
  return serializeCanvas(image, fullHeader);
}

std::vector<Uint8> serializeCanvas(Canvas &image, NFHeader header) {
  std::vector<Uint8> data;
  int width = (int)image[0].size();
  int height = (int)image.size();

  // Validate that DctPlusChromaSubsampling is only used with YCbCr
  if (header.compression == CompressionType::DctPlusChromaSubsampling &&
      header.type != ImageType::YCbCr888) {
    throw std::invalid_argument(
        "DCT+Chroma subsampling can only be used with YCbCr format");
  }

  switch (header.type) {
  case ImageType::RGB888: {
    // Standard RGB888 - 3 bytes/pixel
    if (header.compression == CompressionType::Dct) {
      // Create separate planes for R, G, B
      std::vector<std::vector<float>> Rf(height, std::vector<float>(width));
      std::vector<std::vector<float>> Gf(height, std::vector<float>(width));
      std::vector<std::vector<float>> Bf(height, std::vector<float>(width));

      // Fill the planes
      for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
          Rf[y][x] = (float)image[y][x].r;
          Gf[y][x] = (float)image[y][x].g;
          Bf[y][x] = (float)image[y][x].b;
        }
      }

      auto encodePlaneDCT = [&](std::vector<std::vector<float>> &plane, int w,
                                int h) {
        for (int by = 0; by < h; by += 8) {
          for (int bx = 0; bx < w; bx += 8) {
            // Gather block -> subtract 128
            float block[8][8];
            for (int yb = 0; yb < 8; yb++) {
              for (int xb = 0; xb < 8; xb++) {
                int px = bx + xb;
                int py = by + yb;
                float val = 0.f;
                if (px < w && py < h) {
                  val = plane[py][px] - 128.f;
                }
                block[yb][xb] = val;
              }
            }
            // Forward DCT
            float dctOut[8][8];
            forwardDCT8x8(block, dctOut);
            // Quantize & store
            for (int yb = 0; yb < 8; yb++) {
              for (int xb = 0; xb < 8; xb++) {
                float q = QY[yb][xb]; // Use luminance quantization for RGB
                float coeff = std::round(dctOut[yb][xb] / q);
                int16_t iCoeff = (int16_t)coeff;
                // Store as 2 bytes (little-endian)
                data.push_back((Uint8)(iCoeff & 0xFF));
                data.push_back((Uint8)((iCoeff >> 8) & 0xFF));
              }
            }
          }
        }
      };

      // Encode each color plane
      encodePlaneDCT(Rf, width, height);
      encodePlaneDCT(Gf, width, height);
      encodePlaneDCT(Bf, width, height);
    } else {
      for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
          data.push_back(image[y][x].r);
          data.push_back(image[y][x].g);
          data.push_back(image[y][x].b);
        }
      }
    }
    break;
  }
  case ImageType::RGB555_WITH_BAYER_DITHERING: {
    if (header.compression == CompressionType::Dct) {
      // Create separate planes for R5, G5, B5
      std::vector<std::vector<float>> R5f(height, std::vector<float>(width));
      std::vector<std::vector<float>> G5f(height, std::vector<float>(width));
      std::vector<std::vector<float>> B5f(height, std::vector<float>(width));

      // Fill the planes with dithered 5-bit values
      for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
          R5f[y][x] = (float)applyBayerDithering(x, y, image[y][x].r);
          G5f[y][x] = (float)applyBayerDithering(x, y, image[y][x].g);
          B5f[y][x] = (float)applyBayerDithering(x, y, image[y][x].b);
        }
      }

      auto encodePlaneDCT = [&](std::vector<std::vector<float>> &plane, int w,
                                int h) {
        for (int by = 0; by < h; by += 8) {
          for (int bx = 0; bx < w; bx += 8) {
            // Gather block -> scale to 0-255 range and subtract 128
            float block[8][8];
            for (int yb = 0; yb < 8; yb++) {
              for (int xb = 0; xb < 8; xb++) {
                int px = bx + xb;
                int py = by + yb;
                float val = 0.f;
                if (px < w && py < h) {
                  val = (plane[py][px] * 255.0f / 31.0f) - 128.f;
                }
                block[yb][xb] = val;
              }
            }
            // Forward DCT
            float dctOut[8][8];
            forwardDCT8x8(block, dctOut);
            // Quantize & store
            for (int yb = 0; yb < 8; yb++) {
              for (int xb = 0; xb < 8; xb++) {
                float q = QY[yb][xb]; // Use luminance quantization
                float coeff = std::round(dctOut[yb][xb] / q);
                int16_t iCoeff = (int16_t)coeff;
                // Store as 2 bytes (little-endian)
                data.push_back((Uint8)(iCoeff & 0xFF));
                data.push_back((Uint8)((iCoeff >> 8) & 0xFF));
              }
            }
          }
        }
      };

      // Encode each 5-bit color plane
      encodePlaneDCT(R5f, width, height);
      encodePlaneDCT(G5f, width, height);
      encodePlaneDCT(B5f, width, height);
    } else {
      // Original RGB555 with Bayer code
      for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
          Uint8 r5 = applyBayerDithering(x, y, image[y][x].r);
          Uint8 g5 = applyBayerDithering(x, y, image[y][x].g);
          Uint8 b5 = applyBayerDithering(x, y, image[y][x].b);
          Uint16 pixel = (r5 << 10) | (g5 << 5) | b5;
          data.push_back((Uint8)(pixel & 0xFF));
          data.push_back((Uint8)((pixel >> 8) & 0xFF));
        }
      }
    }
    break;
  }
  case ImageType::YCbCr888: {
    // If compression = DctPlusChromaSubsampling, do block-based DCT
    bool useSubsampling =
        (header.compression == CompressionType::DctPlusChromaSubsampling);
    int subsampleWidth = useSubsampling ? (width + 1) / 2 : width;
    int subsampleHeight = useSubsampling ? (height + 1) / 2 : height;

    // Allocate Y/Cb/Cr planes
    std::vector<std::vector<float>> Yf(height, std::vector<float>(width));
    std::vector<std::vector<float>> Cbf(subsampleHeight,
                                        std::vector<float>(subsampleWidth));
    std::vector<std::vector<float>> Crf(subsampleHeight,
                                        std::vector<float>(subsampleWidth));

    // Convert RGB -> YCbCr
    for (int yy = 0; yy < height; yy++) {
      for (int xx = 0; xx < width; xx++) {
        float r = image[yy][xx].r;
        float g = image[yy][xx].g;
        float b = image[yy][xx].b;

        float yVal = 0.299f * r + 0.587f * g + 0.114f * b;
        float cbVal = -0.169f * r - 0.331f * g + 0.500f * b + 128.f;
        float crVal = 0.500f * r - 0.419f * g - 0.081f * b + 128.f;

        Yf[yy][xx] = clamp<float>(yVal, 0.f, 255.f);

        if (!useSubsampling) {
          Cbf[yy][xx] = clamp<float>(cbVal, 0.f, 255.f);
          Crf[yy][xx] = clamp<float>(crVal, 0.f, 255.f);
        } else {
          if ((xx % 2 == 0) && (yy % 2 == 0)) {
            int cby = yy / 2;
            int cbx = xx / 2;
            Cbf[cby][cbx] = clamp<float>(cbVal, 0.f, 255.f);
            Crf[cby][cbx] = clamp<float>(crVal, 0.f, 255.f);
          }
        }
      }
    }

    if (useSubsampling) {
      // -----------------------------------------------------------------
      // // DCT ADDITIONS: Block-based DCT for Y, Cb, Cr
      // -----------------------------------------------------------------
      // We'll store 8×8 block data as 64 int16 per block in row-major order.

      auto encodePlaneDCT = [&](std::vector<std::vector<float>> &plane, int w,
                                int h, const float quant[8][8]) {
        for (int by = 0; by < h; by += 8) {
          for (int bx = 0; bx < w; bx += 8) {
            // Gather block -> subtract 128
            float block[8][8];
            for (int yb = 0; yb < 8; yb++) {
              for (int xb = 0; xb < 8; xb++) {
                int px = bx + xb;
                int py = by + yb;
                float val = 0.f;
                if (px < w && py < h) {
                  val = plane[py][px] - 128.f;
                }
                block[yb][xb] = val;
              }
            }
            // Forward DCT
            float dctOut[8][8];
            forwardDCT8x8(block, dctOut);
            // Quantize & store
            for (int yb = 0; yb < 8; yb++) {
              for (int xb = 0; xb < 8; xb++) {
                float q = quant[yb][xb];
                float coeff = std::round(dctOut[yb][xb] / q);
                int16_t iCoeff = (int16_t)coeff;
                // Store as 2 bytes (little-endian)
                data.push_back((Uint8)(iCoeff & 0xFF));
                data.push_back((Uint8)((iCoeff >> 8) & 0xFF));
              }
            }
          }
        }
      };

      // Encode Y plane (full res)
      encodePlaneDCT(Yf, width, height, QY);
      // Encode Cb plane (subsampled)
      encodePlaneDCT(Cbf, subsampleWidth, subsampleHeight, QC);
      // Encode Cr plane (subsampled)
      encodePlaneDCT(Crf, subsampleWidth, subsampleHeight, QC);
    } else {
      // Existing path (no DCT, just store the planes raw)
      // Store Y
      for (int yy = 0; yy < height; yy++) {
        for (int xx = 0; xx < width; xx++) {
          data.push_back((Uint8)(Yf[yy][xx]));
        }
      }
      // Store Cb
      for (int yy = 0; yy < height; yy++) {
        for (int xx = 0; xx < width; xx++) {
          data.push_back((Uint8)(Cbf[yy][xx]));
        }
      }
      // Store Cr
      for (int yy = 0; yy < height; yy++) {
        for (int xx = 0; xx < width; xx++) {
          data.push_back((Uint8)(Crf[yy][xx]));
        }
      }
    }
    break;
  }
  }

  // Apply filter if specified
  if (header.filter == FilterType::Average) {
    std::vector<Uint8> filtered;
    filtered.reserve(data.size());

    int bytesPerPixel;
    switch (header.type) {
    case ImageType::RGB555_WITH_BAYER_DITHERING:
      bytesPerPixel = 2;
      break;
    default:
      // Typically 3 for RGB888 or 1 if you treat Y-plane data, etc.
      // We'll keep it simple: For raw RGB888 => 3. For DCT data => 1 not
      // typical.
      bytesPerPixel = 3;
      break;
    }

    for (size_t i = 0; i < data.size(); i += bytesPerPixel) {
      for (int b = 0; b < bytesPerPixel; b++) {
        if (i == 0) {
          filtered.push_back(data[i + b]);
        } else {
          int diff = (int)data[i + b] - (int)data[i - bytesPerPixel + b];
          filtered.push_back((Uint8)diff);
        }
      }
    }
    data = std::move(filtered);
  }

  // Apply RLE if specified (unchanged)
  if (header.compression != CompressionType::None) {
    data = compressRLE(data);
  }

  return data;
}

// -----------------------------------------------------------------------------
// deserializeCanvas (updated to handle DCT if compression ==
// DctPlusChromaSubsampling)
// -----------------------------------------------------------------------------
Canvas deserializeCanvas(std::vector<Uint8> data, NFHeader header) {
  // If RLE was used, decompress first
  if (header.compression != CompressionType::None) {
    data = decompressRLE(data);
  }

  // If average filter was applied, reverse it first
  if (header.filter == FilterType::Average) {
    std::vector<Uint8> unfiltered;
    unfiltered.reserve(data.size());

    int bytesPerPixel;
    switch (header.type) {
    case ImageType::RGB555_WITH_BAYER_DITHERING:
      bytesPerPixel = 2;
      break;
    default:
      bytesPerPixel = 3;
      break;
    }

    for (size_t i = 0; i < data.size(); i += bytesPerPixel) {
      for (int b = 0; b < bytesPerPixel; b++) {
        if (i == 0) {
          unfiltered.push_back(data[i + b]);
        } else {
          Uint8 prev = unfiltered[i - bytesPerPixel + b];
          unfiltered.push_back((Uint8)(prev + data[i + b]));
        }
      }
    }
    data = std::move(unfiltered);
  }

  Canvas image(header.height, std::vector<Color>(header.width));
  int width = header.width;
  int height = header.height;

  size_t dataIndex = 0;
  switch (header.type) {
  case ImageType::RGB888: {
    if (header.compression == CompressionType::Dct) {
      // Create planes for R, G, B
      std::vector<std::vector<float>> Rf(height, std::vector<float>(width));
      std::vector<std::vector<float>> Gf(height, std::vector<float>(width));
      std::vector<std::vector<float>> Bf(height, std::vector<float>(width));

      auto decodePlaneDCT = [&](std::vector<std::vector<float>> &plane, int w,
                                int h) {
        for (int by = 0; by < h; by += 8) {
          for (int bx = 0; bx < w; bx += 8) {
            // read 64 int16
            float blockDCT[8][8];
            for (int yb = 0; yb < 8; yb++) {
              for (int xb = 0; xb < 8; xb++) {
                if (dataIndex + 1 >= data.size()) {
                  // Safety check or throw if data is incomplete
                  blockDCT[yb][xb] = 0;
                } else {
                  int16_t coeff =
                      (int16_t)(data[dataIndex] | (data[dataIndex + 1] << 8));
                  dataIndex += 2;
                  blockDCT[yb][xb] = (float)coeff;
                }
              }
            }
            // Dequantize + iDCT
            float spatial[8][8];
            for (int yb = 0; yb < 8; yb++) {
              for (int xb = 0; xb < 8; xb++) {
                blockDCT[yb][xb] *=
                    QY[yb][xb]; // Use luminance quantization for RGB
              }
            }
            inverseDCT8x8(blockDCT, spatial);
            // place back
            for (int yb = 0; yb < 8; yb++) {
              for (int xb = 0; xb < 8; xb++) {
                int px = bx + xb;
                int py = by + yb;
                if (px < w && py < h) {
                  float val = spatial[yb][xb] + 128.f;
                  val = clamp<float>(val, 0.f, 255.f);
                  plane[py][px] = val;
                }
              }
            }
          }
        }
      };

      // Decode each color plane
      decodePlaneDCT(Rf, width, height);
      decodePlaneDCT(Gf, width, height);
      decodePlaneDCT(Bf, width, height);

      // Convert back to RGB
      for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
          image[y][x].r = (Uint8)clamp<int>((int)std::lround(Rf[y][x]), 0, 255);
          image[y][x].g = (Uint8)clamp<int>((int)std::lround(Gf[y][x]), 0, 255);
          image[y][x].b = (Uint8)clamp<int>((int)std::lround(Bf[y][x]), 0, 255);
        }
      }
    } else {
      // Plain RGB888
      for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
          image[y][x].r = data[dataIndex++];
          image[y][x].g = data[dataIndex++];
          image[y][x].b = data[dataIndex++];
        }
      }
    }
    break;
  }
  case ImageType::RGB555_WITH_BAYER_DITHERING: {
    if (header.compression == CompressionType::Dct) {
      // Create planes for R5, G5, B5
      std::vector<std::vector<float>> R5f(height, std::vector<float>(width));
      std::vector<std::vector<float>> G5f(height, std::vector<float>(width));
      std::vector<std::vector<float>> B5f(height, std::vector<float>(width));

      auto decodePlaneDCT = [&](std::vector<std::vector<float>> &plane, int w,
                                int h) {
        for (int by = 0; by < h; by += 8) {
          for (int bx = 0; bx < w; bx += 8) {
            // read 64 int16
            float blockDCT[8][8];
            for (int yb = 0; yb < 8; yb++) {
              for (int xb = 0; xb < 8; xb++) {
                if (dataIndex + 1 >= data.size()) {
                  // Safety check or throw if data is incomplete
                  blockDCT[yb][xb] = 0;
                } else {
                  int16_t coeff =
                      (int16_t)(data[dataIndex] | (data[dataIndex + 1] << 8));
                  dataIndex += 2;
                  blockDCT[yb][xb] = (float)coeff;
                }
              }
            }
            // Dequantize + iDCT
            float spatial[8][8];
            for (int yb = 0; yb < 8; yb++) {
              for (int xb = 0; xb < 8; xb++) {
                blockDCT[yb][xb] *= QY[yb][xb]; // Use luminance quantization
              }
            }
            inverseDCT8x8(blockDCT, spatial);
            // place back and scale from 0-255 to 0-31
            for (int yb = 0; yb < 8; yb++) {
              for (int xb = 0; xb < 8; xb++) {
                int px = bx + xb;
                int py = by + yb;
                if (px < w && py < h) {
                  float val = spatial[yb][xb] + 128.f;
                  val = clamp<float>(val, 0.f, 255.f);
                  val = val * 31.0f / 255.0f;
                  plane[py][px] = val;
                }
              }
            }
          }
        }
      };

      // Decode each color plane
      decodePlaneDCT(R5f, width, height);
      decodePlaneDCT(G5f, width, height);
      decodePlaneDCT(B5f, width, height);

      // Convert back to RGB888
      for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
          Uint8 r5 = (Uint8)clamp<int>((int)std::lround(R5f[y][x]), 0, 31);
          Uint8 g5 = (Uint8)clamp<int>((int)std::lround(G5f[y][x]), 0, 31);
          Uint8 b5 = (Uint8)clamp<int>((int)std::lround(B5f[y][x]), 0, 31);
          image[y][x].r = (r5 << 3) | (r5 >> 2);
          image[y][x].g = (g5 << 3) | (g5 >> 2);
          image[y][x].b = (b5 << 3) | (b5 >> 2);
        }
      }
    } else {
      // Original RGB555 code
      for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
          Uint16 pixel = (Uint16)(data[dataIndex] | (data[dataIndex + 1] << 8));
          dataIndex += 2;
          Uint8 r5 = (pixel >> 10) & 0x1F;
          Uint8 g5 = (pixel >> 5) & 0x1F;
          Uint8 b5 = pixel & 0x1F;
          image[y][x].r = (r5 << 3) | (r5 >> 2);
          image[y][x].g = (g5 << 3) | (g5 >> 2);
          image[y][x].b = (b5 << 3) | (b5 >> 2);
        }
      }
    }
    break;
  }
  case ImageType::YCbCr888: {
    bool useSubsampling =
        (header.compression == CompressionType::DctPlusChromaSubsampling);
    int subsampleWidth = useSubsampling ? (width + 1) / 2 : width;
    int subsampleHeight = useSubsampling ? (height + 1) / 2 : height;

    // We'll decode into float planes
    std::vector<std::vector<float>> Yf(height, std::vector<float>(width));
    std::vector<std::vector<float>> Cbf(subsampleHeight,
                                        std::vector<float>(subsampleWidth));
    std::vector<std::vector<float>> Crf(subsampleHeight,
                                        std::vector<float>(subsampleWidth));

    if (!useSubsampling) {
      // Existing approach: read raw Y, then Cb, then Cr
      for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
          Yf[y][x] = (float)data[dataIndex++];
        }
      }
      for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
          Cbf[y][x] = (float)data[dataIndex++];
        }
      }
      for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
          Crf[y][x] = (float)data[dataIndex++];
        }
      }
    } else {
      // -----------------------------------------------------------------
      // // DCT ADDITIONS: decode 8×8 blocks for Y, Cb, Cr
      // -----------------------------------------------------------------
      auto decodePlaneDCT = [&](std::vector<std::vector<float>> &plane, int w,
                                int h, const float quant[8][8]) {
        for (int by = 0; by < h; by += 8) {
          for (int bx = 0; bx < w; bx += 8) {
            // read 64 int16
            float blockDCT[8][8];
            for (int yb = 0; yb < 8; yb++) {
              for (int xb = 0; xb < 8; xb++) {
                if (dataIndex + 1 >= data.size()) {
                  // Safety check or throw if data is incomplete
                  blockDCT[yb][xb] = 0;
                } else {
                  int16_t coeff =
                      (int16_t)(data[dataIndex] | (data[dataIndex + 1] << 8));
                  dataIndex += 2;
                  blockDCT[yb][xb] = (float)coeff;
                }
              }
            }
            // Dequantize + iDCT
            float spatial[8][8];
            for (int yb = 0; yb < 8; yb++) {
              for (int xb = 0; xb < 8; xb++) {
                blockDCT[yb][xb] *= quant[yb][xb];
              }
            }
            inverseDCT8x8(blockDCT, spatial);
            // place back
            for (int yb = 0; yb < 8; yb++) {
              for (int xb = 0; xb < 8; xb++) {
                int px = bx + xb;
                int py = by + yb;
                if (px < w && py < h) {
                  float val = spatial[yb][xb] + 128.f;
                  val = clamp<float>(val, 0.f, 255.f);
                  plane[py][px] = val;
                }
              }
            }
          }
        }
      };

      // Decode Y plane
      decodePlaneDCT(Yf, width, height, QY);
      // Decode Cb plane
      decodePlaneDCT(Cbf, subsampleWidth, subsampleHeight, QC);
      // Decode Cr plane
      decodePlaneDCT(Crf, subsampleWidth, subsampleHeight, QC);
    }

    // Convert YCbCr -> RGB
    for (int yy = 0; yy < height; yy++) {
      for (int xx = 0; xx < width; xx++) {
        float yVal = Yf[yy][xx];
        float cbVal, crVal;

        if (useSubsampling) {
          int cby = yy >> 1;
          int cbx = xx >> 1;
          cbVal = Cbf[cby][cbx];
          crVal = Crf[cby][cbx];
        } else {
          cbVal = Cbf[yy][xx];
          crVal = Crf[yy][xx];
        }
        cbVal -= 128.f;
        crVal -= 128.f;

        float r = yVal + 1.402f * crVal;
        float g = yVal - 0.344136f * cbVal - 0.714136f * crVal;
        float b = yVal + 1.772f * cbVal;

        image[yy][xx].r = (Uint8)clamp<int>((int)std::lround(r), 0, 255);
        image[yy][xx].g = (Uint8)clamp<int>((int)std::lround(g), 0, 255);
        image[yy][xx].b = (Uint8)clamp<int>((int)std::lround(b), 0, 255);
      }
    }
    break;
  }
  }

  return image;
}