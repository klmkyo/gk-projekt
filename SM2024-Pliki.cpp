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

void ZapisDoPliku(std::string nazwaPliku, TrybObrazu tryb, Dithering dithering,
                  Canvas &obrazek, Canvas1D &paleta) {
  Uint16 szerokoscObrazu = obrazek[0].size();
  Uint16 wysokoscObrazu = obrazek.size();
  const int iloscBitowDoZapisania = 5 * szerokoscObrazu * wysokoscObrazu;

  if (szerokoscObrazu % 8 != 0) {
    throw std::invalid_argument("Szerokosc obrazka nie jest wielokrotnoscia 8");
  }

  std::cout << "Zapisuje obrazek do pliku..." << std::endl;

  std::ofstream wyjscie(nazwaPliku, std::ios::binary);
  wyjscie.write((char *)&FILE_SIGNATURE, sizeof(char) * 2);
  wyjscie.write((char *)&szerokoscObrazu, sizeof(char) * 2);
  wyjscie.write((char *)&wysokoscObrazu, sizeof(char) * 2);
  wyjscie.write((char *)&tryb, sizeof(Uint8));
  wyjscie.write((char *)&dithering, sizeof(Uint8));

  if (czyTrybJestZPaleta(tryb)) {
    for (const auto &c : paleta) {
      wyjscie.write((char *)&c, sizeof(Color));
    }
  }

  std::vector<std::bitset<5>> bitset5(iloscBitowDoZapisania / 5);

  int maxSteps = wysokoscObrazu / 8;
  int bitIndex = 0;
  for (int step = 0; step < maxSteps; step++) {
    int offset = step * 8;
    for (int k = 0; k < szerokoscObrazu; k++) {
      for (int r = 0; r < 8; r++) {
        int columnAbsolute = k;
        int rowAbsolute = offset + r;

        if (tryb == TrybObrazu::PaletaNarzucona) {
          bitset5[bitIndex] =
              z24RGBna5RGB(obrazek[rowAbsolute][columnAbsolute]) >> 3;
        } else if (tryb == TrybObrazu::SzaroscNarzucona) {
          bitset5[bitIndex] = z24RGBna5BW(obrazek[rowAbsolute][columnAbsolute]);
        } else if (tryb == TrybObrazu::SzaroscDedykowana) {
          bitset5[bitIndex] = znajdzNajblizszyKolorIndex(
              obrazek[rowAbsolute][columnAbsolute], paleta);
        } else if (tryb == TrybObrazu::PaletaWykryta) {
          bitset5[bitIndex] = znajdzNajblizszyKolorIndex(
              obrazek[rowAbsolute][columnAbsolute], paleta);

        } else if (tryb == TrybObrazu::PaletaDedykowana) {
          bitset5[bitIndex] = znajdzNajblizszyKolorIndex(
              obrazek[rowAbsolute][columnAbsolute], paleta);
        } else {
          throw std::invalid_argument("Nieznany tryb obrazu");
        }

        bitIndex++;
      }
    }
  }

  unsigned char buffer = 0;
  int bitCount = 0;
  for (const auto &bs : bitset5) {
    for (int i = 0; i < 5; ++i) {
      if (bs.test(i)) {
        buffer |= (1 << bitCount);
      }

      bitCount++;

      if (bitCount == 8) {
        wyjscie.put(buffer);
        buffer = 0;
        bitCount = 0;
      }
    }
  }

  if (bitCount > 0) {
    wyjscie.put(buffer);
  }

  wyjscie.close();
}

void ZapiszCanvasDoBmp(const Canvas &image, const std::string &filename) {
  int32_t width = image[0].size();
  int32_t height = image.size();
  int32_t rowPadding = (4 - (width * 3) % 4) % 4;
  int32_t fileSize = 54 + (3 * width + rowPadding) * height;

  std::cout << "W obrazku jest " << getUniqueColorsInImage(image)
            << " unikalnych kolorow" << std::endl;

  std::ofstream file(filename, std::ios::out | std::ios::binary);

  file.put('B').put('M');
  file.write(reinterpret_cast<const char *>(&fileSize), 4);
  file.write("\0\0\0\0", 4);
  file.write("\x36\0\0\0", 4);

  file.write("\x28\0\0\0", 4);
  file.write(reinterpret_cast<const char *>(&width), 4);
  file.write(reinterpret_cast<const char *>(&height), 4);
  file.write("\1\0", 2);
  file.write("\x18\0", 2);
  file.write("\0\0\0\0", 4);
  file.write("\0\0\0\0", 4);
  file.write("\x13\0\0\0", 4);
  file.write("\x13\0\0\0", 4);
  file.write("\0\0\0\0", 4);
  file.write("\0\0\0\0", 4);

  for (int y = 0; y < height; ++y) {
    for (int x = 0; x < width; ++x) {
      Color color = image[height - y - 1][x];
      file.put(color.b).put(color.g).put(color.r);
    }
    file.write("\0\0\0", rowPadding);
  }

  file.close();
}

