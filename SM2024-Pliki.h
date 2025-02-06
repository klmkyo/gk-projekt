#pragma once
#include "SM2024-Zmienne.h"
#include <string>

SDL_Color getPixelSurface(int x, int y, SDL_Surface *surface);

enum class ImageType {
  RGB555_WITH_BAYER_DITHERING = 0,
  RGB888 = 1,
  YCbCr888 = 2,
};

enum class FilterType { None = 0, Average = 3 };

enum class CompressionType {
  None = 0,
  Dct = 1,
  DctPlusChromaSubsampling = 2,
  RLE = 3,
};




struct NFHeader {
  char magic[4];
  Uint8 version;
  ImageType type;
  FilterType filter;
  CompressionType compression;
  Uint16 width;
  Uint16 height;
};

const int NFHEADER_SIZE_UNPADDED = 4 + 
                                   1 + 
                                   1 + 
                                   1 + 
                                   1 + 
                                   2 + 
                                   2;  


const int NFHEADER_SIZE = 32;



struct NFHeaderUser {
  ImageType type;
  FilterType filter;
  CompressionType compression;
  Uint16 width;
  Uint16 height;
};

std::vector<Uint8> serializeHeader(NFHeader header);
NFHeader deserializeHeader(std::vector<Uint8> data);

std::vector<Uint8> serializeCanvas(Canvas &image, NFHeaderUser header);
std::vector<Uint8> serializeCanvas(Canvas &image, NFHeader header);
Canvas deserializeCanvas(std::vector<Uint8> data, NFHeader header);

void saveNFImage(const std::string &filename, NFHeaderUser header,
                 Canvas &image);
std::pair<NFHeader, Canvas> loadNFImage(const std::string &filename);
