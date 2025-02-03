#include "SM2024-Pliki.h"
#include "SM2024-Konwersje.h"
#include "SM2024-Zmienne.h"
#include <SDL.h>
#include <algorithm>
#include <ctime>
#include <fstream>
#include <iostream>
#include <stdexcept>

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

  // // debug print the whole file as bytes
  // for (Uint8 byte : bytes) {
  //     std::cout << std::hex << (int)byte << " ";
  // }
  // std::cout << std::dec << std::endl;

  // header is sizeof(NFHeader) bytes
  int headerSize = NFHEADER_SIZE;
  std::cout << "Header size: " << headerSize << std::endl;
  std::vector<Uint8> headerBytes(bytes.begin(), bytes.begin() + headerSize);
  NFHeader header = deserializeHeader(headerBytes);

  // // debug print the header data
  // std::cout << "Loaded image header: " << std::endl;
  // for (int i = 0; i < headerSize; i++) {
  //     std::cout << std::hex << (int)headerBytes[i] << " ";
  // }
  // std::cout << std::dec << std::endl;

  // image data is the rest of the bytes
  std::vector<Uint8> imageData(bytes.begin() + headerSize, bytes.end());

  // // print the data that we're passing to deserializeCanvas (in hex)
  // for (Uint8 byte : imageData) {
  //     std::cout << std::hex << (int)byte << " ";
  // }

  // std::cout << std::dec << std::endl;

  Canvas image = deserializeCanvas(imageData, header);

  return {header, image};
}

