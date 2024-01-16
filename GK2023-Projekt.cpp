#include <SDL2/SDL.h>
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <algorithm>
#include <bitset>
#include <exception>
#include <fstream>
#include <iostream>
#include <vector>
using namespace std;

SDL_Window *window = NULL;
SDL_Surface *screen = NULL;

#define szerokosc 640
#define wysokosc 400

#define szerokoscObrazka (szerokosc / 2)
#define wysokoscObrazka (wysokosc / 2)

#define tytul "GK2023 - Projekt - Zespol 33"

typedef std::vector<std::vector<SDL_Color>> Canvas;

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

bool czyTrybJestZPaleta(TrybObrazu tryb) { return tryb >= 3; }

enum Dithering { Brak = 0, Bayer = 1, Floyd = 2 };

SkladowaRGB najwiekszaRoznica(int start, int koniec);

void setPixel(int x, int y, Uint8 R, Uint8 G, Uint8 B);
SDL_Color getPixel(int x, int y);

Uint8 z24RGBna8RGB(SDL_Color kolor);
SDL_Color z8RGBna24RGB(Uint8 kolor8bit);

Uint8 z24RGBna5RGB(SDL_Color kolor);
SDL_Color z5RGBna24RGB(Uint8 kolor5bit);

void czyscEkran(Uint8 R, Uint8 G, Uint8 B);

Uint8 normalizacja(int wartosc) {
    if (wartosc < 0) return 0;
    if (wartosc > 255) return 255;
    return wartosc;
}

void Funkcja1();
void Funkcja2();
void Funkcja3();
void Funkcja4();
void Funkcja5();
void Funkcja6();
void Funkcja7();
void Funkcja8();
void Funkcja9();
void FunkcjaQ();
void FunkcjaW();
void FunkcjaE();
void FunkcjaR();
void FunkcjaT();

void ladujBMPDoPamieci(char const *nazwa, Canvas &obrazek);
bool porownajKolory(SDL_Color kolor1, SDL_Color kolor2);

#define OBRAZEK_SIZE 64000

SDL_Color obrazek[OBRAZEK_SIZE];
SDL_Color obrazekT[OBRAZEK_SIZE];
SDL_Color paleta[OBRAZEK_SIZE];
constexpr int maxKolorow = 320 * 600;
SDL_Color paleta8[maxKolorow];
int ileKubelkow = 0;

Uint8 z24RGBna8RGB(SDL_Color kolor) {
    Uint8 nowyR, nowyG, nowyB;
    nowyR = round(kolor.r * 7.0 / 255.0);
    nowyG = round(kolor.g * 7.0 / 255.0);
    nowyB = round(kolor.b * 3.0 / 255.0);

    return (nowyR << 5) | (nowyG << 2) | (nowyB);
}

SDL_Color z8RGBna24RGB(Uint8 kolor8bit) {
    SDL_Color kolor;
    kolor.r = kolor8bit & 0b11100000;
    kolor.g = (kolor8bit & 0b00011100) << 3;
    kolor.b = (kolor8bit & 0b00000011) << 6;
    return kolor;
}

Uint8 z24RGBna5RGB(SDL_Color kolor) {
    Uint8 nowyR, nowyG, nowyB;
    nowyR = round(kolor.r * 3.0 / 255.0);
    nowyG = round(kolor.g * 3.0 / 255.0);
    nowyB = round(kolor.b * 1.0 / 255.0);

    return (nowyR << 6) | (nowyG << 4) | (nowyB << 3);
}

SDL_Color z5RGBna24RGB(Uint8 kolor5bit) {
    SDL_Color kolor;
    kolor.r = ((kolor5bit & 0b11000000) >> 6) * 255.0 / 3.0;
    kolor.g = ((kolor5bit & 0b00110000) >> 4) * 255.0 / 3.0;
    kolor.b = ((kolor5bit & 0b00001000) >> 3) * 255.0 / 1.0;

    return kolor;
}

Uint8 z24RGBna5BW(SDL_Color kolor) {
    int szary8bit = 0.299 * kolor.r + 0.587 * kolor.g + 0.114 * kolor.b;
    int szary5bit = round(szary8bit * 31.0 / 255.0);

    return szary5bit;
}

SDL_Color z5BWna24RGB(Uint8 kolor) {
    Uint8 szary8bit = round(kolor * 255.0 / 31.0);

    SDL_Color kolor24bit = {szary8bit, szary8bit, szary8bit};
    return kolor24bit;
}

void Funkcja1() {
    SDL_Color kolor;
    uint8_t R, G, B, nowyR, nowyG, nowyB;
    for (int y = 0; y < wysokosc / 2; y++) {
        for (int x = 0; x < szerokosc / 2; x++) {
            kolor = getPixel(x, y);
            R = kolor.r;
            G = kolor.g;
            B = kolor.b;

            // 111 111 11
            nowyR = R >> 5;
            nowyG = G >> 5;
            nowyB = B >> 6;

            R = nowyR << 5;
            G = nowyG << 5;
            B = nowyB << 6;
            setPixel(x + szerokosc / 2, y, R, G, B);
        }
    }

    SDL_UpdateWindowSurface(window);
}

