#include <SDL2/SDL.h>
#include <stdio.h>

#include <algorithm>
#include <bitset>
#include <exception>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

using namespace std;

SDL_Window *window = NULL;
SDL_Surface *screen = NULL;

#define szerokosc 640
#define wysokosc 400
#define szerokoscObrazka (szerokosc / 2)
#define wysokoscObrazka (wysokosc / 2)
#define tytul "GK2023 - Projekt - Zespol 33"

#define PALETA_SIZE 32      // 2^5
#define OBRAZEK_SIZE 64000  // 320 * 200

typedef std::vector<std::vector<SDL_Color>> Canvas;
typedef std::vector<SDL_Color> Canvas1D;

enum SkladowaRGB {
    R,
    G,
    B,
};

// 1 i 2 oznaczają, że przy czytaniu obrazu będzie używana paleta WBUDOWANA W
// PROGRAM w 2, 3, 4 paleta jest dołączona do pliku
enum TrybObrazu {
    PaletaNarzucona = 1,    // przejście z 24bit obrazka na 5bit
    SzaroscNarzucona = 2,   // przejście z 24bit obrazka na 5bit szarosci
    SzaroscDedykowana = 3,  // utworzenie palety z 32 odcieniami szarosci i
                            // zapisanie obrazka jako indeksy do palety
    PaletaWykryta = 4,      // ?????????
    PaletaDedykowana = 5  // utworzenie palety z 32 kolorami i zapisanie obrazka
                          // jako indeksy do palety
};
enum Dithering { Brak = 0, Bayer = 1, Floyd = 2 };

constexpr int maxKolorow = 320 * 600;
int ileKubelkow = 0;

bool czyTrybJestZPaleta(TrybObrazu tryb) { return tryb >= 3; }
SkladowaRGB najwiekszaRoznica(int start, int koniec, Canvas1D& obrazek);

void setPixel(int x, int y, Uint8 R, Uint8 G, Uint8 B);
SDL_Color getPixel(int x, int y);

Uint8 z24RGBna5RGB(SDL_Color kolor);
SDL_Color z5RGBna24RGB(Uint8 kolor5bit);

void ZapisDoPliku(TrybObrazu tryb, Dithering dithering, Canvas &obrazek, Canvas1D &paleta);
void czyscEkran(Uint8 R, Uint8 G, Uint8 B);

void KonwertujBmpNaKfc(const char *bmpZrodlo);

Uint8 normalizacja(int wartosc) {
    if (wartosc < 0) return 0;
    if (wartosc > 255) return 255;
    return wartosc;
}

void FunkcjaQ() {}
void FunkcjaW();
void FunkcjaE();
void FunkcjaR();
void FunkcjaT();

void ladujBMPDoPamieci(char const *nazwa, Canvas &obrazek);
bool porownajKolory(SDL_Color kolor1, SDL_Color kolor2);

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

void updateBledy(int xx, int yy, float (*bledy)[wysokosc / 2 + 2][3], int blad,
                 int colorIndex, int przesuniecie) {
    bledy[xx + 1 + przesuniecie][yy][colorIndex] += (blad * 7.0 / 16.0);
    bledy[xx - 1 + przesuniecie][yy + 1][colorIndex] += (blad * 3.0 / 16.0);
    bledy[xx + przesuniecie][yy + 1][colorIndex] += (blad * 5.0 / 16.0);
    bledy[xx + 1 + przesuniecie][yy + 1][colorIndex] += (blad * 1.0 / 16.0);
}

bool porownajKolory(SDL_Color kolor1, SDL_Color kolor2) {
    return kolor1.r == kolor2.r && kolor1.g == kolor2.g && kolor1.b == kolor2.b;
}

