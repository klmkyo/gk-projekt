#pragma once
#include "SM2024-Zmienne.h"
#include <algorithm>
#include <stdexcept>

void medianCutBW(int start, int koniec, int iteracja, Canvas1D &obrazek, Canvas1D &paleta);
void medianCutRGB(int start, int koniec, int iteracja, Canvas1D &obrazek, Canvas1D &paleta);
SkladowaRGB najwiekszaRoznica(int start, int koniec, Canvas1D &obrazek);