void Funkcja2() {
    SDL_Color kolor;
    uint8_t R, G, B, nowyR, nowyG, nowyB;
    for (int x = 0; x < szerokosc / 2; x++) {
        for (int y = 0; y < wysokosc / 2; y++) {
            kolor = getPixel(x, y);
            R = kolor.r;
            G = kolor.g;
            B = kolor.b;

            // 111 111 11
            nowyR = R >> 5;
            nowyG = G >> 5;
            nowyB = B >> 6;

            R = nowyR * 255.0 / 7.0;
            G = nowyG * 255.0 / 7.0;
            B = nowyB * 255.0 / 3.0;
            setPixel(x, y + wysokosc / 2, R, G, B);
        }
    }

    SDL_UpdateWindowSurface(window);
}

void Funkcja3() {
    SDL_Color kolor;
    uint8_t R, G, B, nowyR, nowyG, nowyB;
    int kolor8bit;
    for (int x = 0; x < szerokosc / 2; x++) {
        for (int y = 0; y < wysokosc / 2; y++) {
            kolor = getPixel(x, y);
            R = kolor.r;
            G = kolor.g;
            B = kolor.b;

            // 111 111 11
            nowyR = round(R * 7.0 / 255.0);
            nowyG = round(G * 7.0 / 255.0);
            nowyB = round(B * 3.0 / 255.0);

            kolor8bit = (nowyR << 5) | (nowyG << 2) | (nowyB);
            cout << kolor8bit << endl;

            R = nowyR * 255.0 / 7.0;
            G = nowyG * 255.0 / 7.0;
            B = nowyB * 255.0 / 3.0;
            setPixel(x + szerokosc / 2, y + wysokosc / 2, R, G, B);
        }
    }

    SDL_UpdateWindowSurface(window);
}

void Funkcja4() {
    Uint8 kolor8bit;
    SDL_Color kolor, nowyKolor;

    for (int x = 0; x < szerokosc / 2; x++) {
        for (int y = 0; y < wysokosc / 2; y++) {
            kolor = getPixel(x, y);
            kolor8bit = z24RGBna5RGB(kolor);
            nowyKolor = z5RGBna24RGB(kolor8bit);
            setPixel(x + szerokosc / 2, y, nowyKolor.r, nowyKolor.g,
                     nowyKolor.b);
        }
    }

    SDL_UpdateWindowSurface(window);
}

void Funkcja5() {
    SDL_Color kolor;
    kolor.r = 165;
    kolor.g = 210;
    kolor.b = 96;
    cout << bitset<8>(kolor.r) << " " << bitset<8>(kolor.g) << " "
         << bitset<8>(kolor.b) << endl;
    Uint8 kolor8bit = z24RGBna8RGB(kolor);
    cout << bitset<8>(kolor8bit) << endl;
    SDL_Color kolor2 = z8RGBna24RGB(kolor8bit);
    cout << bitset<8>(kolor2.r) << " " << bitset<8>(kolor2.g) << " "
         << bitset<8>(kolor2.b) << endl;

    SDL_UpdateWindowSurface(window);
}

void Funkcja6() {
    // zamienianie z 24bit na 3bit i z powrotem (wyświetlanie na rożnych
    // kwadrantach)
    for (int y = 0; y < wysokosc / 2; y++) {
        for (int x = 0; x < szerokosc / 2; x++) {
            SDL_Color kolor = getPixel(x, y);

            Uint8 szary5bit = z24RGBna5BW(kolor);
            SDL_Color kolor24bit = z5BWna24RGB(szary5bit);

            setPixel(x + szerokosc / 2, y, kolor24bit.r, kolor24bit.g,
                     kolor24bit.b);
        }
    }

    SDL_UpdateWindowSurface(window);
}

void Funkcja7() {
    Uint8 przesuniecie = 1;

    float bledy[(szerokosc / 2) + 2][wysokosc / 2 + 2];
    memset(bledy, 0, sizeof(bledy));

    for (int yy = 0; yy < wysokosc / 2; yy++) {
        for (int xx = 0; xx < szerokosc / 2; xx++) {
            SDL_Color kolor = getPixel(xx, yy);

            Uint8 szaryOrg =
                0.299 * kolor.r + 0.587 * kolor.g + 0.114 * kolor.b;

            Uint8 szaryZBledem =
                normalizacja(szaryOrg + bledy[xx + przesuniecie][yy]);

            SDL_Color tempColor =
                SDL_Color{szaryZBledem, szaryZBledem, szaryZBledem};
            Uint8 szary5bit = z24RGBna5BW(tempColor);
            SDL_Color nowyKolor = z5BWna24RGB(szary5bit);

            int blad = szaryOrg - nowyKolor.r;

            setPixel(xx + szerokosc / 2, yy, nowyKolor.r, nowyKolor.g,
                     nowyKolor.b);

            bledy[xx + 1 + przesuniecie][yy] += (blad * 7.0 / 16.0);
            bledy[xx - 1 + przesuniecie][yy + 1] += (blad * 3.0 / 16.0);
            bledy[xx + przesuniecie][yy + 1] += (blad * 5.0 / 16.0);
            bledy[xx + 1 + przesuniecie][yy + 1] += (blad * 1.0 / 16.0);
        }
    }

    SDL_UpdateWindowSurface(window);
}

void updateBledy(int xx, int yy, float (*bledy)[wysokosc / 2 + 2][3], int blad,
                 int colorIndex, int przesuniecie) {
    bledy[xx + 1 + przesuniecie][yy][colorIndex] += (blad * 7.0 / 16.0);
    bledy[xx - 1 + przesuniecie][yy + 1][colorIndex] += (blad * 3.0 / 16.0);
    bledy[xx + przesuniecie][yy + 1][colorIndex] += (blad * 5.0 / 16.0);
    bledy[xx + 1 + przesuniecie][yy + 1][colorIndex] += (blad * 1.0 / 16.0);
}