void medianCutBW(int start, int koniec, int iteracja, Canvas1D &obrazek,
                 Canvas1D &paleta) {
    if (iteracja > 0) {
        // sortowanie wtorkowego kubełka kfc za 22 zł
        sort(obrazek.begin() + start, obrazek.begin() + koniec,
             [](SDL_Color a, SDL_Color b) { return a.r < b.r; });

        cout << "Dzielenie kubełka KFC na poziomie " << iteracja << endl;

        int srodek = (start + koniec + 1) / 2;
        medianCutBW(start, srodek - 1, iteracja - 1, obrazek, paleta);
        medianCutBW(srodek, koniec, iteracja - 1, obrazek, paleta);
    } else {
        // budowanie palety uśredniając kolory z określonego kubełka KFC
        int sumaBW = 0;
        for (int p = start; p < koniec; p++) {
            sumaBW += obrazek[p].r;
        }
        Uint8 noweBW = sumaBW / (koniec + 1 - start);
        SDL_Color nowyKolor = {noweBW, noweBW, noweBW};
        paleta.push_back(nowyKolor);

        printf("\n");
        cout << "🍿 Kubełek " << paleta.size() << "(" << start << "," << koniec
             << ") = 🍗 " << (int)noweBW << endl;
        cout << "🎨 Kolor " << ileKubelkow << ": " << (int)nowyKolor.r << " "
             << (int)nowyKolor.g << " " << (int)nowyKolor.b << endl;

    }
}

void medianCutRGB(int start, int koniec, int iteracja, Canvas1D& obrazek,
                             Canvas1D &paleta) {
    if (iteracja > 0) {
        // sortowanie wtorkowego kubełka kfc za 22 zł
        SkladowaRGB skladowa = najwiekszaRoznica(start, koniec, obrazek);

        sort(obrazek.begin() + start, obrazek.begin() + koniec,
             [skladowa](SDL_Color a, SDL_Color b) {
                 if (skladowa == R) return a.r < b.r;
                 if (skladowa == G) return a.g < b.g;
                 if (skladowa == B) return a.b < b.b;
             });

        cout << "Dzielenie kubełka KFC na poziomie " << iteracja << endl;

        int srodek = (start + koniec + 1) / 2;
        medianCutRGB(start, srodek - 1, iteracja - 1, obrazek, paleta);
        medianCutRGB(srodek, koniec, iteracja - 1, obrazek, paleta);
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
        paleta.push_back(nowyKolor);

        printf("\n");
        cout << "🍿 Kubełek / 🎨 Kolor " << paleta.size() << ": "
             << (int)nowyKolor.r << " " << (int)nowyKolor.g << " "
             << (int)nowyKolor.b << endl;

    }
}

