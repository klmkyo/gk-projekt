#include <exception>
#include <string.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <SDL2/SDL.h>
#include <bitset>
using namespace std;

SDL_Window* window = NULL;
SDL_Surface* screen = NULL;

#define szerokosc 640
#define wysokosc 400

#define tytul "GK2023 - Projekt - Zespol 33"


void setPixel(int x, int y, Uint8 R, Uint8 G, Uint8 B);
SDL_Color getPixel (int x, int y);

Uint8 z24RGBna8RGB(SDL_Color kolor);
SDL_Color z8RGBna24RGB(Uint8 kolor8bit);

Uint8 z24RGBna5RGB(SDL_Color kolor);
SDL_Color z5RGBna24RGB(Uint8 kolor5bit);

void czyscEkran(Uint8 R, Uint8 G, Uint8 B);

void Funkcja1();
void Funkcja2();
void Funkcja3();
void Funkcja4();
void Funkcja5();
void Funkcja6();
void Funkcja7();
void Funkcja8();
void Funkcja9();

Uint8 z24RGBna8RGB(SDL_Color kolor) {
    Uint8 nowyR, nowyG, nowyB;
    nowyR = round(kolor.r*7.0/255.0);
    nowyG = round(kolor.g*7.0/255.0);
    nowyB = round(kolor.b*3.0/255.0);

    return (nowyR<<5) | (nowyG<<2) | (nowyB);
}

SDL_Color z8RGBna24RGB(Uint8 kolor8bit) {
    SDL_Color kolor;
    kolor.r = kolor8bit & 0b11100000;
    kolor.g = (kolor8bit & 0b00011100)<<3;
    kolor.b = (kolor8bit & 0b00000011)<<6;
    return kolor; 
}

Uint8 z24RGBna5RGB(SDL_Color kolor) {
    Uint8 nowyR, nowyG, nowyB;
    nowyR = round(kolor.r*3.0/255.0);
    nowyG = round(kolor.g*3.0/255.0);
    nowyB = round(kolor.b*1.0/255.0);

    return (nowyR<<6) | (nowyG<<4) | (nowyB<<3);
}

SDL_Color z5RGBna24RGB(Uint8 kolor5bit) {
    SDL_Color kolor;
    kolor.r = ((kolor5bit & 0b11000000)>>6)*255.0/3.0;
    kolor.g = ((kolor5bit & 0b00110000)>>4)*255.0/3.0;
    kolor.b = ((kolor5bit & 0b00001000)>>3)*255.0/1.0;

    return kolor; 
}

Uint8 z24Rgbna5BW(SDL_Color kolor) {
    int szary8bit = 0.299 * kolor.r + 0.587 * kolor.g + 0.114 * kolor.b;
    int szary5bit = round(szary8bit * 31.0/255.0);

    return szary5bit;
}

SDL_Color z5BWna24RGB(Uint8 kolor) {
    Uint8 szary8bit = round(kolor * 255.0/31.0);

    SDL_Color kolor24bit = {szary8bit, szary8bit, szary8bit};
    return kolor24bit;
}

void Funkcja1() {
    SDL_Color kolor;
    uint8_t R, G, B, nowyR, nowyG, nowyB;
    for (int y=0; y<wysokosc/2; y++){
        for (int x=0; x<szerokosc/2; x++) {
            kolor = getPixel(x, y);
            R = kolor.r;
            G = kolor.g;
            B = kolor.b;


            // 111 111 11

            // 
            nowyR = R>>5;
            nowyG = G>>5;
            nowyB = B>>6;

            R = nowyR<<5;
            G = nowyG<<5;
            B = nowyB<<6;
            setPixel(x + szerokosc/2, y, R, G, B);
         }
    }

    SDL_UpdateWindowSurface(window);
}

void Funkcja2() {
    SDL_Color kolor;
    uint8_t R, G, B, nowyR, nowyG, nowyB;
    for (int x=0; x<szerokosc/2; x++){
        for (int y=0; y<wysokosc/2; y++) {
            kolor = getPixel(x, y);
            R = kolor.r;
            G = kolor.g;
            B = kolor.b;


            // 111 111 11
            nowyR = R>>5;
            nowyG = G>>5;
            nowyB = B>>6;

            R = nowyR*255.0/7.0;
            G = nowyG*255.0/7.0;
            B = nowyB*255.0/3.0;
            setPixel(x, y+wysokosc/2, R, G, B);
         }
    }

    SDL_UpdateWindowSurface(window);
}