void Funkcja8() {
    Uint8 przesuniecie = 1;

    float bledy[(szerokosc / 2) + 2][wysokosc / 2 + 2][3];
    memset(bledy, 0, sizeof(bledy));

    for (int yy = 0; yy < wysokosc / 2; yy++) {
        for (int xx = 0; xx < szerokosc / 2; xx++) {
            SDL_Color kolor = getPixel(xx, yy);

            Uint8 rZBledem =
                normalizacja(kolor.r + bledy[xx + przesuniecie][yy][0]);
            Uint8 gZBledem =
                normalizacja(kolor.g + bledy[xx + przesuniecie][yy][1]);
            Uint8 bZBledem =
                normalizacja(kolor.b + bledy[xx + przesuniecie][yy][2]);

            SDL_Color tempColor = SDL_Color{rZBledem, gZBledem, bZBledem};
            Uint8 kolor8bit = z24RGBna8RGB(tempColor);
            SDL_Color nowyKolor = z8RGBna24RGB(kolor8bit);

            int rBlad = rZBledem - nowyKolor.r;
            int gBlad = gZBledem - nowyKolor.g;
            int bBlad = bZBledem - nowyKolor.b;

            setPixel(xx + szerokosc / 2, yy, nowyKolor.r, nowyKolor.g,
                     nowyKolor.b);

            updateBledy(xx, yy, bledy, rBlad, 0, przesuniecie);
            updateBledy(xx, yy, bledy, gBlad, 1, przesuniecie);
            updateBledy(xx, yy, bledy, bBlad, 2, przesuniecie);
        }
    }

    SDL_UpdateWindowSurface(window);
}

bool porownajKolory(SDL_Color kolor1, SDL_Color kolor2) {
    return kolor1.r == kolor2.r && kolor1.g == kolor2.g && kolor1.b == kolor2.b;
}

int dodajKolor(SDL_Color kolor) {
    int aktualnyKolor = ileKubelkow;
    if (ileKubelkow < maxKolorow) {
        paleta[ileKubelkow] = kolor;
    }
    // cout << aktualnyKolor << ": " << (int)kolor.r << " " << (int)kolor.g << "
    // " << (int)kolor.b << endl;
    ileKubelkow++;
    return aktualnyKolor;
}

int sprawdzKolor(SDL_Color kolor) {
    if (ileKubelkow > 0) {
        for (int k = 0; k < ileKubelkow; k++) {
            if (porownajKolory(kolor, paleta[k])) return k;
        }
    }

    return dodajKolor(kolor);
}

void Funkcja9() {

    cout << endl << "ile kolorow: " << ileKubelkow << endl;
    if (ileKubelkow <= maxKolorow)
        cout << "Paleta spelnia ograniczenia 8-bit/piksel" << endl;
    else
        cout << "Paleta przekracza ograniczenia 8-bit/piksel" << endl;

    const int gridRows = std::sqrt(ileKubelkow);
    const int gridCols = (ileKubelkow + gridRows - 1) / gridRows;  // Ceiling division
    const int cellWidth = round((szerokosc / 2) / (float)gridCols);
    const int cellHeight = round((wysokosc / 2) / (float)gridRows);

    cout << "gridRows: " << gridRows << " gridCols: " << gridCols
         << " cellWidth: " << cellWidth << " cellHeight: " << cellHeight
         << endl;

    for (int i = 0; i < ileKubelkow; i++) {
        int row = i / gridCols;
        int col = i % gridCols;
        SDL_Color color = paleta[i];

        int startY = row * cellHeight;
        int endY = std::min((row + 1) * cellHeight, wysokosc / 2);
        int startX = (szerokosc / 2) + col * cellWidth;
        int endX = std::min((szerokosc / 2) + (col + 1) * cellWidth, szerokosc);

        for (int y = startY; y < endY; y++) {
            for (int x = startX; x < endX; x++) {
                setPixel(x, y, color.r, color.g, color.b);
            }
        }
    }

    SDL_UpdateWindowSurface(window);
}

void losujWartosci() {
    Uint8 wartosc;
    for (int i = 0; i < OBRAZEK_SIZE; i++) {
        wartosc = rand() % OBRAZEK_SIZE;
        obrazek[i] = {wartosc, wartosc, wartosc};
        obrazekT[i] = {wartosc, wartosc, wartosc};
    }

    cout << endl;
}

void wyswietlWartosci() {
    for (int i = 0; i < OBRAZEK_SIZE; i++) {
        cout << (int)obrazek[i].r << " ";
    }
    cout << endl;
}

void sortujKubelekKFC(int start, int koniec) {
    sort(obrazek + start, obrazek + koniec,
         [](SDL_Color a, SDL_Color b) { return a.r < b.r; });
}

void sortujKubelekKFCKolor(int start, int koniec, SkladowaRGB skladowa) {
    // sort by rgb using skladowa parameter
    sort(obrazek + start, obrazek + koniec,
         [skladowa](SDL_Color a, SDL_Color b) {
             if (skladowa == R) return a.r < b.r;
             if (skladowa == G) return a.g < b.g;
             if (skladowa == B) return a.b < b.b;
         });
}

