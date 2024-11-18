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
    headerData.push_back((Uint8)header.type);
    headerData.push_back((Uint8)header.filter);
    headerData.push_back((Uint8)header.compression);

    headerData.push_back(header.width & 0xFF);
    headerData.push_back((header.width >> 8) & 0xFF);

    headerData.push_back(header.height & 0xFF);
    headerData.push_back((header.height >> 8) & 0xFF);

    headerData.push_back(header.subsamplingEnabled);

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
    header.filter = (FilterType)data[6];
    header.compression = (CompressionType)data[7];

    header.width = data[8] | (data[9] << 8);
    header.height = data[10] | (data[11] << 8);

    header.subsamplingEnabled = data[10];

    return header;
}

void saveNFImage(const std::string &filename, NFHeaderUser header, Canvas &image) {
    // TODO
}

std::pair<NFHeader, Canvas> loadNFImage(const std::string &filename) {
    // TODO
}


template <typename T>
T clamp(const T& value, const T& min, const T& max) {
    if (value < min) return min;
    if (value > max) return max;
    return value;
}


std::vector<Uint8> serializeCanvas(Canvas &image, ImageType type, bool subsamplingEnabled) {
    std::vector<Uint8> data;
    int width = image[0].size();
    int height = image.size();

    if (type == ImageType::RGB555) {
        // Convert each pixel to RGB555 format
        for (const auto &row : image) {
            for (const auto &pixel : row) {
                Uint16 rgb555 = ((pixel.r & 0xF8) << 7) | ((pixel.g & 0xF8) << 2) | (pixel.b >> 3);
                data.push_back(rgb555 & 0xFF);
                data.push_back((rgb555 >> 8) & 0xFF);
            }
        }
    } else if (type == ImageType::RGB565) {
        // Convert each pixel to RGB565 format
        for (const auto &row : image) {
            for (const auto &pixel : row) {
                Uint16 rgb565 = ((pixel.r & 0xF8) << 8) | ((pixel.g & 0xFC) << 3) | (pixel.b >> 3);
                data.push_back(rgb565 & 0xFF);
                data.push_back((rgb565 >> 8) & 0xFF);
            }
        }
    } else if (type == ImageType::YUV) {
        // Convert RGB to YUV and handle subsampling if enabled
        std::vector<std::vector<Uint8>> Y(height, std::vector<Uint8>(width));
        std::vector<std::vector<Uint8>> U(height, std::vector<Uint8>(width));
        std::vector<std::vector<Uint8>> V(height, std::vector<Uint8>(width));

        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                Color &pixel = image[y][x];
                float r = pixel.r / 255.0f;
                float g = pixel.g / 255.0f;
                float b = pixel.b / 255.0f;

                float Yf = 0.299f * r + 0.587f * g + 0.114f * b;
                float Uf = -0.14713f * r - 0.28886f * g + 0.436f * b + 0.5f;
                float Vf = 0.615f * r - 0.51499f * g - 0.10001f * b + 0.5f;

                Y[y][x] = static_cast<Uint8>(Yf * 255);
                U[y][x] = static_cast<Uint8>(Uf * 255);
                V[y][x] = static_cast<Uint8>(Vf * 255);
            }
        }

        if (subsamplingEnabled) {
            // 4:2:0 subsampling
            int subsampledWidth = (width + 1) / 2;
            int subsampledHeight = (height + 1) / 2;
            std::vector<std::vector<Uint8>> U_sub(subsampledHeight, std::vector<Uint8>(subsampledWidth));
            std::vector<std::vector<Uint8>> V_sub(subsampledHeight, std::vector<Uint8>(subsampledWidth));

            for (int y = 0; y < subsampledHeight; ++y) {
                for (int x = 0; x < subsampledWidth; ++x) {
                    int sumU = 0, sumV = 0, count = 0;
                    for (int dy = 0; dy < 2; ++dy) {
                        for (int dx = 0; dx < 2; ++dx) {
                            int yy = y * 2 + dy;
                            int xx = x * 2 + dx;
                            if (yy < height && xx < width) {
                                sumU += U[yy][xx];
                                sumV += V[yy][xx];
                                ++count;
                            }
                        }
                    }
                    U_sub[y][x] = sumU / count;
                    V_sub[y][x] = sumV / count;
                }
            }

            // Store Y, U_sub, and V_sub
            for (const auto &row : Y)
                data.insert(data.end(), row.begin(), row.end());
            for (const auto &row : U_sub)
                data.insert(data.end(), row.begin(), row.end());
            for (const auto &row : V_sub)
                data.insert(data.end(), row.begin(), row.end());
        } else {
            // No subsampling
            for (const auto &row : Y)
                data.insert(data.end(), row.begin(), row.end());
            for (const auto &row : U)
                data.insert(data.end(), row.begin(), row.end());
            for (const auto &row : V)
                data.insert(data.end(), row.begin(), row.end());
        }
    }
    // Implement other ImageTypes (YIQ, YCbCr, HSL) similarly
    else {
        throw std::invalid_argument("Unsupported ImageType in serializeCanvas");
    }
    return data;
}

