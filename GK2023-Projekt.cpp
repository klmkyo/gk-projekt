#include <math.h>
#include <stdio.h>
#include <string.h>

#include <algorithm>
#include <bitset>
#include <cmath>
#include <exception>
#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <unordered_set>
#include <vector>
#include <ctime>
#include <limits.h>


#include "SDL_surface.h"

using namespace std;

#define szerokosc 640
#define wysokosc 400
#define PALETA_SIZE 32
#define OBRAZEK_SIZE 64000

struct Color {
    Uint8 r, g, b;
};

const char FILE_SIGNATURE[2] = {0x33, 0x33};



// wymagane dla unordered_set
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
}  // namespace std

typedef std::vector<std::vector<Color>> Canvas;
typedef std::vector<Color> Canvas1D;

enum SkladowaRGB {
    R,
    G,
    B,
};

// 1 i 2 oznaczają, że przy czytaniu obrazu będzie używana paleta WBUDOWANA W
// PROGRAM w 2, 3, 4 paleta jest dołączona do pliku
// 3 i 5 MEDIANCUT
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

bool czyTrybJestZPaleta(TrybObrazu tryb) { return tryb >= 3; }
SkladowaRGB najwiekszaRoznica(int start, int koniec, Canvas1D &obrazek);

void setPixel(int x, int y, Uint8 R, Uint8 G, Uint8 B);
Color getPixel(int x, int y);

Uint8 z24RGBna5RGB(Color kolor);
Color z5RGBna24RGB(Uint8 kolor5bit);

void ZapisDoPliku(std::string nazwaPliku, TrybObrazu tryb, Dithering dithering, Canvas &obrazek,
                  Canvas1D &paleta);
void czyscEkran(Uint8 R, Uint8 G, Uint8 B);
Canvas OdczytZPliku(const std::string &filename);

void KonwertujBmpNaKfc(std::string bmpZrodlo, std::string kfcCel, TrybObrazu tryb, Dithering d);
Canvas1D wyprostujCanvas(Canvas &obrazek);

Uint8 normalizacja(int wartosc) {
    if (wartosc < 0) return 0;
    if (wartosc > 255) return 255;
    return wartosc;
}



Canvas ladujBMPDoPamieci(std::string nazwa);
bool porownajKolory(Color kolor1, Color kolor2);

Uint8 z24RGBna5RGB(Color kolor) {
    Uint8 nowyR, nowyG, nowyB;
    nowyR = round(kolor.r * 3.0 / 255.0); // 100 -> 300 -> 1
    nowyG = round(kolor.g * 3.0 / 255.0); // 100 -> 300 -> 1
    nowyB = round(kolor.b * 1.0 / 255.0); // 100 -> 100 -> 1

    return (nowyR << 6) | (nowyG << 4) | (nowyB << 3);
}

Color z5RGBna24RGB(Uint8 kolor5bit) {
    Color kolor;
    kolor.r = ((kolor5bit & 0b11000000) >> 6) * 255.0 / 3.0;
    kolor.g = ((kolor5bit & 0b00110000) >> 4) * 255.0 / 3.0;
    kolor.b = ((kolor5bit & 0b00001000) >> 3) * 255.0 / 1.0;

    return kolor;
}

Uint8 z24RGBna5BW(Color kolor) {
    int szary8bit = 0.299 * kolor.r + 0.587 * kolor.g + 0.114 * kolor.b;
    int szary5bit = round(szary8bit * 31.0 / 255.0);

    return szary5bit;
}

Color z5BWna24RGB(Uint8 kolor) {
    Uint8 szary8bit = round(kolor * 255.0 / 31.0);

    Color kolor24bit = {szary8bit, szary8bit, szary8bit};
    return kolor24bit;
}

bool porownajKolory(Color kolor1, Color kolor2) {
    return kolor1.r == kolor2.r && kolor1.g == kolor2.g && kolor1.b == kolor2.b;
}

void medianCutBW(int start, int koniec, int iteracja, Canvas1D &obrazek,
                 Canvas1D &paleta) {
    if (iteracja > 0) {
        sort(obrazek.begin() + start, obrazek.begin() + koniec,
             [](Color a, Color b) { return a.r < b.r; });

        // cout << "Dzielenie kubełka KFC na poziomie " << iteracja << endl;

        int srodek = (start + koniec + 1) / 2;
        medianCutBW(start, srodek - 1, iteracja - 1, obrazek, paleta);
        medianCutBW(srodek, koniec, iteracja - 1, obrazek, paleta);
    } else {
        // budowanie palety uśredniając kolory z określonego kubełka
        int sumaBW = 0;
        for (int p = start; p < koniec; p++) {
            sumaBW += obrazek[p].r;
        }
        Uint8 noweBW = sumaBW / (koniec + 1 - start);
        Color nowyKolor = {noweBW, noweBW, noweBW};
        paleta.push_back(nowyKolor);

        // cout << "🍿 Kubełek " << paleta.size() << " (" << start << "," << koniec
        //      << ") kolorBW: " << (int)noweBW << endl;
    }
}