void Funkcja3() {
    SDL_Color kolor;
    uint8_t R, G, B, nowyR, nowyG, nowyB;
    int kolor8bit;
    for (int x=0; x<szerokosc/2; x++){
        for (int y=0; y<wysokosc/2; y++) {
            kolor = getPixel(x, y);
            R = kolor.r;
            G = kolor.g;
            B = kolor.b;


            // 111 111 11
            nowyR = round(R*7.0/255.0);
            nowyG = round(G*7.0/255.0);
            nowyB = round(B*3.0/255.0);

            kolor8bit = (nowyR<<5) | (nowyG<<2) | (nowyB);
            cout<<kolor8bit<<endl;

            R = nowyR*255.0/7.0;
            G = nowyG*255.0/7.0;
            B = nowyB*255.0/3.0;
            setPixel(x+szerokosc/2, y+wysokosc/2, R, G, B);
         }
    }

    SDL_UpdateWindowSurface(window);
}

void Funkcja4() {
    Uint8 kolor8bit;
    SDL_Color kolor, nowyKolor;

    for (int x=0; x<szerokosc/2; x++){
        for (int y=0; y<wysokosc/2; y++) {
            kolor = getPixel(x, y);
            kolor8bit = z24RGBna5RGB(kolor);
            nowyKolor = z5RGBna24RGB(kolor8bit);
            setPixel(x+szerokosc/2, y, nowyKolor.r, nowyKolor.g, nowyKolor.b);
         }
    }

    SDL_UpdateWindowSurface(window);
}

void Funkcja5() {

     SDL_Color kolor;
    kolor.r = 165;
    kolor.g = 210;
    kolor.b = 96;
    cout<<bitset<8>(kolor.r)<< " " << bitset<8>(kolor.g) << " " << bitset<8>(kolor.b) << endl;
    Uint8 kolor8bit = z24RGBna8RGB(kolor);
    cout<<bitset<8>(kolor8bit)<<endl;
    SDL_Color kolor2 = z8RGBna24RGB(kolor8bit);
    cout<<bitset<8>(kolor2.r)<< " " << bitset<8>(kolor2.g) << " " << bitset<8>(kolor2.b) << endl;


    SDL_UpdateWindowSurface(window);
}

void Funkcja6() {

    // zamienianie z 24bit na 3bit i z powrotem (wyświetlanie na rożnych kwadrantach)
    for (int y = 0; y < wysokosc/2; y++) {
        for (int x = 0; x < szerokosc/2; x++) {
            SDL_Color kolor = getPixel(x, y);

            Uint8 szary5bit = z24Rgbna5BW(kolor);
            SDL_Color kolor24bit = z5BWna24RGB(szary5bit);

            setPixel(x + szerokosc/2, y, kolor24bit.r, kolor24bit.g, kolor24bit.b);
        }
    }

    SDL_UpdateWindowSurface(window);
}

void Funkcja7() {

    //...

    SDL_UpdateWindowSurface(window);
}

void Funkcja8() {

    //...

    SDL_UpdateWindowSurface(window);
}

void Funkcja9() {

    //...

    SDL_UpdateWindowSurface(window);
}

