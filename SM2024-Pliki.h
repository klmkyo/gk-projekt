#pragma once
#include "SM2024-Zmienne.h"
#include "SM2024-Funkcje.h"
#include "SM2024-MedianCut.h"
#include <string>

void ZapisDoPliku(std::string nazwaPliku, TrybObrazu tryb, Dithering dithering, Canvas &obrazek, Canvas1D &paleta);
Canvas OdczytZPliku(const std::string &filename);
void ZapiszCanvasDoBmp(const Canvas& image, const std::string& filename);
Canvas ladujBMPDoPamieci(std::string nazwa);
void KonwertujBmpNaKfc(std::string bmpZrodlo, std::string kfcCel, TrybObrazu tryb, Dithering d);
void KonwertujKfcNaBmp(std::string kfcZrodlo, std::string bmpCel);
SDL_Color getPixelSurface(int x, int y, SDL_Surface *surface);

enum class ImageType {
    YUV = 0,
    YIQ = 1,
    YCbCr = 2,
    HSL = 3,
    RGB555 = 4,
    RGB565 = 5
};

enum class FilterType {
    None = 0,
    Sub = 1,
    Up = 2,
    Average = 3,
    Paeth = 4
};

enum class CompressionType {
    None = 0,
};

// What the file contains
// After that will be the image data, size of each
// channel will be known based on width, height and subsampling info
struct NFHeader {
    char magic[4];
    Uint8 version;
    ImageType type;
    FilterType filter;
    CompressionType compression;
    Uint16 width;
    Uint16 height;
    // 4:2:0, każdy ImageType ma z góry określone subsamplingi
    bool subsamplingEnabled = false;
};

const int NFHEADER_SIZE_UNPADDED = 
    4 + // Magic
    1 + // Version
    1 + // ImageType
    1 + // FilterType
    1 + // CompressionType
    2 + // Width
    2 + // Height
    1;  // SubsamplingEnabled

// pad to 4 bytes
const int NFHEADER_SIZE = 32;

// What the user uses to create header (the same thing, but without magic and version)
struct NFHeaderUser {
    ImageType type;
    FilterType filter;
    CompressionType compression;
    Uint16 width;
    Uint16 height;
    bool subsamplingEnabled = false;
};

std::vector<Uint8> serializeHeader(NFHeader header);
NFHeader deserializeHeader(std::vector<Uint8> data);

std::vector<Uint8> serializeCanvas(Canvas &image, ImageType type, bool subsamplingEnabled);
Canvas deserializeCanvas(std::vector<Uint8> data, NFHeader header);

void saveNFImage(const std::string &filename, NFHeaderUser header, Canvas &image);
std::pair<NFHeader, Canvas> loadNFImage(const std::string &filename);