template <typename T> T clamp(const T &value, const T &min, const T &max) {
  if (value < min)
    return min;
  if (value > max)
    return max;
  return value;
}

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
  int width = image[0].size();
  int height = image.size();

  switch (header.type) {
  case ImageType::RGB888: {
    // Standard RGB888 format - 3 bytes per pixel
    for (int y = 0; y < height; y++) {
      for (int x = 0; x < width; x++) {
        data.push_back(image[y][x].r);
        data.push_back(image[y][x].g);
        data.push_back(image[y][x].b);
      }
    }
    break;
  }
  case ImageType::RGB555_WITH_BAYER_DITHERING: {
    // RGB555 format - 2 bytes per pixel (5 bits per channel)
    for (int y = 0; y < height; y++) {
      for (int x = 0; x < width; x++) {
        // Convert RGB888 to RGB555
        Uint16 r5 = (image[y][x].r >> 3) & 0x1F;
        Uint16 g5 = (image[y][x].g >> 3) & 0x1F;
        Uint16 b5 = (image[y][x].b >> 3) & 0x1F;

        // Pack into 16 bits: RRRRRGGGGGGBBBBB
        Uint16 pixel = (r5 << 10) | (g5 << 5) | b5;

        // Store as two bytes
        data.push_back(pixel & 0xFF);
        data.push_back((pixel >> 8) & 0xFF);
      }
    }
    break;
  }
  case ImageType::YCbCr888: {
    // YCbCr format with optional subsampling
    std::vector<std::vector<Uint8>> Y(height, std::vector<Uint8>(width));
    std::vector<std::vector<Uint8>> Cb, Cr;

    bool useSubsampling =
        (header.compression == CompressionType::DctPlusChromaSubsampling);
    int subsample_width = useSubsampling ? (width + 1) / 2 : width;
    int subsample_height = useSubsampling ? (height + 1) / 2 : height;

    Cb.resize(subsample_height, std::vector<Uint8>(subsample_width));
    Cr.resize(subsample_height, std::vector<Uint8>(subsample_width));

    // Convert RGB to YCbCr
    for (int y = 0; y < height; y++) {
      for (int x = 0; x < width; x++) {
        // RGB to YCbCr conversion
        float r = image[y][x].r;
        float g = image[y][x].g;
        float b = image[y][x].b;

        Uint8 y_val = clamp<int>(0.299 * r + 0.587 * g + 0.114 * b, 0, 255);
        Y[y][x] = y_val;

        if (!useSubsampling || (x % 2 == 0 && y % 2 == 0)) {
          int cb_x = x >> (useSubsampling ? 1 : 0);
          int cb_y = y >> (useSubsampling ? 1 : 0);

          Uint8 cb_val =
              clamp<int>(-0.169 * r - 0.331 * g + 0.500 * b + 128, 0, 255);
          Uint8 cr_val =
              clamp<int>(0.500 * r - 0.419 * g - 0.081 * b + 128, 0, 255);

          Cb[cb_y][cb_x] = cb_val;
          Cr[cb_y][cb_x] = cr_val;
        }
      }
    }

    // Store Y channel
    for (int y = 0; y < height; y++) {
      for (int x = 0; x < width; x++) {
        data.push_back(Y[y][x]);
      }
    }

    // Store Cb and Cr channels
    for (int y = 0; y < subsample_height; y++) {
      for (int x = 0; x < subsample_width; x++) {
        data.push_back(Cb[y][x]);
      }
    }
    for (int y = 0; y < subsample_height; y++) {
      for (int x = 0; x < subsample_width; x++) {
        data.push_back(Cr[y][x]);
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
      bytesPerPixel = 3;
    }

    for (size_t i = 0; i < data.size(); i += bytesPerPixel) {
      for (int b = 0; b < bytesPerPixel; b++) {
        if (i == 0) {
          filtered.push_back(data[i + b]);
        } else {
          int diff = static_cast<int>(data[i + b]) -
                     static_cast<int>(data[i - bytesPerPixel + b]);
          filtered.push_back(static_cast<Uint8>(diff));
        }
      }
    }
    data = std::move(filtered);
  }

  return data;
}

Canvas deserializeCanvas(std::vector<Uint8> data, NFHeader header) {
  Canvas image(header.height, std::vector<Color>(header.width));
  int width = header.width;
  int height = header.height;

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
    }

    for (size_t i = 0; i < data.size(); i += bytesPerPixel) {
      for (int b = 0; b < bytesPerPixel; b++) {
        if (i == 0) {
          unfiltered.push_back(data[i + b]);
        } else {
          Uint8 prev = unfiltered[i - bytesPerPixel + b];
          unfiltered.push_back(static_cast<Uint8>(prev + data[i + b]));
        }
      }
    }
    data = std::move(unfiltered);
  }

  size_t dataIndex = 0;
  switch (header.type) {
  case ImageType::RGB888: {
    for (int y = 0; y < height; y++) {
      for (int x = 0; x < width; x++) {
        image[y][x].r = data[dataIndex++];
        image[y][x].g = data[dataIndex++];
        image[y][x].b = data[dataIndex++];
      }
    }
    break;
  }
  case ImageType::RGB555_WITH_BAYER_DITHERING: {
    for (int y = 0; y < height; y++) {
      for (int x = 0; x < width; x++) {
        // Read 16-bit pixel value
        Uint16 pixel = data[dataIndex] | (data[dataIndex + 1] << 8);
        dataIndex += 2;

        // Extract RGB555 components
        Uint8 r5 = (pixel >> 10) & 0x1F;
        Uint8 g5 = (pixel >> 5) & 0x1F;
        Uint8 b5 = pixel & 0x1F;

        // Convert back to RGB888
        image[y][x].r = (r5 << 3) | (r5 >> 2);
        image[y][x].g = (g5 << 3) | (g5 >> 2);
        image[y][x].b = (b5 << 3) | (b5 >> 2);
      }
    }
    break;
  }
  case ImageType::YCbCr888: {
    bool useSubsampling =
        (header.compression == CompressionType::DctPlusChromaSubsampling);
    int subsample_width = useSubsampling ? (width + 1) / 2 : width;
    int subsample_height = useSubsampling ? (height + 1) / 2 : height;

    // Read Y channel
    std::vector<std::vector<Uint8>> Y(height, std::vector<Uint8>(width));
    for (int y = 0; y < height; y++) {
      for (int x = 0; x < width; x++) {
        Y[y][x] = data[dataIndex++];
      }
    }

    // Read Cb and Cr channels
    std::vector<std::vector<Uint8>> Cb(subsample_height,
                                       std::vector<Uint8>(subsample_width));
    std::vector<std::vector<Uint8>> Cr(subsample_height,
                                       std::vector<Uint8>(subsample_width));

    for (int y = 0; y < subsample_height; y++) {
      for (int x = 0; x < subsample_width; x++) {
        Cb[y][x] = data[dataIndex++];
      }
    }
    for (int y = 0; y < subsample_height; y++) {
      for (int x = 0; x < subsample_width; x++) {
        Cr[y][x] = data[dataIndex++];
      }
    }

    // Convert YCbCr back to RGB
    for (int y = 0; y < height; y++) {
      for (int x = 0; x < width; x++) {
        float y_val = Y[y][x];
        float cb_val, cr_val;

        if (useSubsampling) {
          // Use nearest neighbor sampling for chroma components
          int cb_x = x >> 1;
          int cb_y = y >> 1;
          cb_val = Cb[cb_y][cb_x];
          cr_val = Cr[cb_y][cb_x];
        } else {
          cb_val = Cb[y][x];
          cr_val = Cr[y][x];
        }

        // YCbCr to RGB conversion
        cb_val -= 128;
        cr_val -= 128;

        float r = y_val + 1.402 * cr_val;
        float g = y_val - 0.344136 * cb_val - 0.714136 * cr_val;
        float b = y_val + 1.772 * cb_val;

        image[y][x].r = clamp<int>(r, 0, 255);
        image[y][x].g = clamp<int>(g, 0, 255);
        image[y][x].b = clamp<int>(b, 0, 255);
      }
    }
    break;
  }
  }

  return image;
}
