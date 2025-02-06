#include "SM2024-Zmienne.h"

const char FILE_SIGNATURE[2] = {0x33, 0x33};

bool czyTrybJestZPaleta(TrybObrazu tryb) { 
    return tryb >= 3; 
}

SDL_Window* window = NULL;
SDL_Surface* screen = NULL;

SDL_Color paleta8[szerokosc*wysokosc];

bool operator==(const Color &lhs, const Color &rhs) {
    return lhs.r == rhs.r && lhs.g == rhs.g && lhs.b == rhs.b;
}

bool operator!=(const Color &lhs, const Color &rhs) {
    return !(lhs == rhs);
}


