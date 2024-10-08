#ifndef SM2024_ZMIENNE_H
#define SM2024_ZMIENNE_H

#include <vector>
#include <unordered_set>
#include "SDL_surface.h"
#include <bitset>
#include <string>

#define szerokosc 640
#define wysokosc 400
#define PALETA_SIZE 32
#define OBRAZEK_SIZE 64000

struct Color {
    Uint8 r, g, b;
};

extern const char FILE_SIGNATURE[2];

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

bool czyTrybJestZPaleta(TrybObrazu tryb);

// Move the hash and equal_to definitions here, fully implementing them
namespace std {
    template <>
    struct hash<Color> {
        size_t operator()(const Color &k) const {
            return ((hash<Uint8>()(k.r) ^ (hash<Uint8>()(k.g) << 1)) >> 1) ^
                   (hash<Uint8>()(k.b) << 1);
        }
    };

    template <>
    struct equal_to<Color> {
        bool operator()(const Color &lhs, const Color &rhs) const {
            return lhs.r == rhs.r && lhs.g == rhs.g && lhs.b == rhs.b;
        }
    };
}

#endif // SM2024_ZMIENNE_H