void medianCutRGB(int start, int koniec, int iteracja, Canvas1D &obrazek,
                  Canvas1D &paleta) {
    // std::cout << "medianCutRGB start: " << start << " koniec: " << koniec
    //           << " iteracja: " << iteracja << std::endl;
    if (iteracja > 0) {
        // sortowanie wtorkowego kubełka kfc za 22 zł
        SkladowaRGB skladowa = najwiekszaRoznica(start, koniec, obrazek);

        sort(obrazek.begin() + start, obrazek.begin() + koniec,
             [skladowa](Color a, Color b) {
                 if (skladowa == R) return a.r < b.r;
                 if (skladowa == G) return a.g < b.g;
                 if (skladowa == B) return a.b < b.b;
             });

        // cout << "Dzielenie kubełka KFC na poziomie " << iteracja << endl;

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
        Color nowyKolor = {Uint8(sumaR / ilosc), Uint8(sumaG / ilosc),
                           Uint8(sumaB / ilosc)};
        paleta.push_back(nowyKolor);

        // cout << "🍿 Kubełek " << paleta.size() << " (" << start << "," << koniec
        //      << ") koloryRGB: " << (int)nowyKolor.r << " " << (int)nowyKolor.g
        //      << " " << (int)nowyKolor.b << endl;
    }
}

int znajdzNajblizszyKolorIndex(Color kolor, Canvas1D &paleta) {
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

int znajdzNajblizszyKolorBWIndex(Uint8 szary, Canvas1D &paleta) {
    Color c;
    c.r = szary;
    return znajdzNajblizszyKolorIndex(c, paleta);
}

Color znajdzNajblizszyKolor(Color kolor, Canvas1D &paleta) {
    int najblizszyKolor = 0;
    int najmniejszaRoznica = INT_MAX;
    for (int j = 0; j < paleta.size(); j++) {
        int roznicaR = paleta[j].r - kolor.r;
        int roznicaG = paleta[j].g - kolor.g;
        int roznicaB = paleta[j].b - kolor.b;
        int roznica = roznicaR * roznicaR + roznicaG * roznicaG + roznicaB * roznicaB;
        if (roznica < najmniejszaRoznica) {
            najmniejszaRoznica = roznica;
            najblizszyKolor = j;
        }
    }
    return paleta[najblizszyKolor];
}


// in obrazek[start..koniec], find the color with highest difference
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

void applyFloydSteinbergDithering(Canvas &image, bool blackWhite) {
    int width = image[0].size();
    int height = image.size();

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            Color oldPixel = image[y][x];
            Color newPixel;
            if (blackWhite) {
                newPixel = z5BWna24RGB(z24RGBna5BW(oldPixel));
            } else {
                newPixel = z5RGBna24RGB(z24RGBna5RGB(oldPixel));
            }
            image[y][x] = newPixel;

            int errR = oldPixel.r - newPixel.r;
            int errG = oldPixel.g - newPixel.g;
            int errB = oldPixel.b - newPixel.b;

            if (x + 1 < width) {
                image[y][x + 1].r = normalizacja(image[y][x + 1].r + errR * 7 / 16);
                image[y][x + 1].g = normalizacja(image[y][x + 1].g + errG * 7 / 16);
                image[y][x + 1].b = normalizacja(image[y][x + 1].b + errB * 7 / 16);
            }
            if (x - 1 >= 0 && y + 1 < height) {
                image[y + 1][x - 1].r = normalizacja(image[y + 1][x - 1].r + errR * 3 / 16);
                image[y + 1][x - 1].g = normalizacja(image[y + 1][x - 1].g + errG * 3 / 16);
                image[y + 1][x - 1].b = normalizacja(image[y + 1][x - 1].b + errB * 3 / 16);
            }
            if (y + 1 < height) {
                image[y + 1][x].r = normalizacja(image[y + 1][x].r + errR * 5 / 16);
                image[y + 1][x].g = normalizacja(image[y + 1][x].g + errG * 5 / 16);
                image[y + 1][x].b = normalizacja(image[y + 1][x].b + errB * 5 / 16);
            }
            if (x + 1 < width && y + 1 < height) {
                image[y + 1][x + 1].r = normalizacja(image[y + 1][x + 1].r + errR / 16);
                image[y + 1][x + 1].g = normalizacja(image[y + 1][x + 1].g + errG / 16);
                image[y + 1][x + 1].b = normalizacja(image[y + 1][x + 1].b + errB / 16);
            }
        }
    }
}