void medianCut(int start, int koniec, int iteracja) {
    cout << "start: " << start << ", koniec: " << koniec
         << ", iteracja: " << iteracja << endl;
    if (iteracja > 0) {
        // sortowanie wtorkowego kubełka kfc za 22 zł
        sortujKubelekKFC(start, koniec);

        cout << "Dzielenie kubełka KFC na poziomie " << iteracja << endl;

        int srodek = (start + koniec + 1) / 2;
        medianCut(start, srodek - 1, iteracja - 1);
        medianCut(srodek, koniec, iteracja - 1);
    } else {
        // budowanie palety uśredniając kolory z określonego kubełka KFC
        int sumaBW = 0;
        for (int p = start; p < koniec; p++) {
            sumaBW += obrazek[p].r;
        }
        Uint8 noweBW = sumaBW / (koniec + 1 - start);
        SDL_Color nowyKolor = {noweBW, noweBW, noweBW};
        paleta[ileKubelkow] = nowyKolor;

        printf("\n");
        cout << "🍿 Kubełek " << ileKubelkow << "(" << start << "," << koniec
             << ") = 🍗 " << (int)noweBW << endl;
        cout << "🎨 Kolor " << ileKubelkow << ": " << (int)nowyKolor.r << " "
             << (int)nowyKolor.g << " " << (int)nowyKolor.b << endl;

        ileKubelkow++;
    }
}

void medianCutRGB(int start, int koniec, int iteracja) {
    cout << "start: " << start << ", koniec: " << koniec
         << ", iteracja: " << iteracja << endl;
    if (iteracja > 0) {
        // sortowanie wtorkowego kubełka kfc za 22 zł
        SkladowaRGB skladowa = najwiekszaRoznica(start, koniec);
        sortujKubelekKFCKolor(start, koniec, skladowa);

        cout << "Dzielenie kubełka KFC na poziomie " << iteracja << endl;

        int srodek = (start + koniec + 1) / 2;
        medianCutRGB(start, srodek - 1, iteracja - 1);
        medianCutRGB(srodek, koniec, iteracja - 1);
    } else {
        // budowanie palety uśredniając kolory z określonego kubełka KFC
        int sumaR = 0;
        int sumaG = 0;
        int sumaB = 0;

        for (int p = start; p < koniec; p++) {
            sumaR += obrazek[p].r;
            sumaG += obrazek[p].g;
            sumaB += obrazek[p].b;
        }
        int ilosc = koniec + 1 - start;
        SDL_Color nowyKolor = {Uint8(sumaR / ilosc), Uint8(sumaG / ilosc),
                               Uint8(sumaB / ilosc)};
        paleta[ileKubelkow] = nowyKolor;

        printf("\n");
        cout << "🍿 Kubełek / 🎨 Kolor " << ileKubelkow << ": "
             << (int)nowyKolor.r << " " << (int)nowyKolor.g << " "
             << (int)nowyKolor.b << endl;

        ileKubelkow++;
    }
}

int znajdzNajblizszyKolorIndex(SDL_Color kolor) {
    int najblizszyKolor = 0;
    int najmniejszaRoznica = 255;
    for (int j = 0; j < ileKubelkow; j++) {
        int roznica = abs(paleta[j].r - kolor.r) + abs(paleta[j].g - kolor.g) +
                      abs(paleta[j].b - kolor.b);
        if (roznica < najmniejszaRoznica) {
            najmniejszaRoznica = roznica;
            najblizszyKolor = j;
        }
    }
    return najblizszyKolor;
}

int znajdzNajblizszyKolorIndex(Uint8 szary) {
    SDL_Color c;
    c.r = szary;
    return znajdzNajblizszyKolorIndex(c);
}

SDL_Color znajdzNajblizszyKolor(SDL_Color kolor) {
    int najblizszyKolor = 0;
    int najmniejszaRoznica = 255;
    for (int j = 0; j < ileKubelkow; j++) {
        int roznica = abs(paleta[j].r - kolor.r);
        if (roznica < najmniejszaRoznica) {
            najmniejszaRoznica = roznica;
            najblizszyKolor = j;
        }
    }
    return paleta[najblizszyKolor];
}

// in obrazek[start..koniec], find the color with highest difference
SkladowaRGB najwiekszaRoznica(int start, int koniec) {
    SDL_Color min = {255, 255, 255};
    SDL_Color max = {0, 0, 0};

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

    printf("wtorkowe kubelki wyprzedaly sie :(");
    throw std::exception();
}

/* Note: The comments and some variable names are in Polish,
and there are some humorous references to KFC buckets.
The actual functionality of the code doesn't
have anything to do with KFC or chicken.*/
// Copilot, 2023
// https://www.youtube.com/watch?v=5e9tj9eqgs0

void FunkcjaQ() {
    SDL_UpdateWindowSurface(window);
    losujWartosci();
    wyswietlWartosci();
    medianCut(0, 255, 2);
    for (int i = 0; i < OBRAZEK_SIZE; i++) {
        obrazek[i] = znajdzNajblizszyKolor(obrazekT[i]);
        cout << znajdzNajblizszyKolorIndex(obrazekT[i]) << " ";
        // setPixel(i, wysokosc / 2, obrazek[i].r, obrazek[i].g,
        //          obrazek[i].b);
    }

    wyswietlWartosci();
}

