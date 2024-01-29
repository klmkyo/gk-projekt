#include <algorithm>
#include <bitset>
#include <cmath>
#include <ctime>
#include <exception>
#include <fstream>
#include <iostream>
#include <limits.h>
#include <map>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <string>
#include <unordered_set>
#include <vector>

#include "SDL_surface.h"

using namespace std;

#define PALETA_SIZE 32
#define OBRAZEK_SIZE 64000

struct Color {
  Uint8 r, g, b;
};

const char FILE_SIGNATURE[2] = {0x33, 0x33};

namespace std {
template <> struct hash<Color> {
  size_t operator()(const Color &k) const {
    return ((hash<Uint8>()(k.r) ^ (hash<Uint8>()(k.g) << 1)) >> 1) ^
           (hash<Uint8>()(k.b) << 1);
  }
};
template <> struct equal_to<Color> {
  bool operator()(const Color &lhs, const Color &rhs) const {
    return lhs.r == rhs.r && lhs.g == rhs.g && lhs.b == rhs.b;
  }
};
} // namespace std

typedef std::vector<std::vector<Color>> Canvas;
typedef std::vector<Color> Canvas1D;

enum SkladowaRGB {
  R,
  G,
  B,
};

enum TrybObrazu {
  PaletaNarzucona = 1,
  SzaroscNarzucona = 2,
  SzaroscDedykowana = 3,
  PaletaWykryta = 4,
  PaletaDedykowana = 5

};
enum Dithering { Brak = 0, Bayer = 1, Floyd = 2 };

constexpr int maxKolorow = 320 * 600;

bool czyTrybJestZPaleta(TrybObrazu tryb) { return tryb >= 3; }
SkladowaRGB najwiekszaRoznica(int start, int koniec, Canvas1D &obrazek);

void setPixel(int x, int y, Uint8 R, Uint8 G, Uint8 B);
Color getPixel(int x, int y);

Uint8 z24RGBna5RGB(Color kolor);
Color z5RGBna24RGB(Uint8 kolor5bit);

void ZapisDoPliku(std::string nazwaPliku, TrybObrazu tryb, Dithering dithering,
                  Canvas &obrazek, Canvas1D &paleta);
void czyscEkran(Uint8 R, Uint8 G, Uint8 B);
Canvas OdczytZPliku(const std::string &filename);

void KonwertujBmpNaKfc(std::string bmpZrodlo, std::string kfcCel,
                       TrybObrazu tryb, Dithering d);
Canvas1D wyprostujCanvas(Canvas &obrazek);

Uint8 normalizacja(int wartosc) {
  if (wartosc < 0)
    return 0;
  if (wartosc > 255)
    return 255;
  return wartosc;
}

Canvas ladujBMPDoPamieci(std::string nazwa);
bool porownajKolory(Color kolor1, Color kolor2);

int getUniqueColorsInImage(const Canvas &image) {
  std::unordered_set<Color> colors;
  for (const auto &row : image) {
    for (const auto &pixel : row) {
      colors.insert(pixel);
    }
  }
  return colors.size();
}

Uint8 z24RGBna5RGB(Color kolor) {
  Uint8 nowyR, nowyG, nowyB;
  nowyR = round(kolor.r * 3.0 / 255.0);
  nowyG = round(kolor.g * 3.0 / 255.0);
  nowyB = round(kolor.b * 1.0 / 255.0);

  return (nowyR << 6) | (nowyG << 4) | (nowyB << 3);
}

Color z5RGBna24RGB(Uint8 kolor5bit) {
  Color kolor;
  kolor.r = ((kolor5bit & 0b11000000) >> 6) * 255.0 / 3.0;
  kolor.g = ((kolor5bit & 0b00110000) >> 4) * 255.0 / 3.0;
  kolor.b = ((kolor5bit & 0b00001000) >> 3) * 255.0 / 1.0;

  return kolor;
}

Uint8 z24RGBna5BW(Color kolor) {
  int szary8bit = 0.299 * kolor.r + 0.587 * kolor.g + 0.114 * kolor.b;
  int szary5bit = round(szary8bit * 31.0 / 255.0);

  return szary5bit;
}

Color z5BWna24RGB(Uint8 kolor) {
  Uint8 szary8bit = round(kolor * 255.0 / 31.0);

  Color kolor24bit = {szary8bit, szary8bit, szary8bit};
  return kolor24bit;
}