void KonwertujKfcNaBmp(std::string kfcZrodlo, std::string bmpCel) {
  Canvas obrazek = OdczytZPliku(kfcZrodlo);
  std::cout << "Zapisuje obrazek do pliku..." << std::endl;

  ZapiszCanvasDoBmp(obrazek, bmpCel);
}

Canvas OdczytZPliku(const std::string &filename) {
  std::cout << "Wczytuje obrazek " << filename << " z pliku..." << std::endl;

  std::ifstream wejscie(filename, std::ios::binary);
  char id[2];
  Uint16 szerokoscObrazu;
  Uint16 wysokoscObrazu;

  char _tryb, _dithering;

  wejscie.read((char *)&id, sizeof(char) * 2);
  wejscie.read((char *)&szerokoscObrazu, sizeof(char) * 2);
  wejscie.read((char *)&wysokoscObrazu, sizeof(char) * 2);
  wejscie.read((char *)&_tryb, sizeof(char));
  wejscie.read((char *)&_dithering, sizeof(char));

  TrybObrazu tryb = (TrybObrazu)_tryb;
  Dithering dithering = (Dithering)_dithering;

  Canvas1D paleta;
  if (czyTrybJestZPaleta(tryb)) {
    paleta = Canvas1D(PALETA_SIZE);
    for (auto &c : paleta) {
      wejscie.read((char *)&c, sizeof(Color));
    }
  }

  int iloscBitowDoOdczytania = szerokoscObrazu * wysokoscObrazu * 5;

  std::vector<std::bitset<5>> bitset5(iloscBitowDoOdczytania / 5);

  unsigned char buffer;
  int bitCount = 0;
  int bitIndex = 0;

  while (wejscie.get((char &)buffer)) {
    for (int i = 0; i < 8; ++i) {
      int bit = (buffer >> i) & 1;

      bitset5[bitIndex].set(bitCount, bit);

      bitCount++;

      if (bitCount == 5) {
        bitCount = 0;
        bitIndex++;
      }
    }
  }

  int maxSteps = wysokoscObrazu / 8;
  bitIndex = 0;

  Canvas obrazek(wysokoscObrazu, std::vector<Color>(szerokoscObrazu));

  for (int step = 0; step < maxSteps; step++) {
    int offset = step * 8;
    for (int k = 0; k < szerokoscObrazu; k++) {
      for (int r = 0; r < 8; r++) {
        int columnAbsolute = k;
        int rowAbsolute = offset + r;

        if (tryb == TrybObrazu::PaletaNarzucona) {
          obrazek[rowAbsolute][columnAbsolute] =
              z5RGBna24RGB(bitset5[bitIndex].to_ulong() << 3);
        } else if (tryb == TrybObrazu::SzaroscNarzucona) {
          obrazek[rowAbsolute][columnAbsolute] =
              z5BWna24RGB(bitset5[bitIndex].to_ulong());
        } else if (tryb == TrybObrazu::SzaroscDedykowana) {
          obrazek[rowAbsolute][columnAbsolute] =
              paleta[bitset5[bitIndex].to_ulong()];
        } else if (tryb == TrybObrazu::PaletaWykryta) {
          obrazek[rowAbsolute][columnAbsolute] =
              paleta[bitset5[bitIndex].to_ulong()];
        } else if (tryb == TrybObrazu::PaletaDedykowana) {
          obrazek[rowAbsolute][columnAbsolute] =
              paleta[bitset5[bitIndex].to_ulong()];
        } else {
          throw std::invalid_argument("Nieznany tryb obrazu");
        }

        bitIndex++;
      }
    }
  }

  wejscie.close();
  return obrazek;
}

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