void FunkcjaW() {
    SDL_Color kolor;
    Uint8 szary;
    int numer = 0, indeks = 0;
    for (int y = 0; y < wysokosc / 2; y++) {
        for (int x = 0; x < szerokosc / 2; x++) {
            kolor = getPixel(x, y);
            szary = 0.299 * kolor.r + 0.587 * kolor.g + 0.114 * kolor.b;
            obrazek[numer] = {szary, szary, szary};
            setPixel(x + szerokosc / 2, y, szary, szary, szary);
            numer++;
        }
    }
    SDL_UpdateWindowSurface(window);
    medianCut(0, numer - 1, 2);

    for (int y = 0; y < wysokosc / 2; y++) {
        for (int x = 0; x < szerokosc / 2; x++) {
            kolor = getPixel(x, y);
            szary = 0.299 * kolor.r + 0.587 * kolor.g + 0.114 * kolor.b;
            indeks = znajdzNajblizszyKolorIndex(szary);

            setPixel(x + szerokosc / 2, y + wysokosc / 2, paleta[indeks].r,
                     paleta[indeks].g, paleta[indeks].b);
        }
    }

    SDL_UpdateWindowSurface(window);
}

void FunkcjaE() {
    SDL_Color kolor;
    SDL_Color nowyKolor;
    int numer = 0, indeks = 0;
    for (int y = 0; y < wysokosc / 2; y++) {
        for (int x = 0; x < szerokosc / 2; x++) {
            kolor = getPixel(x, y);
            obrazek[numer] = kolor;
            numer++;
        }
    }
    medianCutRGB(0, numer - 1, 5);

    for (int y = 0; y < wysokosc / 2; y++) {
        for (int x = 0; x < szerokosc / 2; x++) {
            kolor = getPixel(x, y);
            indeks = znajdzNajblizszyKolorIndex(kolor);

            cout << "Dla " << x << ", " << y << " wybrano kolor index "
                 << indeks << endl;

            setPixel(x + szerokosc / 2, y + wysokosc / 2, paleta[indeks].r,
                     paleta[indeks].g, paleta[indeks].b);
        }
    }

    SDL_UpdateWindowSurface(window);
}

// pakowanie bitowe (dla 2 pierwszych trybow chyba, reszta paleta)

void ZapisDoPliku(TrybObrazu tryb, Dithering dithering, Canvas &obrazek);

void FunkcjaR() {
    Canvas obrazek(wysokoscObrazka, std::vector<SDL_Color>(szerokoscObrazka));

    ladujBMPDoPamieci("obrazek1.bmp", obrazek);
    ZapisDoPliku(TrybObrazu::PaletaNarzucona, Dithering::Brak, obrazek);
}

/// takes a path to bmp file, and creates a converted version of it
/// abc.bmp -> abc.kfc
void KonwertujBmpNaKfc(const char *bmpZrodlo) {
    Canvas obrazek(wysokoscObrazka, std::vector<SDL_Color>(szerokoscObrazka));

    ladujBMPDoPamieci(bmpZrodlo, obrazek);

    // dithering itd
    // tutaj powstaje paleta

    // podaje obrazek + palete
    ZapisDoPliku(TrybObrazu::PaletaNarzucona, Dithering::Brak, obrazek);
}

// jest jakieś dzielenie na bloki? funkcja tworząca i odczytująca tablicę

// idzie się od lewej do prawej, ale czytając tylko po 8 pikseli z każdej
// kolumny
// (0, 0), (0, 1), (0, 2), (0, 3), (0, 4), (0, 5), (0, 6), (0, 7)
// (1, 0), (1, 1), (1, 2), (1, 3), (1, 4), (1, 5), (1, 6), (1, 7)
// ...
// (n, 0), (n, 1), (n, 2), (n, 3), (n, 4), (n, 5), (n, 6), (n, 7)
// i następnie odczytuje się piksele, tym razem zaczynając odczyt kolumny z
// offsetem 8 pixeli (zaczynające się od (0, 8)), ale zapisuje się tak samo (bez
// offsetu jakoś to pomaga)

/**
 * @brief Zapisuje Canvas do pliku
 * @param tryb Tryb w jakim obraz zostanie zapisany
 * @param dithering Informacja o tym, z jakim ditheringiem jest podany Canvas
 * @param obrazek Canvas, który zostanie zapisany do pliku
 */