bool porownajKolory(Color kolor1, Color kolor2) {
  return kolor1.r == kolor2.r && kolor1.g == kolor2.g && kolor1.b == kolor2.b;
}

void medianCutBW(int start, int koniec, int iteracja, Canvas1D &obrazek,
                 Canvas1D &paleta) {
  if (iteracja > 0) {
    sort(obrazek.begin() + start, obrazek.begin() + koniec,
         [](Color a, Color b) { return a.r < b.r; });

    int srodek = (start + koniec + 1) / 2;
    medianCutBW(start, srodek - 1, iteracja - 1, obrazek, paleta);
    medianCutBW(srodek, koniec, iteracja - 1, obrazek, paleta);
  } else {

    int sumaBW = 0;
    for (int p = start; p < koniec; p++) {
      sumaBW += obrazek[p].r;
    }
    Uint8 noweBW = sumaBW / (koniec + 1 - start);
    Color nowyKolor = {noweBW, noweBW, noweBW};
    paleta.push_back(nowyKolor);
  }
}

void medianCutRGB(int start, int koniec, int iteracja, Canvas1D &obrazek,
                  Canvas1D &paleta) {

  if (iteracja > 0) {

    SkladowaRGB skladowa = najwiekszaRoznica(start, koniec, obrazek);

    sort(obrazek.begin() + start, obrazek.begin() + koniec,
         [skladowa](Color a, Color b) {
           if (skladowa == R)
             return a.r < b.r;
           if (skladowa == G)
             return a.g < b.g;
           if (skladowa == B)
             return a.b < b.b;
         });

    int srodek = (start + koniec + 1) / 2;
    medianCutRGB(start, srodek - 1, iteracja - 1, obrazek, paleta);
    medianCutRGB(srodek, koniec, iteracja - 1, obrazek, paleta);
  } else {

    int sumaR = 0;
    int sumaG = 0;
    int sumaB = 0;

    for (int p = start; p < koniec; p++) {
      sumaR += obrazek[p].r;
      sumaG += obrazek[p].g;
      sumaB += obrazek[p].b;
    }
    int ilosc = koniec + 1 - start;
    Color nowyKolor = {Uint8(sumaR / ilosc), Uint8(sumaG / ilosc),
                       Uint8(sumaB / ilosc)};
    paleta.push_back(nowyKolor);
  }
}

int znajdzNajblizszyKolorIndex(Color kolor, Canvas1D &paleta) {
  int najblizszyKolor = 0;
  int najmniejszaRoznica = 255;
  for (int j = 0; j < paleta.size(); j++) {
    int roznica = abs(paleta[j].r - kolor.r) + abs(paleta[j].g - kolor.g) +
                  abs(paleta[j].b - kolor.b);
    if (roznica < najmniejszaRoznica) {
      najmniejszaRoznica = roznica;
      najblizszyKolor = j;
    }
  }
  return najblizszyKolor;
}

int znajdzNajblizszyKolorBWIndex(Uint8 szary, Canvas1D &paleta) {
  Color c;
  c.r = szary;
  return znajdzNajblizszyKolorIndex(c, paleta);
}

Color znajdzNajblizszyKolor(Color kolor, Canvas1D &paleta) {
  int najblizszyKolor = 0;
  int najmniejszaRoznica = INT_MAX;
  for (int j = 0; j < paleta.size(); j++) {
    int roznicaR = paleta[j].r - kolor.r;
    int roznicaG = paleta[j].g - kolor.g;
    int roznicaB = paleta[j].b - kolor.b;
    int roznica =
        roznicaR * roznicaR + roznicaG * roznicaG + roznicaB * roznicaB;
    if (roznica < najmniejszaRoznica) {
      najmniejszaRoznica = roznica;
      najblizszyKolor = j;
    }
  }
  return paleta[najblizszyKolor];
}

SkladowaRGB najwiekszaRoznica(int start, int koniec, Canvas1D &obrazek) {
  Color min = {255, 255, 255};
  Color max = {0, 0, 0};

  for (int i = start; i <= koniec; i++) {
    if (obrazek[i].r < min.r)
      min.r = obrazek[i].r;
    if (obrazek[i].g < min.g)
      min.g = obrazek[i].g;
    if (obrazek[i].b < min.b)
      min.b = obrazek[i].b;

    if (obrazek[i].r > max.r)
      max.r = obrazek[i].r;
    if (obrazek[i].g > max.g)
      max.g = obrazek[i].g;
    if (obrazek[i].b > max.b)
      max.b = obrazek[i].b;
  }

  int diffR = max.r - min.r;
  int diffG = max.g - min.g;
  int diffB = max.b - min.b;

  if (diffR >= diffG && diffR >= diffB)
    return R;
  if (diffG >= diffR && diffG >= diffB)
    return G;
  if (diffB >= diffR && diffB >= diffG)
    return B;

  throw std::invalid_argument("Nieznana skladowa RGB");
}

