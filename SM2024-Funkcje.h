#ifndef SM2024_FUNKCJE_H
#define SM2024_FUNKCJE_H

#include "SM2024-Zmienne.h"
#include <cmath>
#include <climits>

Uint8 z24RGBna5RGB(Color kolor);
Color z5RGBna24RGB(Uint8 kolor5bit);
Uint8 z24RGBna5BW(Color kolor);
Color z5BWna24RGB(Uint8 kolor);
bool porownajKolory(Color kolor1, Color kolor2);
Uint8 normalizacja(int wartosc);
Canvas1D wyprostujCanvas(Canvas &obrazek);
int getUniqueColorsInImage(const Canvas &image);
int znajdzNajblizszyKolorIndex(Color kolor, Canvas1D &paleta);
Color znajdzNajblizszyKolor(Color kolor, Canvas1D &paleta);
void applyFloydSteinbergDithering(Canvas &image, bool blackWhite);
void applyBayerDithering(Canvas &image, bool blackWhite);
void applyFloydSteinbergDithering(Canvas &image, Canvas1D &palette, bool blackWhite);
void applyBayerDithering(Canvas &image, Canvas1D &palette);

#endif // SM2024_FUNKCJE_H