void ZapisDoPliku(TrybObrazu tryb, Dithering dithering, Canvas &obrazek) {
    Uint16 szerokoscObrazu = szerokosc / 2;
    Uint16 wysokoscObrazu = wysokosc / 2;
    cout << "Zapisuje obrazek do pliku" << endl;

    // Data założenia KFC - September 24, 1952; 71 years ago
    char id[2] = {0x19, 0x52};

    ofstream wyjscie("obraz.z33", ios::binary);
    wyjscie.write((char *)&id, sizeof(char) * 2);
    wyjscie.write((char *)&szerokoscObrazu, sizeof(char) * 2);
    wyjscie.write((char *)&wysokoscObrazu, sizeof(char) * 2);
    wyjscie.write((char *)&tryb, sizeof(Uint8));

    // 1, 2 - tryby bez palety (zapisywanie pikseli z pakowaniem bitowym)
    // 3, 4, 5 - tryby z paletą
    if (czyTrybJestZPaleta(tryb)) {
        wyjscie.write((char *)&paleta, sizeof(paleta) / sizeof(paleta[0]));
        // yyyy nie wiem czy to na dole powinno byc
    } else {
        wyjscie.write((char *)&dithering, sizeof(Uint8));
    }

    if (czyTrybJestZPaleta(tryb)) {
        int iloscBitowDoZapisania = 5 * szerokoscObrazka * wysokoscObrazka;

        vector<bitset<5>> bitset5(iloscBitowDoZapisania);


        // Nowa wersja - zapisuje po kolumnach ale max 8 rzędów
        // i potem przechodzi 8 rzedów niżej i znowu wszystkie kolumny itd......
        int maxSteps = wysokoscObrazka / 8;
        int bitIndex = 0;
        for (int step = 0; step < maxSteps; step++) {
            int offset = step * 8;
            for (int k = 0; k < szerokoscObrazu; k++) {
                for (int r = 0; r < 8; r++) {
                    int columnAbsolute = k;
                    int rowAbsolute = offset + r;

                    if (tryb == TrybObrazu::PaletaNarzucona) {
                        bitset5[bitIndex] = z24RGBna5RGB(obrazek[columnAbsolute][rowAbsolute]) >> 3;
                    } else {
                        bitset5[bitIndex] = z24RGBna5BW(obrazek[columnAbsolute][rowAbsolute]) >> 3;
                    }

                    bitIndex++;
                }
            }
        }

        // Stara wersja - zapisuje od lewej do prawej i gory na dol
        /*
            for (int y; y < wysokoscObrazka; y++) {
                for (int x; x < szerokoscObrazka; x++) {
                    int index = y * szerokoscObrazka + x;

                    if (tryb == TrybObrazu::PaletaNarzucona) {
                        bitset5[index] = z24RGBna5RGB(obrazek[y][x]) >> 3;
                    } else {
                        bitset5[index] = z24RGBna5BW(obrazek[y][x]) >> 3;
                    }
                }
            }
        */

        std::vector<uint8_t> packedBits;
        int bitCounter = 0;
        uint8_t currentByte = 0;

        for (const auto &bit5 : bitset5) {
            // Convert the 5-bit value to an unsigned long and then to an 8-bit
            // value
            uint8_t value = bit5.to_ulong();

            // Check if adding 5 bits to the current byte exceeds 8 bits
            if (bitCounter + 5 <= 8) {
                // If not, shift the 5-bit value left by the number of empty bit
                // positions in the current byte and OR it with the current byte
                // to add these bits to the byte.
                currentByte |= (value << (8 - bitCounter - 5));
                // Increase the bit counter by 5 since we have added 5 more
                // bits.
                bitCounter += 5;
            } else {
                // If adding 5 bits exceeds 8 bits, calculate how many bits will
                // overflow
                int overflow = bitCounter + 5 - 8;
                // Add the non-overflowing part of the value to the current byte
                currentByte |= (value >> overflow);
                // Push the completed 8-bit byte to the packedBits vector
                packedBits.push_back(currentByte);
                // Create a new byte with the overflow bits, shifted left to
                // their position in the new byte

                currentByte = (value & ((1 << overflow) - 1)) << (8 - overflow);
                // Set the bit counter to the number of bits in the overflow
                bitCounter = overflow;
            }
        }

        if (bitCounter > 0) {
            packedBits.push_back(currentByte);
        }

        // save to file
        wyjscie.write((char *)&packedBits[0], packedBits.size());
    }

    int maxSteps = wysokoscObrazu / 8;
    for (int step = 0; step < maxSteps; step++) {
        int offset = step * 8;
        for (int k = 0; k < szerokoscObrazu; k++) {
            for (int r = 0; r < 8; r++) {
                int columnAbsolute = k;
                int rowAbsolute = offset + r;
            }
        }
    }

    if (tryb == TrybObrazu::PaletaNarzucona) {
    }

    wyjscie.close();
}

// read
void FunkcjaT() {
    cout << "Wczytaj obrazek z pliku" << endl;

    ifstream wejscie("obraz.z33", ios::binary);
    char id[2];
    Uint16 szerokoscObrazu;
    Uint16 wysokoscObrazu;
    TrybObrazu tryb;
    Dithering dithering;

    wejscie.read((char *)&id, sizeof(char) * 2);
    wejscie.read((char *)&szerokoscObrazu, sizeof(char) * 2);
    wejscie.read((char *)&wysokoscObrazu, sizeof(char) * 2);
    wejscie.read((char *)&tryb, sizeof(Uint8));
    wejscie.read((char *)&dithering, sizeof(Uint8));
    // if (tryb >= TrybObrazu::SzaroscDedykowana) {
    //     // wczytac palete[256???] w offsetcie 8
    //     // tryb 1 to funkcja 5(tutaj z24 na 8bit) lub 6(tutaj z 24 na 5 bit)
    //     // tryb 2 to funkcja 7 chyba tylko jakies przesuniecie jest
    //     // tryb 3 to funkcja ???
    //     // tryb 4 to funkcja ???
    //     // tryb 5 to funkcja ???

    // } else {
    // }

    // ZapisDoPliku(1,0,);

    cout << "id: " << id[0] << id[1] << endl;
    cout << "szerokosc: " << szerokoscObrazka << endl;
    cout << "wysokosc: " << wysokoscObrazka << endl;
    cout << "tryb: " << (int)tryb << endl;
    cout << "dithering: " << (int)dithering << endl;

    wejscie.close();
}