void applyFloydSteinbergDithering(Canvas &image, bool blackWhite) {
  int width = image[0].size();
  int height = image.size();

  for (int y = 0; y < height; y++) {
    for (int x = 0; x < width; x++) {
      Color oldPixel = image[y][x];
      Color newPixel;
      if (blackWhite) {
        newPixel = z5BWna24RGB(z24RGBna5BW(oldPixel));
      } else {
        newPixel = z5RGBna24RGB(z24RGBna5RGB(oldPixel));
      }
      image[y][x] = newPixel;

      int errR = oldPixel.r - newPixel.r;
      int errG = oldPixel.g - newPixel.g;
      int errB = oldPixel.b - newPixel.b;

      if (x + 1 < width) {
        image[y][x + 1].r = normalizacja(image[y][x + 1].r + errR * 7 / 16);
        image[y][x + 1].g = normalizacja(image[y][x + 1].g + errG * 7 / 16);
        image[y][x + 1].b = normalizacja(image[y][x + 1].b + errB * 7 / 16);
      }
      if (x - 1 >= 0 && y + 1 < height) {
        image[y + 1][x - 1].r =
            normalizacja(image[y + 1][x - 1].r + errR * 3 / 16);
        image[y + 1][x - 1].g =
            normalizacja(image[y + 1][x - 1].g + errG * 3 / 16);
        image[y + 1][x - 1].b =
            normalizacja(image[y + 1][x - 1].b + errB * 3 / 16);
      }
      if (y + 1 < height) {
        image[y + 1][x].r = normalizacja(image[y + 1][x].r + errR * 5 / 16);
        image[y + 1][x].g = normalizacja(image[y + 1][x].g + errG * 5 / 16);
        image[y + 1][x].b = normalizacja(image[y + 1][x].b + errB * 5 / 16);
      }
      if (x + 1 < width && y + 1 < height) {
        image[y + 1][x + 1].r = normalizacja(image[y + 1][x + 1].r + errR / 16);
        image[y + 1][x + 1].g = normalizacja(image[y + 1][x + 1].g + errG / 16);
        image[y + 1][x + 1].b = normalizacja(image[y + 1][x + 1].b + errB / 16);
      }
    }
  }
}

void applyBayerDithering(Canvas &image, bool blackWhite) {
  int bayerMatrix[4][4] = {
      {1, 9, 3, 11}, {13, 5, 15, 7}, {4, 12, 2, 10}, {16, 8, 14, 6}};

  int width = image[0].size();
  int height = image.size();

  for (int y = 0; y < height; y++) {
    for (int x = 0; x < width; x++) {
      Color oldPixel = image[y][x];

      oldPixel.r = min(max(0, oldPixel.r + bayerMatrix[y % 4][x % 4] - 8), 255);
      oldPixel.g = min(max(0, oldPixel.g + bayerMatrix[y % 4][x % 4] - 8), 255);
      oldPixel.b = min(max(0, oldPixel.b + bayerMatrix[y % 4][x % 4] - 8), 255);

      Color newPixel;
      if (blackWhite) {
        newPixel = z5BWna24RGB(z24RGBna5BW(oldPixel));
      } else {
        newPixel = z5RGBna24RGB(z24RGBna5RGB(oldPixel));
      }
      image[y][x] = newPixel;
    }
  }
}