int znajdzNajblizszyKolorIndex(SDL_Color kolor, Canvas1D& paleta) {
    int najblizszyKolor = 0;
    int najmniejszaRoznica = 255;
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

int znajdzNajblizszyKolorBWIndex(Uint8 szary, Canvas1D& paleta) {
    SDL_Color c;
    c.r = szary;
    return znajdzNajblizszyKolorIndex(c, paleta);
}

SDL_Color znajdzNajblizszyKolor(SDL_Color kolor, Canvas1D& paleta) {
    int najblizszyKolor = 0;
    int najmniejszaRoznica = 255;
    for (int j = 0; j < paleta.size(); j++) {
        int roznica = abs(paleta[j].r - kolor.r);
        if (roznica < najmniejszaRoznica) {
            najmniejszaRoznica = roznica;
            najblizszyKolor = j;
        }
    }
    return paleta[najblizszyKolor];
}

// in obrazek[start..koniec], find the color with highest difference
SkladowaRGB najwiekszaRoznica(int start, int koniec, Canvas1D& obrazek) {
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

    throw std::invalid_argument("Nieznana skladowa RGB");
}

// Co robi funkcja W
// 1. Dzieli kubełki KFC na poziomie 2
// 2. Buduje paletę uśredniając kolory z określonego kubełka KFC
// 3. Dla każdego piksela znajduje najbliższy kolor z palety
// 4. Wyświetla wartości
void FunkcjaW() {
    // SDL_Color kolor;
    // Uint8 szary;
    // int numer = 0, indeks = 0;
    // for (int y = 0; y < wysokosc / 2; y++) {
    //     for (int x = 0; x < szerokosc / 2; x++) {
    //         kolor = getPixel(x, y);
    //         szary = 0.299 * kolor.r + 0.587 * kolor.g + 0.114 * kolor.b;
    //         obrazek[numer] = {szary, szary, szary};
    //         setPixel(x + szerokosc / 2, y, szary, szary, szary);
    //         numer++;
    //     }
    // }
    // SDL_UpdateWindowSurface(window);
    // medianCutBW(0, numer - 1, 2);

    // for (int y = 0; y < wysokosc / 2; y++) {
    //     for (int x = 0; x < szerokosc / 2; x++) {
    //         kolor = getPixel(x, y);
    //         szary = 0.299 * kolor.r + 0.587 * kolor.g + 0.114 * kolor.b;
    //         indeks = znajdzNajblizszyKolorBWIndex(szary);

    //         setPixel(x + szerokosc / 2, y + wysokosc / 2, paleta[indeks].r,
    //                  paleta[indeks].g, paleta[indeks].b);
    //     }
    // }

    SDL_UpdateWindowSurface(window);
}

// Co robi funkcja E
// 1. Dzieli kubełki KFC na poziomie 2
// 2. Buduje paletę uśredniając kolory z określonego kubełka KFC
// 3. Dla każdego piksela znajduje najbliższy kolor z palety
// 4. Wyświetla wartości
// dobra, tak bez pierdolenia, ta funkcja robi to samo co funkcja W, tylko
// zamiast szarości używa RGB

void FunkcjaE() {
    // SDL_Color kolor;
    // SDL_Color nowyKolor;
    // int numer = 0, indeks = 0;
    // for (int y = 0; y < wysokosc / 2; y++) {
    //     for (int x = 0; x < szerokosc / 2; x++) {
    //         kolor = getPixel(x, y);
    //         obrazek[numer] = kolor;
    //         numer++;
    //     }
    // }
    // medianCutRGB(0, numer - 1, 5);

    // for (int y = 0; y < wysokosc / 2; y++) {
    //     for (int x = 0; x < szerokosc / 2; x++) {
    //         kolor = getPixel(x, y);
    //         indeks = znajdzNajblizszyKolorIndex(kolor);

    //         cout << "Dla " << x << ", " << y << " wybrano kolor index "
    //              << indeks << endl;

    //         setPixel(x + szerokosc / 2, y + wysokosc / 2, paleta[indeks].r,
    //                  paleta[indeks].g, paleta[indeks].b);
    //     }
    // }

    SDL_UpdateWindowSurface(window);
}

// pakowanie bitowe (dla 2 pierwszych trybow chyba, reszta paleta)

void FunkcjaR() {
    KonwertujBmpNaKfc("obrazek1.bmp");
}

/// takes a path to bmp file, and creates a converted version of it
/// abc.bmp -> abc.kfc
void KonwertujBmpNaKfc(const char *bmpZrodlo) {
    Canvas obrazek(wysokoscObrazka, std::vector<SDL_Color>(szerokoscObrazka));

    ladujBMPDoPamieci(bmpZrodlo, obrazek);

    // dithering itd
    // tutaj powstaje paleta
    Canvas1D paleta;

    // podaje obrazek + palete
    ZapisDoPliku(TrybObrazu::SzaroscNarzucona, Dithering::Brak, obrazek, paleta);
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
void ZapisDoPliku(TrybObrazu tryb, Dithering dithering, Canvas &obrazek, Canvas1D &paleta) {
    Uint16 szerokoscObrazu = szerokosc / 2;
    Uint16 wysokoscObrazu = wysokosc / 2;
    cout << "Zapisuje obrazek do pliku" << endl;
    // Data założenia KFC - September 24, 1952; 71 years ago

    char id[2] = {0x19, 0x52};
    ofstream wyjscie("obraz.kfc", ios::binary);
    wyjscie.write((char *)&id, sizeof(char) * 2);
    wyjscie.write((char *)&szerokoscObrazu, sizeof(char) * 2);
    wyjscie.write((char *)&wysokoscObrazu, sizeof(char) * 2);
    wyjscie.write((char *)&tryb, sizeof(Uint8));
    wyjscie.write((char *)&dithering, sizeof(Uint8));

    // 1, 2 - tryby bez palety -> rozmiar danych
    // 3, 4, 5 - tryby z paletą -> poleta, rozmiar danych.
    if (czyTrybJestZPaleta(tryb)) {
        // TODO: PALETA_SIZEE moze byc tutaj bledne
        wyjscie.write((char *)&paleta, PALETA_SIZE);
    }

    // ilosc bitow zawsze taka sama niezaleznie od trybu
    int iloscBitowDoZapisania = 5 * szerokoscObrazka * wysokoscObrazka;

    vector<bitset<5>> bitset5(iloscBitowDoZapisania);

    if (szerokoscObrazka % 8 != 0) {
        throw std::invalid_argument(
            "Szerokosc obrazka nie jest wielokrotnoscia 8");
    }

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
                    bitset5[bitIndex] =
                        z24RGBna5RGB(obrazek[columnAbsolute][rowAbsolute]) >> 3;
                } else if (tryb == TrybObrazu::SzaroscNarzucona) {
                    bitset5[bitIndex] =
                        z24RGBna5BW(obrazek[columnAbsolute][rowAbsolute]) >> 3;
                } else if (tryb == TrybObrazu::PaletaDedykowana) {
                    // adresy do palety
                    bitset5[bitIndex] = znajdzNajblizszyKolorIndex(
                        obrazek[columnAbsolute][rowAbsolute], paleta);
                } else if (tryb == TrybObrazu::SzaroscDedykowana) {
                    // też adresy do palety (która jest poprostu szara xD)
                    bitset5[bitIndex] = znajdzNajblizszyKolorIndex(
                        obrazek[columnAbsolute][rowAbsolute], paleta);
                } else if (tryb == TrybObrazu::PaletaWykryta) {
                    // ?????????
                } else {
                    throw std::invalid_argument("Nieznany tryb obrazu");
                }

                bitIndex++;
            }
        }
    }

    std::vector<uint8_t> packedBits;
    int bitCounter = 0;
    uint8_t currentByte = 0;

    for (const auto &bit5 : bitset5) {
        // Konwertuj 5-bitową wartość na unsigned long, a następnie na 8-bitową
        // wartość
        uint8_t value = bit5.to_ulong();

        // Sprawdź, czy dodanie 5 bitów do bieżącego bajtu przekracza 8 bitów
        if (bitCounter + 5 <= 8) {
            // Jeśli nie, przesuń wartość 5-bitową w lewo o liczbę pustych
            // pozycji bitowych w bieżącym bajcie i wykonaj operację OR z
            // bieżącym bajtem, aby dodać te bity do bajtu.
            currentByte |= (value << (8 - bitCounter - 5));
            // Zwiększ licznik bitów o 5, ponieważ dodaliśmy kolejne 5 bitów.
            bitCounter += 5;
        } else {
            // Jeśli dodanie 5 bitów przekracza 8 bitów, oblicz, ile bitów
            // zostanie przekroczone
            int overflow = bitCounter + 5 - 8;
            // Dodaj część wartości, która nie przekracza, do bieżącego bajtu
            currentByte |= (value >> overflow);
            // Dodaj ukończony 8-bitowy bajt do wektora packedBits
            packedBits.push_back(currentByte);
            // Utwórz nowy bajt z przekraczającymi bitami, przesuniętymi w lewo
            // do ich pozycji w nowym bajcie

            currentByte = (value & ((1 << overflow) - 1)) << (8 - overflow);
            // Ustaw licznik bitów na liczbę bitów w przekroczeniu
            bitCounter = overflow;
        }
    }

    if (bitCounter > 0) {
        packedBits.push_back(currentByte);
    }

    wyjscie.write((char *)&packedBits[0], packedBits.size());

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
    std::cout << "Hello World!" << std::endl;
    return 0;
}