void applyBayerDithering(Canvas &image, bool blackWhite) {
    int bayerMatrix[4][4] = {
        { 1,  9,  3, 11},
        {13,  5, 15,  7},
        { 4, 12,  2, 10},
        {16,  8, 14,  6}
    };

    int width = image[0].size();
    int height = image.size();

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            Color oldPixel = image[y][x];

            // Adjust each color channel based on the Bayer matrix
            oldPixel.r = min(max(0, oldPixel.r + bayerMatrix[y % 4][x % 4] - 8), 255);
            oldPixel.g = min(max(0, oldPixel.g + bayerMatrix[y % 4][x % 4] - 8), 255);
            oldPixel.b = min(max(0, oldPixel.b + bayerMatrix[y % 4][x % 4] - 8), 255);

            Color newPixel;
            if (blackWhite) {
                newPixel = z5BWna24RGB(z24RGBna5BW(oldPixel));
            } else {
                newPixel = z5RGBna24RGB(z24RGBna5RGB(oldPixel));
            }
            image[y][x] = newPixel;
        }
    }
}


void applyFloydSteinbergDithering(Canvas &image, Canvas1D &palette) {
    int width = image[0].size();
    int height = image.size();

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            Color oldPixel = image[y][x];
            Color newPixel = znajdzNajblizszyKolor(oldPixel, palette); // Find closest color in the palette
            image[y][x] = newPixel;

            int errR = oldPixel.r - newPixel.r;
            int errG = oldPixel.g - newPixel.g;
            int errB = oldPixel.b - newPixel.b;

            if (x + 1 < width) {
                image[y][x + 1].r = normalizacja(image[y][x + 1].r + errR * 7 / 16);
                image[y][x + 1].g = normalizacja(image[y][x + 1].g + errG * 7 / 16);
                image[y][x + 1].b = normalizacja(image[y][x + 1].b + errB * 7 / 16);
            }
            if (x - 1 >= 0 && y + 1 < height) {
                image[y + 1][x - 1].r = normalizacja(image[y + 1][x - 1].r + errR * 3 / 16);
                image[y + 1][x - 1].g = normalizacja(image[y + 1][x - 1].g + errG * 3 / 16);
                image[y + 1][x - 1].b = normalizacja(image[y + 1][x - 1].b + errB * 3 / 16);
            }
            if (y + 1 < height) {
                image[y + 1][x].r = normalizacja(image[y + 1][x].r + errR * 5 / 16);
                image[y + 1][x].g = normalizacja(image[y + 1][x].g + errG * 5 / 16);
                image[y + 1][x].b = normalizacja(image[y + 1][x].b + errB * 5 / 16);
            }
            if (x + 1 < width && y + 1 < height) {
                image[y + 1][x + 1].r = normalizacja(image[y + 1][x + 1].r + errR / 16);
                image[y + 1][x + 1].g = normalizacja(image[y + 1][x + 1].g + errG / 16);
                image[y + 1][x + 1].b = normalizacja(image[y + 1][x + 1].b + errB / 16);
            }
        }
    }
}

void applyBayerDithering(Canvas &image, Canvas1D &palette) {
    int bayerMatrix[4][4] = {
        { 1,  9,  3, 11},
        {13,  5, 15,  7},
        { 4, 12,  2, 10},
        {16,  8, 14,  6}
    };

    int width = image[0].size();
    int height = image.size();

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            Color oldPixel = image[y][x];

            // Adjust each color channel based on the Bayer matrix
            oldPixel.r = min(max(0, oldPixel.r + bayerMatrix[y % 4][x % 4] - 8), 255);
            oldPixel.g = min(max(0, oldPixel.g + bayerMatrix[y % 4][x % 4] - 8), 255);
            oldPixel.b = min(max(0, oldPixel.b + bayerMatrix[y % 4][x % 4] - 8), 255);

            Color newPixel = znajdzNajblizszyKolor(oldPixel, palette);
            image[y][x] = newPixel;
        }
    }
}