void setPixel(int x, int y, Uint8 R, Uint8 G, Uint8 B) {
    if ((x >= 0) && (x < szerokosc) && (y >= 0) && (y < wysokosc)) {
        /* Zamieniamy poszczególne składowe koloru na format koloru piksela */
        Uint32 pixel = SDL_MapRGB(screen->format, R, G, B);

        /* Pobieramy informację ile bajtów zajmuje jeden piksel */
        int bpp = screen->format->BytesPerPixel;

        /* Obliczamy adres piksela */
        Uint8 *p1 =
            (Uint8 *)screen->pixels + (y * 2) * screen->pitch + (x * 2) * bpp;
        Uint8 *p2 = (Uint8 *)screen->pixels + (y * 2 + 1) * screen->pitch +
                    (x * 2) * bpp;
        Uint8 *p3 = (Uint8 *)screen->pixels + (y * 2) * screen->pitch +
                    (x * 2 + 1) * bpp;
        Uint8 *p4 = (Uint8 *)screen->pixels + (y * 2 + 1) * screen->pitch +
                    (x * 2 + 1) * bpp;

        /* Ustawiamy wartość piksela, w zależnoœci od formatu powierzchni*/
        switch (bpp) {
            case 1:  // 8-bit
                *p1 = pixel;
                *p2 = pixel;
                *p3 = pixel;
                *p4 = pixel;
                break;

            case 2:  // 16-bit
                *(Uint16 *)p1 = pixel;
                *(Uint16 *)p2 = pixel;
                *(Uint16 *)p3 = pixel;
                *(Uint16 *)p4 = pixel;
                break;

            case 3:  // 24-bit
                if (SDL_BYTEORDER == SDL_BIG_ENDIAN) {
                    p1[0] = (pixel >> 16) & 0xff;
                    p1[1] = (pixel >> 8) & 0xff;
                    p1[2] = pixel & 0xff;
                    p2[0] = (pixel >> 16) & 0xff;
                    p2[1] = (pixel >> 8) & 0xff;
                    p2[2] = pixel & 0xff;
                    p3[0] = (pixel >> 16) & 0xff;
                    p3[1] = (pixel >> 8) & 0xff;
                    p3[2] = pixel & 0xff;
                    p4[0] = (pixel >> 16) & 0xff;
                    p4[1] = (pixel >> 8) & 0xff;
                    p4[2] = pixel & 0xff;
                } else {
                    p1[0] = pixel & 0xff;
                    p1[1] = (pixel >> 8) & 0xff;
                    p1[2] = (pixel >> 16) & 0xff;
                    p2[0] = pixel & 0xff;
                    p2[1] = (pixel >> 8) & 0xff;
                    p2[2] = (pixel >> 16) & 0xff;
                    p3[0] = pixel & 0xff;
                    p3[1] = (pixel >> 8) & 0xff;
                    p3[2] = (pixel >> 16) & 0xff;
                    p4[0] = pixel & 0xff;
                    p4[1] = (pixel >> 8) & 0xff;
                    p4[2] = (pixel >> 16) & 0xff;
                }
                break;

            case 4:  // 32-bit
                *(Uint32 *)p1 = pixel;
                *(Uint32 *)p2 = pixel;
                *(Uint32 *)p3 = pixel;
                *(Uint32 *)p4 = pixel;
                break;
        }
    }
}

void setPixelSurface(int x, int y, Uint8 R, Uint8 G, Uint8 B) {
    if ((x >= 0) && (x < szerokosc * 2) && (y >= 0) && (y < wysokosc * 2)) {
        /* Zamieniamy poszczególne składowe koloru na format koloru piksela */
        Uint32 pixel = SDL_MapRGB(screen->format, R, G, B);

        /* Pobieramy informację ile bajtów zajmuje jeden piksel */
        int bpp = screen->format->BytesPerPixel;

        /* Obliczamy adres piksela */
        Uint8 *p = (Uint8 *)screen->pixels + y * screen->pitch + x * bpp;

        /* Ustawiamy wartość piksela, w zależności od formatu powierzchni*/
        switch (bpp) {
            case 1:  // 8-bit
                *p = pixel;
                break;

            case 2:  // 16-bit
                *(Uint16 *)p = pixel;
                break;

            case 3:  // 24-bit
                if (SDL_BYTEORDER == SDL_BIG_ENDIAN) {
                    p[0] = (pixel >> 16) & 0xff;
                    p[1] = (pixel >> 8) & 0xff;
                    p[2] = pixel & 0xff;
                } else {
                    p[0] = pixel & 0xff;
                    p[1] = (pixel >> 8) & 0xff;
                    p[2] = (pixel >> 16) & 0xff;
                }
                break;

            case 4:  // 32-bit
                *(Uint32 *)p = pixel;
                break;
        }
    }
}

SDL_Color getPixel(int x, int y) {
    SDL_Color color;
    Uint32 col = 0;
    if ((x >= 0) && (x < szerokosc) && (y >= 0) && (y < wysokosc)) {
        // określamy pozycję
        char *pPosition = (char *)screen->pixels;

        // przesunięcie względem y
        pPosition += (screen->pitch * y * 2);

        // przesunięcie względem x
        pPosition += (screen->format->BytesPerPixel * x * 2);

        // kopiujemy dane piksela
        memcpy(&col, pPosition, screen->format->BytesPerPixel);

        // konwertujemy kolor
        SDL_GetRGB(col, screen->format, &color.r, &color.g, &color.b);
    }
    return (color);
}

SDL_Color getPixelSurface(int x, int y, SDL_Surface *surface) {
    SDL_Color color;
    Uint32 col = 0;
    if ((x >= 0) && (x < szerokosc) && (y >= 0) && (y < wysokosc)) {
        // określamy pozycję
        char *pPosition = (char *)surface->pixels;

        // przesunięcie względem y
        pPosition += (surface->pitch * y);

        // przesunięcie względem x
        pPosition += (surface->format->BytesPerPixel * x);

        // kopiujemy dane piksela
        memcpy(&col, pPosition, surface->format->BytesPerPixel);

        // konwertujemy kolor
        SDL_GetRGB(col, surface->format, &color.r, &color.g, &color.b);
    }
    return (color);
}