Canvas ladujBMPDoPamieci(std::string nazwa) {
  SDL_Surface *bmp = SDL_LoadBMP(nazwa.c_str());
  if (!bmp) {
    printf("Unable to load bitmap: %s\n", SDL_GetError());
    exit(1);
  } else {
    Canvas obrazek(bmp->h, std::vector<Color>(bmp->w));
    Color kolor;

    for (int yy = 0; yy < bmp->h; yy++) {
      for (int xx = 0; xx < bmp->w; xx++) {
        SDL_Color kolorSDL = getPixelSurface(xx, yy, bmp);
        kolor.r = kolorSDL.r;
        kolor.g = kolorSDL.g;
        kolor.b = kolorSDL.b;
        obrazek[yy][xx] = kolor;
      }
    }
    SDL_FreeSurface(bmp);
    return obrazek;
  }
}

void KonwertujBmpNaKfc(std::string bmpZrodlo, std::string kfcCel,
                       TrybObrazu tryb, Dithering d) {
  Canvas obrazek = ladujBMPDoPamieci(bmpZrodlo);
  Canvas1D obrazek1D = wyprostujCanvas(obrazek);
  Canvas1D paleta;

  switch (tryb) {
  case TrybObrazu::PaletaDedykowana: {
    medianCutRGB(0, obrazek1D.size() - 1, 5, obrazek1D, paleta);
    break;
  }
  case TrybObrazu::SzaroscDedykowana: {
    medianCutBW(0, obrazek1D.size() - 1, 5, obrazek1D, paleta);
    break;
  }
  case TrybObrazu::PaletaWykryta: {
    std::unordered_set<Color> paletaSet;

    Canvas1D shuffledObrazek1D(obrazek1D);
    std::srand(std::time(nullptr));
    std::random_shuffle(shuffledObrazek1D.begin(), shuffledObrazek1D.end());

    for (const auto &c : shuffledObrazek1D) {
      paletaSet.insert(c);
      if (paletaSet.size() >= 32)
        break;
    }

    paleta = Canvas1D(paletaSet.begin(), paletaSet.end());

    while (paleta.size() < 32) {
      paleta.push_back({0, 0, 0});
    }
    break;
  }

  default: {
    break;
  }
  }

  switch (d) {
  case Dithering::Floyd: {
    if (czyTrybJestZPaleta(tryb)) {
      applyFloydSteinbergDithering(obrazek, paleta,
                                   tryb == TrybObrazu::SzaroscDedykowana);
    } else {
      applyFloydSteinbergDithering(obrazek,
                                   tryb == TrybObrazu::SzaroscNarzucona);
    }
    break;
  }
  case Dithering::Bayer: {
    if (czyTrybJestZPaleta(tryb)) {
      applyBayerDithering5RGB(obrazek, paleta);
    } else {
      applyBayerDithering5RGB(obrazek, tryb == TrybObrazu::SzaroscNarzucona);
    }
    break;
  }
  default: {
    break;
  }
  }

  ZapisDoPliku(kfcCel, tryb, d, obrazek, paleta);
  std::cout << "Zapisano obrazek w formacie KFC" << std::endl;
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

  if (type == ImageType::RGB555) {
    for (const auto &row : image) {
      for (const auto &pixel : row) {
        Uint16 rgb555 =
            ((pixel.r & 0xF8) << 7) | ((pixel.g & 0xF8) << 2) | (pixel.b >> 3);
        data.push_back(rgb555 & 0xFF);
        data.push_back((rgb555 >> 8) & 0xFF);
      }
    }
  } else if (type == ImageType::RGB565) {
    for (const auto &row : image) {
      for (const auto &pixel : row) {
        Uint16 rgb565 =
            ((pixel.r & 0xF8) << 8) | ((pixel.g & 0xFC) << 3) | (pixel.b >> 3);
        data.push_back(rgb565 & 0xFF);
        data.push_back((rgb565 >> 8) & 0xFF);
      }
    }
  } else if (type == ImageType::YUV || type == ImageType::YIQ ||
             type == ImageType::YCbCr) {
    std::vector<std::vector<Uint8>> Y(height, std::vector<Uint8>(width));
    std::vector<std::vector<Uint8>> C1(height, std::vector<Uint8>(width));
    std::vector<std::vector<Uint8>> C2(height, std::vector<Uint8>(width));

    for (int y = 0; y < height; ++y) {
      for (int x = 0; x < width; ++x) {
        Color &pixel = image[y][x];
        if (type == ImageType::YUV) {
          YUV yuv = rgbToYuv(pixel.r, pixel.g, pixel.b);
          Y[y][x] = yuv.y;
          C1[y][x] = yuv.u;
          C2[y][x] = yuv.v;
        } else if (type == ImageType::YIQ) {
          YIQ yiq = rgbToYiq(pixel.r, pixel.g, pixel.b);
          Y[y][x] = yiq.y;
          C1[y][x] = yiq.i;
          C2[y][x] = yiq.q;
        } else if (type == ImageType::YCbCr) {
          YCbCr ycbcr = rgbToYcbcr(pixel.r, pixel.g, pixel.b);
          Y[y][x] = ycbcr.y;
          C1[y][x] = ycbcr.cb;
          C2[y][x] = ycbcr.cr;
        }
      }
    }

    if (subsamplingEnabled) {
      int subsampledWidth = (width + 1) / 2;
      int subsampledHeight = (height + 1) / 2;
      std::vector<std::vector<Uint8>> C1_sub(
          subsampledHeight, std::vector<Uint8>(subsampledWidth));
      std::vector<std::vector<Uint8>> C2_sub(
          subsampledHeight, std::vector<Uint8>(subsampledWidth));

      for (int y = 0; y < subsampledHeight; ++y) {
        for (int x = 0; x < subsampledWidth; ++x) {
          int sumC1 = 0, sumC2 = 0, count = 0;
          for (int dy = 0; dy < 2; ++dy) {
            for (int dx = 0; dx < 2; ++dx) {
              int yy = y * 2 + dy;
              int xx = x * 2 + dx;
              if (yy < height && xx < width) {
                sumC1 += C1[yy][xx];
                sumC2 += C2[yy][xx];
                ++count;
              }
            }
          }
          C1_sub[y][x] = sumC1 / count;
          C2_sub[y][x] = sumC2 / count;
        }
      }

      for (const auto &row : Y)
        data.insert(data.end(), row.begin(), row.end());
      for (const auto &row : C1_sub)
        data.insert(data.end(), row.begin(), row.end());
      for (const auto &row : C2_sub)
        data.insert(data.end(), row.begin(), row.end());
    } else {
      for (const auto &row : Y)
        data.insert(data.end(), row.begin(), row.end());
      for (const auto &row : C1)
        data.insert(data.end(), row.begin(), row.end());
      for (const auto &row : C2)
        data.insert(data.end(), row.begin(), row.end());
    }
  } else if (type == ImageType::HSL) {
    std::vector<std::vector<Uint8>> H(height, std::vector<Uint8>(width));
    std::vector<std::vector<Uint8>> S(height, std::vector<Uint8>(width));
    std::vector<std::vector<Uint8>> L(height, std::vector<Uint8>(width));

    for (int y = 0; y < height; ++y) {
      for (int x = 0; x < width; ++x) {
        Color &pixel = image[y][x];
        HSL hsl = rgbToHsl(pixel.r, pixel.g, pixel.b);

        H[y][x] = hsl.h;
        S[y][x] = hsl.s;
        L[y][x] = hsl.l;
      }
    }

    for (const auto &row : H)
      data.insert(data.end(), row.begin(), row.end());
    for (const auto &row : S)
      data.insert(data.end(), row.begin(), row.end());
    for (const auto &row : L)
      data.insert(data.end(), row.begin(), row.end());
  } else {
    throw std::invalid_argument("Unsupported ImageType in serializeCanvas");
  }
  return data;
}