void applyFloydSteinbergDithering(Canvas &image, Canvas1D &palette,
                                  bool blackWhite) {
  int width = image[0].size();
  int height = image.size();

  for (int y = 0; y < height; y++) {
    for (int x = 0; x < width; x++) {
      Color oldPixel = image[y][x];
      Color newPixel;

      if (blackWhite) {
        newPixel =
            znajdzNajblizszyKolor(z5BWna24RGB(z24RGBna5BW(oldPixel)), palette);
      } else {
        newPixel = znajdzNajblizszyKolor(oldPixel, palette);
      }

      image[y][x] = newPixel;

      int errR = oldPixel.r - newPixel.r;
      int errG = oldPixel.g - newPixel.g;
      int errB = oldPixel.b - newPixel.b;

      if (x + 1 < width) {
        image[y][x + 1].r = normalizacja(image[y][x + 1].r + errR * 7 / 16);
        image[y][x + 1].g = normalizacja(image[y][x + 1].g + errG * 7 / 16);
        image[y][x + 1].b = normalizacja(image[y][x + 1].b + errB * 7 / 16);
      }
      if (x - 1 >= 0 && y + 1 < height) {
        image[y + 1][x - 1].r =
            normalizacja(image[y + 1][x - 1].r + errR * 3 / 16);
        image[y + 1][x - 1].g =
            normalizacja(image[y + 1][x - 1].g + errG * 3 / 16);
        image[y + 1][x - 1].b =
            normalizacja(image[y + 1][x - 1].b + errB * 3 / 16);
      }
      if (y + 1 < height) {
        image[y + 1][x].r = normalizacja(image[y + 1][x].r + errR * 5 / 16);
        image[y + 1][x].g = normalizacja(image[y + 1][x].g + errG * 5 / 16);
        image[y + 1][x].b = normalizacja(image[y + 1][x].b + errB * 5 / 16);
      }
      if (x + 1 < width && y + 1 < height) {
        image[y + 1][x + 1].r = normalizacja(image[y + 1][x + 1].r + errR / 16);
        image[y + 1][x + 1].g = normalizacja(image[y + 1][x + 1].g + errG / 16);
        image[y + 1][x + 1].b = normalizacja(image[y + 1][x + 1].b + errB / 16);
      }
    }
  }
}