void ladujBMP(char const *nazwa, int x, int y) {
    SDL_Surface *bmp = SDL_LoadBMP(nazwa);
    if (!bmp) {
        printf("Unable to load bitmap: %s\n", SDL_GetError());
    } else {
        SDL_Color kolor;
        for (int yy = 0; yy < bmp->h; yy++) {
            for (int xx = 0; xx < bmp->w; xx++) {
                kolor = getPixelSurface(xx, yy, bmp);
                setPixel(xx, yy, kolor.r, kolor.g, kolor.b);
            }
        }
        SDL_FreeSurface(bmp);
        SDL_UpdateWindowSurface(window);
    }
}

void ladujBMPDoPamieci(char const *nazwa, Canvas &obrazek) {
    SDL_Surface *bmp = SDL_LoadBMP(nazwa);
    if (!bmp) {
        printf("Unable to load bitmap: %s\n", SDL_GetError());
    } else {
        SDL_Color kolor;
        for (int yy = 0; yy < bmp->h; yy++) {
            for (int xx = 0; xx < bmp->w; xx++) {
                kolor = getPixelSurface(xx, yy, bmp);
                obrazek[yy][xx] = kolor;
            }
        }
        SDL_FreeSurface(bmp);
    }
}

void czyscEkran(Uint8 R, Uint8 G, Uint8 B) {
    SDL_FillRect(screen, 0, SDL_MapRGB(screen->format, R, G, B));
    SDL_UpdateWindowSurface(window);
}

int main(int argc, char *argv[]) {
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        printf("SDL_Init Error: %s\n", SDL_GetError());
        return EXIT_FAILURE;
    }

    window =
        SDL_CreateWindow(tytul, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                         szerokosc * 2, wysokosc * 2, SDL_WINDOW_SHOWN);

    if (window == NULL) {
        printf("SDL_CreateWindow Error: %s\n", SDL_GetError());
        return EXIT_FAILURE;
    }

    screen = SDL_GetWindowSurface(window);
    if (screen == NULL) {
        fprintf(stderr, "SDL_GetWindowSurface Error: %s\n", SDL_GetError());
        return false;
    }
    SDL_UpdateWindowSurface(window);

    bool done = false;
    SDL_Event event;
    // główna pętla programu
    while (SDL_WaitEvent(&event)) {
        // sprawdzamy czy pojawiło się zdarzenie
        switch (event.type) {
            case SDL_QUIT:
                cout << "esc" << endl;
                done = true;
                break;

            // sprawdzamy czy został wciśnięty klawisz
            case SDL_KEYDOWN: {
                // wychodzimy, gdy wciśnięto ESC
                if (event.key.keysym.sym == SDLK_ESCAPE) done = true;

                if (event.key.keysym.sym == SDLK_1) Funkcja1();
                if (event.key.keysym.sym == SDLK_2) Funkcja2();
                if (event.key.keysym.sym == SDLK_3) Funkcja3();
                if (event.key.keysym.sym == SDLK_4) Funkcja4();
                if (event.key.keysym.sym == SDLK_5) Funkcja5();
                if (event.key.keysym.sym == SDLK_6) Funkcja6();
                if (event.key.keysym.sym == SDLK_7) Funkcja7();
                if (event.key.keysym.sym == SDLK_8) Funkcja8();
                if (event.key.keysym.sym == SDLK_9) Funkcja9();
                if (event.key.keysym.sym == SDLK_q) FunkcjaQ();
                if (event.key.keysym.sym == SDLK_w) FunkcjaW();
                if (event.key.keysym.sym == SDLK_e) FunkcjaE();
                if (event.key.keysym.sym == SDLK_r) FunkcjaR();
                if (event.key.keysym.sym == SDLK_t) FunkcjaT();

                if (event.key.keysym.sym == SDLK_a)
                    ladujBMP("obrazek1.bmp", 0, 0);
                if (event.key.keysym.sym == SDLK_s)
                    ladujBMP("obrazek2.bmp", 0, 0);
                if (event.key.keysym.sym == SDLK_d)
                    ladujBMP("obrazek3.bmp", 0, 0);
                if (event.key.keysym.sym == SDLK_f)
                    ladujBMP("obrazek4.bmp", 0, 0);
                if (event.key.keysym.sym == SDLK_g)
                    ladujBMP("obrazek5.bmp", 0, 0);
                if (event.key.keysym.sym == SDLK_h)
                    ladujBMP("obrazek6.bmp", 0, 0);
                if (event.key.keysym.sym == SDLK_j)
                    ladujBMP("obrazek7.bmp", 0, 0);
                if (event.key.keysym.sym == SDLK_k)
                    ladujBMP("obrazek8.bmp", 0, 0);
                if (event.key.keysym.sym == SDLK_l)
                    ladujBMP("obrazek9.bmp", 0, 0);
                if (event.key.keysym.sym == SDLK_b)
                    czyscEkran(0, 0, 0);
                else
                    break;
            }
        }
        if (done) break;
    }

    if (screen) {
        SDL_FreeSurface(screen);
    }

    if (window) {
        SDL_DestroyWindow(window);
    }

    SDL_Quit();
    return 0;
}