void setPixel(int x, int y, Uint8 R, Uint8 G, Uint8 B)
{
  if ((x>=0) && (x<szerokosc) && (y>=0) && (y<wysokosc))
  {
    /* Zamieniamy poszczególne składowe koloru na format koloru piksela */
    Uint32 pixel = SDL_MapRGB(screen->format, R, G, B);

    /* Pobieramy informację ile bajtów zajmuje jeden piksel */
    int bpp = screen->format->BytesPerPixel;

    /* Obliczamy adres piksela */
    Uint8 *p1 = (Uint8 *)screen->pixels + (y*2) * screen->pitch + (x*2) * bpp;
    Uint8 *p2 = (Uint8 *)screen->pixels + (y*2+1) * screen->pitch + (x*2) * bpp;
    Uint8 *p3 = (Uint8 *)screen->pixels + (y*2) * screen->pitch + (x*2+1) * bpp;
    Uint8 *p4 = (Uint8 *)screen->pixels + (y*2+1) * screen->pitch + (x*2+1) * bpp;

    /* Ustawiamy wartość piksela, w zależnoœci od formatu powierzchni*/
    switch(bpp)
    {
        case 1: //8-bit
            *p1 = pixel;
            *p2 = pixel;
            *p3 = pixel;
            *p4 = pixel;
            break;

        case 2: //16-bit
            *(Uint16 *)p1 = pixel;
            *(Uint16 *)p2 = pixel;
            *(Uint16 *)p3 = pixel;
            *(Uint16 *)p4 = pixel;
            break;

        case 3: //24-bit
            if(SDL_BYTEORDER == SDL_BIG_ENDIAN) {
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

        case 4: //32-bit
            *(Uint32 *)p1 = pixel;
            *(Uint32 *)p2 = pixel;
            *(Uint32 *)p3 = pixel;
            *(Uint32 *)p4 = pixel;
            break;

        }
    }
}

void setPixelSurface(int x, int y, Uint8 R, Uint8 G, Uint8 B)
{
  if ((x>=0) && (x<szerokosc*2) && (y>=0) && (y<wysokosc*2))
  {
    /* Zamieniamy poszczególne składowe koloru na format koloru piksela */
    Uint32 pixel = SDL_MapRGB(screen->format, R, G, B);

    /* Pobieramy informację ile bajtów zajmuje jeden piksel */
    int bpp = screen->format->BytesPerPixel;

    /* Obliczamy adres piksela */
    Uint8 *p = (Uint8 *)screen->pixels + y * screen->pitch + x * bpp;

    /* Ustawiamy wartość piksela, w zależności od formatu powierzchni*/
    switch(bpp)
    {
        case 1: //8-bit
            *p = pixel;
            break;

        case 2: //16-bit
            *(Uint16 *)p = pixel;
            break;

        case 3: //24-bit
            if(SDL_BYTEORDER == SDL_BIG_ENDIAN) {
                p[0] = (pixel >> 16) & 0xff;
                p[1] = (pixel >> 8) & 0xff;
                p[2] = pixel & 0xff;
            } else {
                p[0] = pixel & 0xff;
                p[1] = (pixel >> 8) & 0xff;
                p[2] = (pixel >> 16) & 0xff;
            }
            break;

        case 4: //32-bit
            *(Uint32 *)p = pixel;
            break;
        }
    }
}

SDL_Color getPixel(int x, int y) {
    SDL_Color color ;
    Uint32 col = 0 ;
    if ((x>=0) && (x<szerokosc) && (y>=0) && (y<wysokosc)) {
        //określamy pozycję
        char* pPosition=(char*)screen->pixels ;

        //przesunięcie względem y
        pPosition+=(screen->pitch*y*2) ;

        //przesunięcie względem x
        pPosition+=(screen->format->BytesPerPixel*x*2);

        //kopiujemy dane piksela
        memcpy(&col, pPosition, screen->format->BytesPerPixel);

        //konwertujemy kolor
        SDL_GetRGB(col, screen->format, &color.r, &color.g, &color.b);
    }
    return ( color ) ;
}

SDL_Color getPixelSurface(int x, int y, SDL_Surface *surface) {
    SDL_Color color ;
    Uint32 col = 0 ;
    if ((x>=0) && (x<szerokosc) && (y>=0) && (y<wysokosc)) {
        //określamy pozycję
        char* pPosition=(char*)surface->pixels ;

        //przesunięcie względem y
        pPosition+=(surface->pitch*y) ;

        //przesunięcie względem x
        pPosition+=(surface->format->BytesPerPixel*x);

        //kopiujemy dane piksela
        memcpy(&col, pPosition, surface->format->BytesPerPixel);

        //konwertujemy kolor
        SDL_GetRGB(col, surface->format, &color.r, &color.g, &color.b);
    }
    return ( color ) ;
}


void ladujBMP(char const* nazwa, int x, int y)
{
    SDL_Surface* bmp = SDL_LoadBMP(nazwa);
    if (!bmp)
    {
        printf("Unable to load bitmap: %s\n", SDL_GetError());
    }
    else
    {
        SDL_Color kolor;
        for (int yy=0; yy<bmp->h; yy++) {
			for (int xx=0; xx<bmp->w; xx++) {
				kolor = getPixelSurface(xx, yy, bmp);
				setPixel(xx, yy, kolor.r, kolor.g, kolor.b);
			}
        }
		SDL_FreeSurface(bmp);
        SDL_UpdateWindowSurface(window);
    }

}


void czyscEkran(Uint8 R, Uint8 G, Uint8 B)
{
    SDL_FillRect(screen, 0, SDL_MapRGB(screen->format, R, G, B));
    SDL_UpdateWindowSurface(window);
}



int main(int argc, char* argv[]) {
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
		printf("SDL_Init Error: %s\n", SDL_GetError());
		return EXIT_FAILURE;
    }

    window = SDL_CreateWindow(tytul, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, szerokosc*2, wysokosc*2, SDL_WINDOW_SHOWN);

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
                cout << "esc" <<endl;
                done = true;
                break;

            // sprawdzamy czy został wciśnięty klawisz
            case SDL_KEYDOWN: {
                // wychodzimy, gdy wciśnięto ESC
                if (event.key.keysym.sym == SDLK_ESCAPE)
                    done = true;
                if (event.key.keysym.sym == SDLK_1)
                    Funkcja1();
                if (event.key.keysym.sym == SDLK_2)
                    Funkcja2();
                if (event.key.keysym.sym == SDLK_3)
                    Funkcja3();
                if (event.key.keysym.sym == SDLK_4)
                    Funkcja4();
                if (event.key.keysym.sym == SDLK_5)
                    Funkcja5();
                if (event.key.keysym.sym == SDLK_6)
                    Funkcja6();
                if (event.key.keysym.sym == SDLK_7)
                    Funkcja7();
                if (event.key.keysym.sym == SDLK_8)
                    Funkcja8();
                if (event.key.keysym.sym == SDLK_9)
                    Funkcja9();
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