Canvas deserializeCanvas(std::vector<Uint8> data, NFHeader header) {
    Canvas image(header.height, std::vector<Color>(header.width));
    int width = header.width;
    int height = header.height;

    if (header.type == ImageType::RGB555) {
        // Reconstruct image from RGB555 data
        int idx = 0;
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                Uint16 rgb555 = data[idx] | (data[idx + 1] << 8);
                idx += 2;
                Uint8 r = ((rgb555 >> 7) & 0xF8) | ((rgb555 >> 12) & 0x07);
                Uint8 g = ((rgb555 >> 2) & 0xF8) | ((rgb555 >> 7) & 0x07);
                Uint8 b = ((rgb555 << 3) & 0xF8) | ((rgb555 >> 2) & 0x07);
                image[y][x] = {r, g, b};
            }
        }
    } else if (header.type == ImageType::RGB565) {
        // Reconstruct image from RGB565 data
        int idx = 0;
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                Uint16 rgb565 = data[idx] | (data[idx + 1] << 8);
                idx += 2;
                Uint8 r = ((rgb565 >> 8) & 0xF8) | ((rgb565 >> 13) & 0x07);
                Uint8 g = ((rgb565 >> 3) & 0xFC) | ((rgb565 >> 9) & 0x03);
                Uint8 b = ((rgb565 << 3) & 0xF8) | ((rgb565 >> 2) & 0x07);
                image[y][x] = {r, g, b};
            }
        }
    } else if (header.type == ImageType::YUV) {
        // Reconstruct image from YUV data
        int idx = 0;
        std::vector<std::vector<Uint8>> Y(height, std::vector<Uint8>(width));
        std::vector<std::vector<Uint8>> U, V;

        if (header.subsamplingEnabled) {
            int subsampledWidth = (width + 1) / 2;
            int subsampledHeight = (height + 1) / 2;
            U.resize(subsampledHeight, std::vector<Uint8>(subsampledWidth));
            V.resize(subsampledHeight, std::vector<Uint8>(subsampledWidth));

            // Read Y channel
            for (auto &row : Y)
                for (auto &val : row)
                    val = data[idx++];

            // Read subsampled U and V
            for (auto &row : U)
                for (auto &val : row)
                    val = data[idx++];
            for (auto &row : V)
                for (auto &val : row)
                    val = data[idx++];

            // Upsample U and V
            std::vector<std::vector<Uint8>> U_full(height, std::vector<Uint8>(width));
            std::vector<std::vector<Uint8>> V_full(height, std::vector<Uint8>(width));

            for (int y = 0; y < height; ++y) {
                int yy = y / 2;
                for (int x = 0; x < width; ++x) {
                    int xx = x / 2;
                    U_full[y][x] = U[yy][xx];
                    V_full[y][x] = V[yy][xx];
                }
            }
            U = std::move(U_full);
            V = std::move(V_full);
        } else {
            U.resize(height, std::vector<Uint8>(width));
            V.resize(height, std::vector<Uint8>(width));

            // Read Y, U, and V channels
            for (auto &row : Y)
                for (auto &val : row)
                    val = data[idx++];
            for (auto &row : U)
                for (auto &val : row)
                    val = data[idx++];
            for (auto &row : V)
                for (auto &val : row)
                    val = data[idx++];
        }

        // Convert YUV to RGB
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                float Yf = Y[y][x] / 255.0f;
                float Uf = (U[y][x] / 255.0f) - 0.5f;
                float Vf = (V[y][x] / 255.0f) - 0.5f;

                float r = Yf + 1.13983f * Vf;
                float g = Yf - 0.39465f * Uf - 0.58060f * Vf;
                float b = Yf + 2.03211f * Uf;

                image[y][x].r = static_cast<Uint8>(clamp(r, 0.0f, 1.0f) * 255);
                image[y][x].g = static_cast<Uint8>(clamp(g, 0.0f, 1.0f) * 255);
                image[y][x].b = static_cast<Uint8>(clamp(b, 0.0f, 1.0f) * 255);
            }
        }
    }
    // Implement other ImageTypes (YIQ, YCbCr, HSL) similarly
    else {
        throw std::invalid_argument("Unsupported ImageType in deserializeCanvas");
    }
    return image;
}
