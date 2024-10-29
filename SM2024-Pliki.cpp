#include "SM2024-Pliki.h"
#include <fstream>
#include <iostream>
#include <algorithm>
#include <ctime>
#include <stdexcept>
#include <SDL.h>

void ZapisDoPliku(std::string nazwaPliku, TrybObrazu tryb, Dithering dithering,
                  Canvas &obrazek, Canvas1D &paleta) {
    Uint16 szerokoscObrazu = obrazek[0].size();
    Uint16 wysokoscObrazu = obrazek.size();
    const int iloscBitowDoZapisania = 5 * szerokoscObrazu * wysokoscObrazu;

    if (szerokoscObrazu % 8 != 0) {
        throw std::invalid_argument(
            "Szerokosc obrazka nie jest wielokrotnoscia 8");
    }

    std::cout << "Zapisuje obrazek do pliku..." << std::endl;

    std::ofstream wyjscie(nazwaPliku, std::ios::binary);
    wyjscie.write((char *)&FILE_SIGNATURE, sizeof(char) * 2);
    wyjscie.write((char *)&szerokoscObrazu, sizeof(char) * 2);
    wyjscie.write((char *)&wysokoscObrazu, sizeof(char) * 2);
    wyjscie.write((char *)&tryb, sizeof(Uint8));
    wyjscie.write((char *)&dithering, sizeof(Uint8));

    if (czyTrybJestZPaleta(tryb)) {
        for(const auto& c: paleta) {
            wyjscie.write((char *)&c, sizeof(Color));
        }
    }

    std::vector<std::bitset<5>> bitset5(iloscBitowDoZapisania / 5);

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
                } else if (tryb == TrybObrazu::SzaroscDedykowana) {
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
            if (bs.test(i)) {
                buffer |= (1 << bitCount);
            }

            bitCount++;

            if (bitCount == 8) {
                wyjscie.put(buffer);
                buffer = 0;
                bitCount = 0;
            }
        }
    }

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

    std::cout << "W obrazku jest " << getUniqueColorsInImage(image) << " unikalnych kolorow" << std::endl;

    std::ofstream file(filename, std::ios::out | std::ios::binary);

    file.put('B').put('M');
    file.write(reinterpret_cast<const char*>(&fileSize), 4);
    file.write("\0\0\0\0", 4);
    file.write("\x36\0\0\0", 4);

    file.write("\x28\0\0\0", 4);
    file.write(reinterpret_cast<const char*>(&width), 4);
    file.write(reinterpret_cast<const char*>(&height), 4);
    file.write("\1\0", 2);
    file.write("\x18\0", 2);
    file.write("\0\0\0\0", 4);
    file.write("\0\0\0\0", 4);
    file.write("\x13\0\0\0", 4);
    file.write("\x13\0\0\0", 4);
    file.write("\0\0\0\0", 4);
    file.write("\0\0\0\0", 4);

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
    std::cout << "Zapisuje obrazek do pliku..." << std::endl;

    ZapiszCanvasDoBmp(obrazek, bmpCel);
}

Canvas OdczytZPliku(const std::string &filename) {
    std::cout << "Wczytuje obrazek " << filename << " z pliku..." << std::endl;

    std::ifstream wejscie(filename, std::ios::binary);
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

    Canvas1D paleta;
    if (czyTrybJestZPaleta(tryb)) {
        paleta = Canvas1D(PALETA_SIZE);
        for (auto &c : paleta) {
            wejscie.read((char *)&c, sizeof(Color));
        }
    }

    int iloscBitowDoOdczytania = szerokoscObrazu * wysokoscObrazu * 5;

    std::vector<std::bitset<5>> bitset5(iloscBitowDoOdczytania / 5);

    unsigned char buffer;
    int bitCount = 0;
    int bitIndex = 0;

    while (wejscie.get((char &)buffer)) {
        for (int i = 0; i < 8; ++i) {
            int bit = (buffer >> i) & 1;

            bitset5[bitIndex].set(bitCount, bit);

            bitCount++;

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
        char *pPosition = (char *)surface->pixels;

        pPosition += (surface->pitch * y);

        pPosition += (surface->format->BytesPerPixel * x);

        memcpy(&col, pPosition, surface->format->BytesPerPixel);

        SDL_GetRGB(col, surface->format, &color.r, &color.g, &color.b);
    }
    return color;
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
        return obrazek;
    }
}

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
            std::unordered_set<Color> paletaSet;

            Canvas1D shuffledObrazek1D(obrazek1D);
            std::srand(std::time(nullptr));
            std::random_shuffle(shuffledObrazek1D.begin(), shuffledObrazek1D.end());

            for (const auto &c : shuffledObrazek1D) {
                paletaSet.insert(c);
                if (paletaSet.size() >= 32) break;
            }

            paleta = Canvas1D(paletaSet.begin(), paletaSet.end());

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
                applyFloydSteinbergDithering(obrazek, paleta, tryb == TrybObrazu::SzaroscDedykowana);
            } else {
                applyFloydSteinbergDithering(obrazek, tryb == TrybObrazu::SzaroscNarzucona);
            }
            break;
        }
        case Dithering::Bayer: {
            if(czyTrybJestZPaleta(tryb)) {
                applyBayerDithering5RGB(obrazek, paleta);
            } else {
                applyBayerDithering5RGB(obrazek, tryb == TrybObrazu::SzaroscNarzucona);
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

const char *NOFI_MAGIC = "NOFI";
const int NFVERSION = 1;


std::vector<Uint8> serializeHeader(NFHeaderUser header) {
    std::vector<Uint8> headerData;
    headerData.reserve(sizeof(NFHeader));

    // Setting Magic
    for (int i = 0; i < 4; i++) {
        headerData.push_back(NOFI_MAGIC[i]);
    }

    headerData.push_back(NFVERSION);
    headerData.push_back((Uint8) header.type);

    headerData.push_back(header.width & 0xFF);
    headerData.push_back((header.width >> 8) & 0xFF);

    headerData.push_back(header.height & 0xFF);
    headerData.push_back((header.height >> 8) & 0xFF);

    headerData.push_back(header.channel1HorizontalSubsampling);
    headerData.push_back(header.channel1VerticalSubsampling);
    headerData.push_back(header.channel2HorizontalSubsampling);
    headerData.push_back(header.channel2VerticalSubsampling);
    headerData.push_back(header.channel3HorizontalSubsampling);
    headerData.push_back(header.channel3VerticalSubsampling);

    return headerData;
}

NFHeader deserializeHeader(std::vector<Uint8> data) {
    NFHeader header;
    for (int i = 0; i < 4; i++) {
        header.magic[i] = data[i];

        if(header.magic[i] != NOFI_MAGIC[i]) {
            throw std::invalid_argument("Invalid NOFI file");
        }
    }

    header.version = data[4];

    if(header.version != NFVERSION) {
        throw std::invalid_argument("Invalid NOFI version. Expected " + std::to_string(NFVERSION) + " but got " + std::to_string(header.version));
    }

    header.type = (ImageType)data[5];

    header.width = data[6] | (data[7] << 8);
    header.height = data[8] | (data[9] << 8);

    header.channel1HorizontalSubsampling = data[10];
    header.channel1VerticalSubsampling = data[11];
    header.channel2HorizontalSubsampling = data[12];
    header.channel2VerticalSubsampling = data[13];
    header.channel3HorizontalSubsampling = data[14];
    header.channel3VerticalSubsampling = data[15];

    return header;
}