void applyBayerDithering(Canvas &image, Canvas1D &palette) {
  int bayerMatrix[4][4] = {
      {1, 9, 3, 11}, {13, 5, 15, 7}, {4, 12, 2, 10}, {16, 8, 14, 6}};

  int width = image[0].size();
  int height = image.size();

  for (int y = 0; y < height; y++) {
    for (int x = 0; x < width; x++) {
      Color oldPixel = image[y][x];

      oldPixel.r = min(max(0, oldPixel.r + bayerMatrix[y % 4][x % 4] - 8), 255);
      oldPixel.g = min(max(0, oldPixel.g + bayerMatrix[y % 4][x % 4] - 8), 255);
      oldPixel.b = min(max(0, oldPixel.b + bayerMatrix[y % 4][x % 4] - 8), 255);

      Color newPixel = znajdzNajblizszyKolor(oldPixel, palette);
      image[y][x] = newPixel;
    }
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
      applyBayerDithering(obrazek, paleta);
    } else {
      applyBayerDithering(obrazek, tryb == TrybObrazu::SzaroscNarzucona);
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

void ZapisDoPliku(std::string nazwaPliku, TrybObrazu tryb, Dithering dithering,
                  Canvas &obrazek, Canvas1D &paleta) {

  Uint16 szerokoscObrazu = obrazek[0].size();
  Uint16 wysokoscObrazu = obrazek.size();
  const int iloscBitowDoZapisania = 5 * szerokoscObrazu * wysokoscObrazu;

  if (szerokoscObrazu % 8 != 0) {
    throw std::invalid_argument("Szerokosc obrazka nie jest wielokrotnoscia 8");
  }

  cout << "Zapisuje obrazek do pliku..." << endl;

  ofstream wyjscie(nazwaPliku, ios::binary);
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

  vector<bitset<5>> bitset5(iloscBitowDoZapisania / 5);

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
  cout << "Zapisuje obrazek do pliku..." << endl;

  ZapiszCanvasDoBmp(obrazek, bmpCel);
}

Canvas OdczytZPliku(const std::string &filename) {
  std::cout << "Wczytuje obrazek " << filename << " z pliku..." << std::endl;

  ifstream wejscie(filename, ios::binary);
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

  vector<bitset<5>> bitset5(iloscBitowDoOdczytania / 5);

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
  int w = surface->w;
  int h = surface->h;
  if ((x >= 0) && (x < w) && (y >= 0) && (y < h)) {

    char *pPosition = (char *)surface->pixels;

    pPosition += (surface->pitch * y);

    pPosition += (surface->format->BytesPerPixel * x);

    memcpy(&col, pPosition, surface->format->BytesPerPixel);

    SDL_GetRGB(col, surface->format, &color.r, &color.g, &color.b);
  }
  return (color);
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

Canvas1D wyprostujCanvas(Canvas &obrazek) {
  Canvas1D obrazek1D;
  obrazek1D.reserve(obrazek.size() * obrazek[0].size());

  for (const auto r : obrazek) {
    for (const auto c : r) {
      obrazek1D.push_back(c);
    }
  }

  return obrazek1D;
}

typedef std::map<int, std::vector<std::string>> CommandAliasMap;

template <typename T> using ParameterMap = std::map<char, T>;

int findCommand(CommandAliasMap &commandsAliases, std::string command) {
  for (auto &aliases : commandsAliases) {
    for (auto &alias : aliases.second) {
      if (alias == command)
        return aliases.first;
    }
  }
  return 0;
}

void readParameterMap(ParameterMap<std::string> &parameterMap, int offset,
                      int argc, char *argv[]) {
  for (int i = offset; i < argc; i++) {
    if (sizeof(argv[i]) < 2)
      continue;
    if (argv[i][0] == '-' && i + 1 < argc)
      parameterMap[argv[i][1]] = std::string(argv[i + 1]);
  }
}

bool hasParameter(ParameterMap<std::string> &parameterMap, char parameter) {
  return parameterMap.find(parameter) != parameterMap.end();
}

int main(int argc, char *argv[]) {
  CommandAliasMap commandsAliases;

  /* tobmp - odczytuje plik kfc, zapisuje plik bmp */
  commandsAliases[1] = {"tobmp", "-t", "-tobmp"};
  /* frombmp - odczytuje plik bmp, zapisuje plik kfc */
  commandsAliases[2] = {"frombmp", "-f", "-frombmp"};

  const std::string appName = argc < 1 ? "kfc" : argv[0];

  /* Wypisuje wszystkie dostępne komendy bez opisu */
  if (argc <= 1 || (argc == 2 && (std::string(argv[1]) == "help" ||
                                  std::string(argv[1]) == "-help"))) {
    std::cout << "  Witamy w konwerterze obrazów 🍗 KFC <-> 🎨 BMP.\n"
              << "Dostępne operacje:\n"
              << "1. Konwersja formatu KFC na BMP\n"
              << "> " << appName
              << " tobmp <ścieżka_pliku_kfc> [-s ścieżka_pliku_bmp]\n"
              << "Wyświetl więcej informacji używając '" << appName
              << " -help tobmp'\n"
              << "2. Konwersja formatu BMP na KFC\n"
              << "> " << appName
              << " frombmp <ścieżka_pliku_bmp> [-s ścieżka_pliku_kfc] [-t "
                 "tryb(1-5)] [-d dithering(none/bayer/floyd)]\n"
              << "Wyświetl więcej informacji używając '" << appName
              << " -help tobmp'\n";
  }

  /* W przypadku wysłania 'kfc help <command_name>' wyświetlony zostanie opis
     komendy */
  else if (argc == 3 && (std::string(argv[1]) == "help" ||
                         std::string(argv[1]) == "-help")) {
    int primaryCommandId = findCommand(commandsAliases, argv[2]);
    switch (primaryCommandId) {
    case 1: { /* tobmp */
      std::cout << "> " << appName
                << " tobmp <ścieżka_pliku_kfc> [-s ścieżka_pliku_bmp]\n"
                << "Opis: Komenda 'tobmp' konwertuje plik w formacie KFC "
                   "na format BMP \n"
                << "Parametry obowiązkowe:\n"
                << "\t<ścieżka_pliku_kfc> - ścieżka do pliku w formacie "
                   "kfc (relatywna lub absolutna)\n"
                << "Parametry opcjonalne:\n"
                << "\t[-s ścieżka_pliku_bmp] - ścieżka do nowo utworzonego "
                   "pliku (domyślnie plik kfc ze zmienionym "
                   "rozszerzeniem)\n";
      break;
    }
    case 2: { /* frombmp*/
      std::cout << "> " << appName
                << " frombmp <ścieżka_pliku_kfc> [-s ścieżka_pliku_bmp] "
                   "[-t tryb(1-5)] [-d dithering(none/bayer/floyd)]\n"
                << "Opis: Komenda 'frombmp' konwertuje plik w formacie BMP "
                   "na format KFC \n"
                << "Parametry obowiązkowe:\n"
                << "\t<ścieżka_pliku_bmp> - ścieżka do pliku w formacie "
                   "bmp (relatywna lub absolutna)\n"
                << "Parametry opcjonalne:\n"
                << "\t[-s ścieżka_pliku_kfc] - ścieżka do nowo utworzonego "
                   "pliku (domyślnie plik bmp ze zmienionym "
                   "rozszerzeniem)\n"
                << "\t[-t tryb(1-5)] - tryb konwersji obrazu (domyślnie "
                   "1), dostępne tryby:\n"
                << "\t\t1 - Paleta narzucona\n"
                << "\t\t2 - Szarość narzucona\n"
                << "\t\t3 - Paleta wykryta\n"
                << "\t\t4 - Szarość wykryta\n"
                << "\t\t5 - Paleta dedykowana\n"
                << "\t[-d dithering(none/bayer/floyd)] - tryb ditheringu "
                   "(domyślnie none - bez ditheringu)\n";
      break;
    }
    default: {
      std::cout << "Nieznana komenda. Użyj '" << appName
                << " help' aby dowiedzieć się o istniejących komendach."
                << std::endl;
      break;
    }
    }
  }

  /* W pozostałych przypadkach będzie próba rozpoznania komendy z 1 argumentu
     i jej wykonanie */
  else if (argc > 1) {
    int primaryCommandId = findCommand(commandsAliases, argv[1]);

    switch (primaryCommandId) {
    case 1: { /* tobmp <ścieżka_pliku_kfc> [-s ścieżka_pliku_bmp] */
      if (argc < 3) {
        std::cout << "Nie podano ścieżki do pliku kfc. Użyj '" << appName
                  << " help tobmp' aby dowiedzieć się więcej." << std::endl;
        break;
      }
      std::string kfcPath = argv[2];
      ParameterMap<std::string> parameterMap;
      readParameterMap(parameterMap, 3, argc, argv);
      /* parametr s - scieżka pliku kfc */
      std::string bmpPath =
          hasParameter(parameterMap, 's')
              ? parameterMap['s']
              : kfcPath.substr(0, kfcPath.find_last_of('.')) + ".bmp";

      KonwertujKfcNaBmp(kfcPath, bmpPath);
      break;
    }
    case 2: { /* frombmp <ścieżka_pliku_kfc> [-s ścieżka_pliku_bmp] [-t
                 tryb(1-5)] [-d dithering(none/bayer/floyd)] */
      if (argc < 3) {
        std::cout << "Nie podano ścieżki do pliku bmp. Użyj '" << appName
                  << " help frombmp' aby dowiedzieć się więcej." << std::endl;
        break;
      }
      std::string bmpPath = argv[2];
      ParameterMap<std::string> parameterMap;
      readParameterMap(parameterMap, 3, argc, argv);
      /* parametr s - scieżka pliku kfc */
      std::string kfcPath =
          hasParameter(parameterMap, 's')
              ? parameterMap['s']
              : bmpPath.substr(0, bmpPath.find_last_of('.')) + ".kfc";
      /* parametr t - tryb obrazu */
      TrybObrazu tryb = TrybObrazu::PaletaDedykowana;
      if (hasParameter(parameterMap, 't')) {
        int _tryb = std::stoi(parameterMap['t']);
        if (_tryb < 1 || _tryb > 5) {
          std::cout << "Nieprawidłowy tryb konwersji. Użyj '" << appName
                    << " help frombmp' aby dowiedzieć się więcej." << std::endl;
          break;
        }
        tryb = static_cast<TrybObrazu>(_tryb);
      }
      /* parametr d - dithering*/
      Dithering dithering = Dithering::Brak;
      if (hasParameter(parameterMap, 'd')) {
        std::string _dithering = parameterMap['d'];
        if (_dithering == "none")
          dithering = Dithering::Brak;
        else if (_dithering == "bayer")
          dithering = Dithering::Bayer;
        else if (_dithering == "floyd")
          dithering = Dithering::Floyd;
        else {
          std::cout << "Nieprawidłowy tryb ditheringu. Użyj '" << appName
                    << " help frombmp' aby dowiedzieć się więcej." << std::endl;
          break;
        }
      }
      KonwertujBmpNaKfc(bmpPath, kfcPath, tryb, dithering);
      break;
    }
    default: {
      std::cout << "Nieznana komenda. Użyj '" << appName
                << " help' aby dowiedzieć się o dostępnych komendach."
                << std::endl;
      break;
    }
    }
  }
  return 0;
}
