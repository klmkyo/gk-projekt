#include "SM2024-Zmienne.h"

const char FILE_SIGNATURE[2] = {0x33, 0x33};

bool czyTrybJestZPaleta(TrybObrazu tryb) { 
    return tryb >= 3; 
}

SDL_Window* window = NULL;
SDL_Surface* screen = NULL;

SDL_Color paleta8[szerokosc*wysokosc];

// Removed the template specializations of hash<Color> and equal_to<Color>