/// takes a path to bmp file, and creates a converted version of it
/// abc.bmp -> abc.kfc
void KonwertujBmpNaKfc(std::string bmpZrodlo, std::string kfcCel, TrybObrazu tryb, Dithering d) {
    Canvas obrazek = ladujBMPDoPamieci(bmpZrodlo);
    Canvas1D obrazek1D = wyprostujCanvas(obrazek);
    Canvas1D paleta;

    switch (tryb) {
        case TrybObrazu::PaletaDedykowana: {
            medianCutRGB(0, obrazek1D.size() - 1, 5, obrazek1D, paleta); 
            break;
        }
        case TrybObrazu::SzaroscDedykowana: {
            medianCutBW(0, obrazek1D.size() - 1, 5, obrazek1D, paleta);
            break;
        }
        case TrybObrazu::PaletaWykryta: {
            // PaletaWykryta z tego co patrzylem na stare commity
            // to poprostu iteracja po kolorach w obrazie i jak nie ma go juz w
            // palecie to go dodajemy jak juz jest 256 to wiecej nie dodajemyp
            // mozna to by bylo zrobic find_if ale chyba tez tak git
            std::unordered_set<Color> paletaSet;

            Canvas1D shuffledObrazek1D(obrazek1D);
            std::srand(std::time(nullptr));
            std::random_shuffle(shuffledObrazek1D.begin(), shuffledObrazek1D.end());

            for (const auto &c : shuffledObrazek1D) {
                paletaSet.insert(c);
                // mamy tutaj 256 kolorow max, w dokumentacji jest ze paleta ma
                // 256 * 3 (bo RGB)
                // ale my mamy 5 bitow wiec 32??
                if (paletaSet.size() >= 32) break;
            }

            paleta = Canvas1D(paletaSet.begin(), paletaSet.end());

            // paddowanie palety, jeśli nie sięga 32 kolorów
            while (paleta.size() < 32) {
                paleta.push_back({0, 0, 0});
            }
            break;
        }

        default: {
            break;
        }
    }

    switch (d) {
        case Dithering::Floyd: {
            if(czyTrybJestZPaleta(tryb)) {
                applyFloydSteinbergDithering(obrazek, paleta);
            } else {
                applyFloydSteinbergDithering(obrazek, tryb == TrybObrazu::SzaroscNarzucona);
            }
            break;
        }
        case Dithering::Bayer: {
            if(czyTrybJestZPaleta(tryb)) {
                applyBayerDithering(obrazek, paleta);
            } else {
                applyBayerDithering(obrazek, tryb == TrybObrazu::SzaroscNarzucona);
            }
            break;
        }
        default: {
            break;
        }
    }

    

    ZapisDoPliku(kfcCel, tryb, d, obrazek, paleta);
    std::cout << "Zapisano obrazek w formacie KFC" << std::endl;

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
void ZapisDoPliku(std::string nazwaPliku, TrybObrazu tryb, Dithering dithering,
                  Canvas &obrazek, Canvas1D &paleta) {
    // TODO: moze byc zle znow width z heightem tutaj
    Uint16 szerokoscObrazu = obrazek[0].size();
    Uint16 wysokoscObrazu = obrazek.size();
    const int iloscBitowDoZapisania = 5 * szerokoscObrazu * wysokoscObrazu;

    if (szerokoscObrazu % 8 != 0) {
        throw std::invalid_argument(
            "Szerokosc obrazka nie jest wielokrotnoscia 8");
    }
    
    cout << "Zapisuje obrazek do pliku..." << endl;

    ofstream wyjscie(nazwaPliku, ios::binary);
    wyjscie.write((char *)&FILE_SIGNATURE, sizeof(char) * 2);
    wyjscie.write((char *)&szerokoscObrazu, sizeof(char) * 2);
    wyjscie.write((char *)&wysokoscObrazu, sizeof(char) * 2);
    wyjscie.write((char *)&tryb, sizeof(Uint8));
    wyjscie.write((char *)&dithering, sizeof(Uint8));

    // print the cursor (where we are at the file)
    // cout << "Cursor: " << wyjscie.tellp() << endl;

    // wypisanie palety
    // for (const auto& c: paleta) {
    //     cout << "Paleta: " << (int)c.r << " " << (int)c.g << " " << (int)c.b << endl;
    // }

    if (czyTrybJestZPaleta(tryb)) {
        for(const auto& c: paleta) {
            wyjscie.write((char *)&c, sizeof(Color));
        }
        // cout << "Zapisano palete" << endl;
    }

    // print the cursor (where we are at the file)
    // cout << "Cursor: " << wyjscie.tellp() << endl;

    vector<bitset<5>> bitset5(iloscBitowDoZapisania / 5);

    // Nowa wersja - zapisuje po kolumnach ale max 8 rzędów
    // i potem przechodzi 8 rzedów niżej i znowu wszystkie kolumny itd......
    int maxSteps = wysokoscObrazu / 8;
    int bitIndex = 0;
    for (int step = 0; step < maxSteps; step++) {
        int offset = step * 8;
        for (int k = 0; k < szerokoscObrazu; k++) {
            for (int r = 0; r < 8; r++) {
                int columnAbsolute = k;
                int rowAbsolute = offset + r;
            
                if (tryb == TrybObrazu::PaletaNarzucona) {
                    bitset5[bitIndex] =
                        z24RGBna5RGB(obrazek[rowAbsolute][columnAbsolute]) >> 3;
                } else if (tryb == TrybObrazu::SzaroscNarzucona) {
                    bitset5[bitIndex] =
                        z24RGBna5BW(obrazek[rowAbsolute][columnAbsolute]);
                    cout << (int)z24RGBna5BW(obrazek[rowAbsolute][columnAbsolute]) << " ";
                } else if (tryb == TrybObrazu::SzaroscDedykowana) {
                    // też adresy do palety (która jest poprostu szara xD)
                    bitset5[bitIndex] = znajdzNajblizszyKolorIndex(
                        obrazek[rowAbsolute][columnAbsolute], paleta);
                } else if (tryb == TrybObrazu::PaletaWykryta) {
                    bitset5[bitIndex] = znajdzNajblizszyKolorIndex(
                        obrazek[rowAbsolute][columnAbsolute], paleta);

                } else if (tryb == TrybObrazu::PaletaDedykowana) {
                    bitset5[bitIndex] = znajdzNajblizszyKolorIndex(
                        obrazek[rowAbsolute][columnAbsolute], paleta);
                } else {
                    throw std::invalid_argument("Nieznany tryb obrazu");
                }

                bitIndex++;
            }
        }
    }

    unsigned char buffer = 0;
    int bitCount = 0;
    for (const auto& bs : bitset5) {
        for (int i = 0; i < 5; ++i) {
            // Set the bit in the buffer
            if (bs.test(i)) {
                buffer |= (1 << bitCount);
            }

            bitCount++;

            // When buffer is full (8 bits), write it to file and reset
            if (bitCount == 8) {
                wyjscie.put(buffer);
                buffer = 0;
                bitCount = 0;
            }
        }
    }

    // Write remaining bits if there are any
    if (bitCount > 0) {
        wyjscie.put(buffer);
    }

    wyjscie.close();
}

void ZapiszCanvasDoBmp(const Canvas& image, const std::string& filename) {
    int32_t width = image[0].size();
    int32_t height = image.size();
    int32_t rowPadding = (4 - (width * 3) % 4) % 4;
    int32_t fileSize = 54 + (3 * width + rowPadding) * height;

    std::ofstream file(filename, std::ios::out | std::ios::binary);

    // BMP Header
    file.put('B').put('M');
    file.write(reinterpret_cast<const char*>(&fileSize), 4);
    file.write("\0\0\0\0", 4); // Reserved
    file.write("\x36\0\0\0", 4); // Pixel data offset

    // DIB Header
    file.write("\x28\0\0\0", 4); // Header size
    file.write(reinterpret_cast<const char*>(&width), 4);
    file.write(reinterpret_cast<const char*>(&height), 4);
    file.write("\1\0", 2); // Planes
    file.write("\x18\0", 2); // Bits per pixel
    file.write("\0\0\0\0", 4); // Compression
    file.write("\0\0\0\0", 4); // Image size (can be 0 for uncompressed)
    file.write("\x13\0\0\0", 4); // Horizontal resolution (pixels/meter)
    file.write("\x13\0\0\0", 4); // Vertical resolution (pixels/meter)
    file.write("\0\0\0\0", 4); // Colors in color table
    file.write("\0\0\0\0", 4); // Important color count

    // Pixel Data
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            Color color = image[height-y-1][x];
            file.put(color.b).put(color.g).put(color.r);
        }
        file.write("\0\0\0", rowPadding);
    }

    file.close();
}

