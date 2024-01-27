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

