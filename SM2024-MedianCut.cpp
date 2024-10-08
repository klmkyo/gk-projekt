#include "SM2024-MedianCut.h"

SkladowaRGB najwiekszaRoznica(int start, int koniec, Canvas1D &obrazek) {
    Color min = {255, 255, 255};
    Color max = {0, 0, 0};

    for (int i = start; i <= koniec; i++) {
        if (obrazek[i].r < min.r) min.r = obrazek[i].r;
        if (obrazek[i].g < min.g) min.g = obrazek[i].g;
        if (obrazek[i].b < min.b) min.b = obrazek[i].b;

        if (obrazek[i].r > max.r) max.r = obrazek[i].r;
        if (obrazek[i].g > max.g) max.g = obrazek[i].g;
        if (obrazek[i].b > max.b) max.b = obrazek[i].b;
    }

    int diffR = max.r - min.r;
    int diffG = max.g - min.g;
    int diffB = max.b - min.b;

    if (diffR >= diffG && diffR >= diffB) return R;
    if (diffG >= diffR && diffG >= diffB) return G;
    if (diffB >= diffR && diffB >= diffG) return B;

    throw std::invalid_argument("Nieznana skladowa RGB");
}

void medianCutBW(int start, int koniec, int iteracja, Canvas1D &obrazek,
                 Canvas1D &paleta) {
    if (iteracja > 0) {
        std::sort(obrazek.begin() + start, obrazek.begin() + koniec,
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

        std::sort(obrazek.begin() + start, obrazek.begin() + koniec,
                 [skladowa](Color a, Color b) {
                     if (skladowa == R) return a.r < b.r;
                     if (skladowa == G) return a.g < b.g;
                     if (skladowa == B) return a.b < b.b;
                     return false; // Should not reach here
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
