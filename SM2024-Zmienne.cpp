#include "SM2024-Zmienne.h"

const char FILE_SIGNATURE[2] = {0x33, 0x33};

bool czyTrybJestZPaleta(TrybObrazu tryb) { 
    return tryb >= 3; 
}

// Removed the template specializations of hash<Color> and equal_to<Color>
