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

  headerData.push_back((Uint8)header.subsamplingEnabled);

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

  header.subsamplingEnabled = data[12];

  return header;
}

void saveNFImage(const std::string &filename, NFHeaderUser header,
                 Canvas &image) {
  std::vector<Uint8> bytes;

  // add header
  std::vector<Uint8> headerBytes = serializeHeader(header);
  bytes.insert(bytes.end(), headerBytes.begin(), headerBytes.end());

  // add image data
  std::vector<Uint8> imageData =
      serializeCanvas(image, header.type, header.subsamplingEnabled);

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

std::vector<Uint8> serializeCanvas(Canvas &image, ImageType type,
                                   bool subsamplingEnabled) {
  std::vector<Uint8> data;
  int width = image[0].size();
  int height = image.size();

  // TODO serializing all ImageType into bytes

  return data;
}

Canvas deserializeCanvas(std::vector<Uint8> data, NFHeader header) {
  Canvas image(header.height, std::vector<Color>(header.width));
  int width = header.width;
  int height = header.height;

  // TODO deserializing all ImageType into bytes

  return image;
}
