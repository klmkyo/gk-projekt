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