void KonwertujKfcNaBmp(std::string kfcZrodlo, std::string bmpCel) {
    Canvas obrazek = OdczytZPliku(kfcZrodlo);
    cout << "Zapisuje obrazek do pliku..." << endl;

    // // DEBUG make obrazek a white to black, l to r gradient
    // for (int i = 0; i < obrazek.size(); i++) {
    //     for (int j = 0; j < obrazek[i].size(); j++) {
    //         obrazek[i][j] = {Uint8(j * 255.0 / obrazek[i].size()),
    //                          Uint8(j * 255.0 / obrazek[i].size()),
    //                          Uint8(j * 255.0 / obrazek[i].size())};
    //     }
    // }

    ZapiszCanvasDoBmp(obrazek, bmpCel);
}


Canvas OdczytZPliku(const std::string &filename) {
    std::cout << "Wczytuje obrazek " << filename << " z pliku..." << std::endl;

    ifstream wejscie(filename, ios::binary);
    char id[2];
    Uint16 szerokoscObrazu;
    Uint16 wysokoscObrazu;
    
    char _tryb, _dithering;

    wejscie.read((char *)&id, sizeof(char) * 2);
    wejscie.read((char *)&szerokoscObrazu, sizeof(char) * 2);
    wejscie.read((char *)&wysokoscObrazu, sizeof(char) * 2);
    wejscie.read((char *)&_tryb, sizeof(char));
    wejscie.read((char *)&_dithering, sizeof(char));
    
    TrybObrazu tryb = (TrybObrazu)_tryb;
    Dithering dithering = (Dithering)_dithering;

    // cout << "id: " << id[0] << id[1] << endl;
    // cout << "szerokosc: " << szerokoscObrazu << endl;
    // cout << "wysokosc: " << wysokoscObrazu << endl;
    // cout << "tryb: " << (int)tryb << endl;
    // cout << "dithering: " << (int)dithering << endl;


    Canvas1D paleta;
    if (czyTrybJestZPaleta(tryb)) {
        paleta = Canvas1D(PALETA_SIZE);
        for (auto &c : paleta) {
            wejscie.read((char *)&c, sizeof(Color));
        }

        // cout << "Odczytanio palete" << endl;
    }

    int iloscBitowDoOdczytania = szerokoscObrazu * wysokoscObrazu * 5;

    vector<bitset<5>> bitset5(iloscBitowDoOdczytania / 5);

    unsigned char buffer;
    int bitCount = 0;
    int bitIndex = 0;

    while (wejscie.get((char &)buffer)) {
        for (int i = 0; i < 8; ++i) {
            int bit = (buffer >> i) & 1;

            // Set the bit in the buffer
            bitset5[bitIndex].set(bitCount, bit);

            bitCount++;

            // When buffer is full (8 bits), write it to file and reset
            if (bitCount == 5) {
                bitCount = 0;
                bitIndex++;
            }
        }
    }

    int maxSteps = wysokoscObrazu / 8;
    bitIndex = 0;

    Canvas obrazek(wysokoscObrazu, std::vector<Color>(szerokoscObrazu));


    for (int step = 0; step < maxSteps; step++) {
        int offset = step * 8;
        for (int k = 0; k < szerokoscObrazu; k++) {
            for (int r = 0; r < 8; r++) {
                int columnAbsolute = k;
                int rowAbsolute = offset + r;

                if (tryb == TrybObrazu::PaletaNarzucona) {
                    obrazek[rowAbsolute][columnAbsolute] =
                        z5RGBna24RGB(bitset5[bitIndex].to_ulong() << 3);
                } else if (tryb == TrybObrazu::SzaroscNarzucona) {
                    obrazek[rowAbsolute][columnAbsolute] =
                        z5BWna24RGB(bitset5[bitIndex].to_ulong());
                } else if (tryb == TrybObrazu::SzaroscDedykowana) {
                    obrazek[rowAbsolute][columnAbsolute] =
                        paleta[bitset5[bitIndex].to_ulong()];
                } else if (tryb == TrybObrazu::PaletaWykryta) {
                    obrazek[rowAbsolute][columnAbsolute] =
                        paleta[bitset5[bitIndex].to_ulong()];
                } else if (tryb == TrybObrazu::PaletaDedykowana) {
                    obrazek[rowAbsolute][columnAbsolute] =
                        paleta[bitset5[bitIndex].to_ulong()];
                } else {
                    throw std::invalid_argument("Nieznany tryb obrazu");
                }

                bitIndex++;
            }
        }
    }

    wejscie.close();
    return obrazek;
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

Canvas ladujBMPDoPamieci(std::string nazwa) {
    SDL_Surface *bmp = SDL_LoadBMP(nazwa.c_str());
    if (!bmp) {
        printf("Unable to load bitmap: %s\n", SDL_GetError());
        exit(1);
    } else {
        Canvas obrazek(bmp->h, std::vector<Color>(bmp->w));
        Color kolor;
        
        for (int yy = 0; yy < bmp->h; yy++) {
            for (int xx = 0; xx < bmp->w; xx++) {
                SDL_Color kolorSDL = getPixelSurface(xx, yy, bmp);
                kolor.r = kolorSDL.r;
                kolor.g = kolorSDL.g;
                kolor.b = kolorSDL.b;
                obrazek[yy][xx] = kolor;
            }
        }
        SDL_FreeSurface(bmp);
        // std::cout << "zaladowano obrazek essa" << std::endl;
        return obrazek;
    }
}

// flatten canvas
Canvas1D wyprostujCanvas(Canvas &obrazek) {
    Canvas1D obrazek1D;
    obrazek1D.reserve(obrazek.size() * obrazek[0].size());

    for (const auto r : obrazek) {
        for (const auto c : r) {
            obrazek1D.push_back(c);
        }
    }

    return obrazek1D;
}

typedef std::map<int, std::vector<std::string>> CommandAliasMap;

template <typename T>
using ParameterMap = std::map<char, T>;

int findCommand(CommandAliasMap &commandsAliases, std::string command) {
    for (auto &aliases : commandsAliases) {
        for (auto &alias : aliases.second) {
            if (alias == command) return aliases.first;
        }
    }
    return 0;
}

/* Czyta parametry -t, -s itd.. oraz ich wartości do mapy parametrów */
void readParameterMap(ParameterMap<std::string> &parameterMap, int offset,
                      int argc, char *argv[]) {
    for (int i = offset; i < argc; i++) {
        if (sizeof(argv[i]) < 2) continue;
        if (argv[i][0] == '-' && i + 1 < argc)
            parameterMap[argv[i][1]] = std::string(argv[i + 1]);
    }
}

bool hasParameter(ParameterMap<std::string> &parameterMap, char parameter) {
    return parameterMap.find(parameter) != parameterMap.end();
}

int main(int argc, char *argv[]) {
    CommandAliasMap commandsAliases;

    /* tobmp - odczytuje plik kfc, zapisuje plik bmp */
    commandsAliases[1] = {"tobmp", "-t", "-tobmp"};
    /* frombmp - odczytuje plik bmp, zapisuje plik kfc */
    commandsAliases[2] = {"frombmp", "-f", "-frombmp"};

    const std::string appName = argc < 1 ? "kfc" : argv[0];

    /* Wypisuje wszystkie dostępne komendy bez opisu */
    if (argc <= 1 || (argc == 2 && (std::string(argv[1]) == "help" ||
                                    std::string(argv[1]) == "-help"))) {
        std::cout << "  Witamy w konwerterze obrazów 🍗 KFC <-> 🎨 BMP.\n"
                  << "Dostępne operacje:\n"
                  << "1. Konwersja formatu KFC na BMP\n"
                  << "> " << appName
                  << " tobmp <ścieżka_pliku_kfc> [-s ścieżka_pliku_bmp]\n"
                  << "Wyświetl więcej informacji używając '" << appName
                  << " -help tobmp'\n"
                  << "2. Konwersja formatu BMP na KFC\n"
                  << "> " << appName
                  << " frombmp <ścieżka_pliku_bmp> [-s ścieżka_pliku_kfc] [-t "
                     "tryb(1-5)] [-d dithering(none/bayer/floyd)]\n"
                  << "Wyświetl więcej informacji używając '" << appName
                  << " -help tobmp'\n";
    }

    /* W przypadku wysłania 'kfc help <command_name>' wyświetlony zostanie opis
       komendy */
    else if (argc == 3 && (std::string(argv[1]) == "help" ||
                           std::string(argv[1]) == "-help")) {
        int primaryCommandId = findCommand(commandsAliases, argv[2]);
        switch (primaryCommandId) {
            case 1: { /* tobmp */
                std::cout
                    << "> " << appName
                    << " tobmp <ścieżka_pliku_kfc> [-s ścieżka_pliku_bmp]\n"
                    << "Opis: Komenda 'tobmp' konwertuje plik w formacie KFC "
                       "na format BMP \n"
                    << "Parametry obowiązkowe:\n"
                    << "\t<ścieżka_pliku_kfc> - ścieżka do pliku w formacie "
                       "kfc (relatywna lub absolutna)\n"
                    << "Parametry opcjonalne:\n"
                    << "\t[-s ścieżka_pliku_bmp] - ścieżka do nowo utworzonego "
                       "pliku (domyślnie plik kfc ze zmienionym "
                       "rozszerzeniem)\n";
                break;
            }
            case 2: { /* frombmp*/
                std::cout
                    << "> " << appName
                    << " frombmp <ścieżka_pliku_kfc> [-s ścieżka_pliku_bmp] "
                       "[-t tryb(1-5)] [-d dithering(none/bayer/floyd)]\n"
                    << "Opis: Komenda 'frombmp' konwertuje plik w formacie BMP "
                       "na format KFC \n"
                    << "Parametry obowiązkowe:\n"
                    << "\t<ścieżka_pliku_bmp> - ścieżka do pliku w formacie "
                       "bmp (relatywna lub absolutna)\n"
                    << "Parametry opcjonalne:\n"
                    << "\t[-s ścieżka_pliku_kfc] - ścieżka do nowo utworzonego "
                       "pliku (domyślnie plik bmp ze zmienionym "
                       "rozszerzeniem)\n"
                    << "\t[-t tryb(1-5)] - tryb konwersji obrazu (domyślnie "
                       "1), dostępne tryby:\n"
                    << "\t\t1 - Paleta narzucona\n"
                    << "\t\t2 - Szarość narzucona\n"
                    << "\t\t3 - Paleta wykryta\n"
                    << "\t\t4 - Szarość wykryta\n"
                    << "\t\t5 - Paleta dedykowana\n"
                    << "\t[-d dithering(none/bayer/floyd)] - tryb ditheringu "
                       "(domyślnie none - bez ditheringu)\n";
                break;
            }
            default: {
                std::cout
                    << "Nieznana komenda. Użyj '" << appName
                    << " help' aby dowiedzieć się o istniejących komendach."
                    << std::endl;
                break;
            }
        }
    }

    /* W pozostałych przypadkach będzie próba rozpoznania komendy z 1 argumentu
       i jej wykonanie */
    else if (argc > 1) {
        int primaryCommandId = findCommand(commandsAliases, argv[1]);

        switch (primaryCommandId) {
            case 1: { /* tobmp <ścieżka_pliku_kfc> [-s ścieżka_pliku_bmp] */
                if (argc < 3) {
                    std::cout << "Nie podano ścieżki do pliku kfc. Użyj '"
                              << appName
                              << " help tobmp' aby dowiedzieć się więcej."
                              << std::endl;
                    break;
                }
                std::string kfcPath = argv[2];
                ParameterMap<std::string> parameterMap;
                readParameterMap(parameterMap, 3, argc, argv);
                /* parametr s - scieżka pliku kfc */
                std::string bmpPath =
                    hasParameter(parameterMap, 's')
                        ? parameterMap['s']
                        : kfcPath.substr(0, kfcPath.find_last_of('.')) + ".bmp";

                KonwertujKfcNaBmp(kfcPath, bmpPath);
                break;
            }
            case 2: { /* frombmp <ścieżka_pliku_kfc> [-s ścieżka_pliku_bmp] [-t
                         tryb(1-5)] [-d dithering(none/bayer/floyd)] */
                if (argc < 3) {
                    std::cout << "Nie podano ścieżki do pliku bmp. Użyj '"
                              << appName
                              << " help frombmp' aby dowiedzieć się więcej."
                              << std::endl;
                    break;
                }
                std::string bmpPath = argv[2];
                ParameterMap<std::string> parameterMap;
                readParameterMap(parameterMap, 3, argc, argv);
                /* parametr s - scieżka pliku kfc */
                std::string kfcPath =
                    hasParameter(parameterMap, 's')
                        ? parameterMap['s']
                        : bmpPath.substr(0, bmpPath.find_last_of('.')) + ".kfc";
                /* parametr t - tryb obrazu */
                TrybObrazu tryb = TrybObrazu::PaletaDedykowana;
                if (hasParameter(parameterMap, 't')) {
                    int _tryb = std::stoi(parameterMap['t']);
                    if (_tryb < 1 || _tryb > 5) {
                        std::cout << "Nieprawidłowy tryb konwersji. Użyj '"
                                  << appName
                                  << " help frombmp' aby dowiedzieć się więcej."
                                  << std::endl;
                        break;
                    }
                    tryb = static_cast<TrybObrazu>(_tryb);
                }
                /* parametr d - dithering*/
                Dithering dithering = Dithering::Brak;
                if (hasParameter(parameterMap, 'd')) {
                    std::string _dithering = parameterMap['d'];
                    if (_dithering == "none")
                        dithering = Dithering::Brak;
                    else if (_dithering == "bayer")
                        dithering = Dithering::Bayer;
                    else if (_dithering == "floyd")
                        dithering = Dithering::Floyd;
                    else {
                        std::cout << "Nieprawidłowy tryb ditheringu. Użyj '"
                                  << appName
                                  << " help frombmp' aby dowiedzieć się więcej."
                                  << std::endl;
                        break;
                    }
                }
                KonwertujBmpNaKfc(bmpPath, kfcPath, tryb, dithering);
                break;
            }
            default: {
                std::cout << "Nieznana komenda. Użyj '" << appName
                          << " help' aby dowiedzieć się o dostępnych komendach."
                          << std::endl;
                break;
            }
        }
    }
    return 0;
}
