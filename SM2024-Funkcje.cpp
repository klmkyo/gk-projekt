#include "SM2024-Funkcje.h"
#include <algorithm>

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

Uint8 normalizacja(int wartosc) {
    if (wartosc < 0) return 0;
    if (wartosc > 255) return 255;
    return wartosc;
}

Canvas1D wyprostujCanvas(Canvas &obrazek) {
    Canvas1D obrazek1D;
    obrazek1D.reserve(obrazek.size() * obrazek[0].size());

    for (const auto &r : obrazek) {
        for (const auto &c : r) {
            obrazek1D.push_back(c);
        }
    }

    return obrazek1D;
}

int getUniqueColorsInImage(const Canvas &image) {
    std::unordered_set<Color> colors;
    for (const auto &row : image) {
        for (const auto &pixel : row) {
            colors.insert(pixel);
        }
    }
    return colors.size();
}

int znajdzNajblizszyKolorIndex(Color kolor, Canvas1D &paleta) {
    int najblizszyKolor = 0;
    int najmniejszaRoznica = INT_MAX;
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

Color znajdzNajblizszyKolor(Color kolor, Canvas1D &paleta) {
    int najblizszyKolor = 0;
    int najmniejszaRoznica = INT_MAX;
    for (int j = 0; j < paleta.size(); j++) {
        int roznicaR = paleta[j].r - kolor.r;
        int roznicaG = paleta[j].g - kolor.g;
        int roznicaB = paleta[j].b - kolor.b;
        int roznica = roznicaR * roznicaR + roznicaG * roznicaG + roznicaB * roznicaB;
        if (roznica < najmniejszaRoznica) {
            najmniejszaRoznica = roznica;
            najblizszyKolor = j;
        }
    }
    return paleta[najblizszyKolor];
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
                image[y + 1][x - 1].r = normalizacja(image[y + 1][x - 1].r + errR * 3 / 16);
                image[y + 1][x - 1].g = normalizacja(image[y + 1][x - 1].g + errG * 3 / 16);
                image[y + 1][x - 1].b = normalizacja(image[y + 1][x - 1].b + errB * 3 / 16);
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
        {1,  9,  3, 11},
        {13, 5, 15, 7},
        {4, 12, 2, 10},
        {16, 8, 14, 6}
    };

    int width = image[0].size();
    int height = image.size();

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            Color oldPixel = image[y][x];

            oldPixel.r = std::min(std::max(0, oldPixel.r + bayerMatrix[y % 4][x % 4] - 8), 255);
            oldPixel.g = std::min(std::max(0, oldPixel.g + bayerMatrix[y % 4][x % 4] - 8), 255);
            oldPixel.b = std::min(std::max(0, oldPixel.b + bayerMatrix[y % 4][x % 4] - 8), 255);

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

void applyFloydSteinbergDithering(Canvas &image, Canvas1D &palette, bool blackWhite) {
    int width = image[0].size();
    int height = image.size();

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            Color oldPixel = image[y][x];
            Color newPixel;
            
            if (blackWhite) {
                newPixel = znajdzNajblizszyKolor(z5BWna24RGB(z24RGBna5BW(oldPixel)), palette);
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
                image[y + 1][x - 1].r = normalizacja(image[y + 1][x - 1].r + errR * 3 / 16);
                image[y + 1][x - 1].g = normalizacja(image[y + 1][x - 1].g + errG * 3 / 16);
                image[y + 1][x - 1].b = normalizacja(image[y + 1][x - 1].b + errB * 3 / 16);
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
        {1,  9,  3, 11},
        {13, 5, 15, 7},
        {4, 12, 2, 10},
        {16, 8, 14, 6}
    };

    int width = image[0].size();
    int height = image.size();

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            Color oldPixel = image[y][x];

            oldPixel.r = std::min(std::max(0, oldPixel.r + bayerMatrix[y % 4][x % 4] - 8), 255);
            oldPixel.g = std::min(std::max(0, oldPixel.g + bayerMatrix[y % 4][x % 4] - 8), 255);
            oldPixel.b = std::min(std::max(0, oldPixel.b + bayerMatrix[y % 4][x % 4] - 8), 255);

            Color newPixel = znajdzNajblizszyKolor(oldPixel, palette);
            image[y][x] = newPixel;
        }
    }
}