Canvas deserializeCanvas(std::vector<Uint8> data, NFHeader header) {
  Canvas image(header.height, std::vector<Color>(header.width));
  int width = header.width;
  int height = header.height;

  if (header.type == ImageType::RGB555) {
    int idx = 0;
    for (int y = 0; y < height; ++y) {
      for (int x = 0; x < width; ++x) {
        Uint16 rgb555 = data[idx] | (data[idx + 1] << 8);
        idx += 2;
        Uint8 r = ((rgb555 >> 7) & 0xF8) | ((rgb555 >> 12) & 0x07);
        Uint8 g = ((rgb555 >> 2) & 0xF8) | ((rgb555 >> 7) & 0x07);
        Uint8 b = ((rgb555 << 3) & 0xF8) | ((rgb555 >> 2) & 0x07);
        image[y][x] = {r, g, b};
      }
    }
  } else if (header.type == ImageType::RGB565) {
    int idx = 0;
    for (int y = 0; y < height; ++y) {
      for (int x = 0; x < width; ++x) {
        Uint16 rgb565 = data[idx] | (data[idx + 1] << 8);
        idx += 2;
        Uint8 r = ((rgb565 >> 8) & 0xF8) | ((rgb565 >> 13) & 0x07);
        Uint8 g = ((rgb565 >> 3) & 0xFC) | ((rgb565 >> 9) & 0x03);
        Uint8 b = ((rgb565 << 3) & 0xF8) | ((rgb565 >> 2) & 0x07);
        image[y][x] = {r, g, b};
      }
    }
  } else if (header.type == ImageType::YUV || header.type == ImageType::YIQ ||
             header.type == ImageType::YCbCr) {
    int idx = 0;
    std::vector<std::vector<Uint8>> Y(height, std::vector<Uint8>(width));
    std::vector<std::vector<Uint8>> C1, C2;

    if (header.subsamplingEnabled) {
      int subsampledWidth = (width + 1) / 2;
      int subsampledHeight = (height + 1) / 2;
      C1.resize(subsampledHeight, std::vector<Uint8>(subsampledWidth));
      C2.resize(subsampledHeight, std::vector<Uint8>(subsampledWidth));

      for (auto &row : Y)
        for (auto &val : row)
          val = data[idx++];

      for (auto &row : C1)
        for (auto &val : row)
          val = data[idx++];
      for (auto &row : C2)
        for (auto &val : row)
          val = data[idx++];

      std::vector<std::vector<Uint8>> C1_full(height,
                                              std::vector<Uint8>(width));
      std::vector<std::vector<Uint8>> C2_full(height,
                                              std::vector<Uint8>(width));

      for (int y = 0; y < height; ++y) {
        int yy = y / 2;
        for (int x = 0; x < width; ++x) {
          int xx = x / 2;
          C1_full[y][x] = C1[yy][xx];
          C2_full[y][x] = C2[yy][xx];
        }
      }
      C1 = std::move(C1_full);
      C2 = std::move(C2_full);
    } else {
      C1.resize(height, std::vector<Uint8>(width));
      C2.resize(height, std::vector<Uint8>(width));

      for (auto &row : Y)
        for (auto &val : row)
          val = data[idx++];
      for (auto &row : C1)
        for (auto &val : row)
          val = data[idx++];
      for (auto &row : C2)
        for (auto &val : row)
          val = data[idx++];
    }

    for (int y = 0; y < height; ++y) {
      for (int x = 0; x < width; ++x) {
        Color &pixel = image[y][x];
        if (header.type == ImageType::YUV) {
          SDL_Color color = yuvToRgb(Y[y][x], C1[y][x], C2[y][x]);
          pixel.r = color.r;
          pixel.g = color.g;
          pixel.b = color.b;
        } else if (header.type == ImageType::YIQ) {
          SDL_Color color = yiqToRgb(Y[y][x], C1[y][x], C2[y][x]);
          pixel.r = color.r;
          pixel.g = color.g;
          pixel.b = color.b;
        } else if (header.type == ImageType::YCbCr) {
          SDL_Color color = ycbcrToRgb(Y[y][x], C1[y][x], C2[y][x]);
          pixel.r = color.r;
          pixel.g = color.g;
          pixel.b = color.b;
        }
      }
    }
  } else if (header.type == ImageType::HSL) {
    int idx = 0;
    std::vector<std::vector<Uint8>> H(height, std::vector<Uint8>(width));
    std::vector<std::vector<Uint8>> S(height, std::vector<Uint8>(width));
    std::vector<std::vector<Uint8>> L(height, std::vector<Uint8>(width));

    for (auto &row : H)
      for (auto &val : row)
        val = data[idx++];
    for (auto &row : S)
      for (auto &val : row)
        val = data[idx++];
    for (auto &row : L)
      for (auto &val : row)
        val = data[idx++];

    for (int y = 0; y < height; ++y) {
      for (int x = 0; x < width; ++x) {
        HSL hsl = {H[y][x], S[y][x], L[y][x]};
        SDL_Color color = hslToRgb(hsl.h, hsl.s, hsl.l);
        image[y][x].r = color.r;
        image[y][x].g = color.g;
        image[y][x].b = color.b;
      }
    }
  } else {
    throw std::invalid_argument("Unsupported ImageType in deserializeCanvas");
  }
  return image;
}
