// podstawowe funkcje
#include "SM2024-Funkcje.h"
#include "SM2024-Zmienne.h"
#include "SM2024-Paleta.h"
#include "SM2024-MedianCut.h"
#include "SM2024-Pliki.h"
#include "SM2024-Konwersje.h"
#include "SM2024-Gui.h"
#include "SM2024-Kompresja.h"


void czyscEkran(Uint8 R, Uint8 G, Uint8 B)
{
    SDL_FillRect(screen, 0, SDL_MapRGB(screen->format, R, G, B));
    SDL_UpdateWindowSurface(window);
}

void Funkcja1() {
    int nieskompresowane[] = {0, 0, 0, 1, 1, 1, 1, 2, 0, 0, 3, 1, 3, 2, 2, 0, 0, 0, 3, 3, 3, 3, 1, 2, 1, 2, 3, 1, 2, 0, 0, 1, 1, 1, 3, 3};
    int dlugosc = 36;
    
    std::cout<<"wejscie:"<<std::endl;
    for (int c=0; c<dlugosc; c++)
        std::cout<<(int)nieskompresowane[c]<<" ";
    std::cout<<"\n";

    auto result = LZWKompresja(nieskompresowane, dlugosc);
    auto skompresowane = result.first;
    auto slownik = result.second;
    std::cout<<"\n";

    std::cout<<"skompresowane:"<<std::endl;
    for (int c=0; c<skompresowane.size(); c++)
        std::cout<<skompresowane[c]<<" ";
    std::cout<<"\n";

    auto zdekompresowane = LZWDekompresja(skompresowane, slownik);
    
    std::cout<<"zdekompresowane:"<<std::endl;
    for (int c=0; c<zdekompresowane.size(); c++)
        std::cout<<zdekompresowane[c]<<" ";
    std::cout<<"\n";

    // check if equal
    for (int c=0; c<dlugosc; c++) {
        if (nieskompresowane[c] != zdekompresowane[c]) {
            std::cout<<"ERROR: "<<c<<std::endl;
            break;
        }
    }

    SDL_UpdateWindowSurface(window);
}

void Funkcja2() {

    Canvas canvas = getCanvasFromPixels(0, 0, szerokosc / 2, wysokosc / 2);

    applyBayerDithering16RGB(canvas);

    paintCanvasInCorner(canvas, 1, 0);

    SDL_UpdateWindowSurface(window);
}

void Funkcja3() {

    // Kopiowanie obrazu z RGB565
    for (int yy = 0; yy < wysokosc / 2; yy++) {
        for (int xx = 0; xx < szerokosc / 2; xx++) {
            Uint16 rgb565 = getRGB565_(xx, yy);
            setRGB565(xx + szerokosc / 2, yy, rgb565);
        }
    }

    SDL_UpdateWindowSurface(window);
}

void Funkcja4() {

    // Rozbicie RGB565 na składowe
    for (int yy = 0; yy < wysokosc / 2; yy++) {
        for (int xx = 0; xx < szerokosc / 2; xx++) {
            Uint16 rgb565 = getRGB565_(xx, yy);
            Uint8 r = (rgb565 & 0b11111) * 8;
            Uint8 g = ((rgb565 >> 5) & 0b111111) * 4;
            Uint8 b = ((rgb565 >> 11) & 0b11111) * 8;
            setPixel(xx + szerokosc / 2, yy, r, r, r);
            setPixel(xx + szerokosc / 2, yy + wysokosc / 2, g, g, g);
            setPixel(xx, yy + wysokosc / 2, b, b, b);
        }
    }

    SDL_UpdateWindowSurface(window);
}


void Funkcja5() {

     for (int yy=0; yy<wysokosc/2; yy++) {
        for (int xx=0; xx<szerokosc/2; xx++) {
            YIQ yiq = getYIQ(xx, yy);
            setPixel(xx + szerokosc/2, yy, yiq.y, yiq.y, yiq.y);
            setPixel(xx, yy + wysokosc/2, yiq.i, yiq.i, yiq.i);
            setPixel(xx + szerokosc/2, yy + wysokosc/2, yiq.q, yiq.q, yiq.q);
        }
    }

    SDL_UpdateWindowSurface(window);
}

void Funkcja6() {

    for (int yy=0; yy<wysokosc/2; yy++) {
        for (int xx=0; xx<szerokosc/2; xx++) {
            YIQ yiq = getYIQ(xx, yy);
            setYIQ(xx + szerokosc/2, yy, yiq.y, yiq.i, yiq.q);
        }
    }


    SDL_UpdateWindowSurface(window);
}

void Funkcja7() {

     for (int yy=0; yy<wysokosc/2; yy++) {
        for (int xx=0; xx<szerokosc/2; xx++) {
            YCbCr ycbcr = getYCbCr(xx, yy);
            setPixel(xx + szerokosc/2, yy, ycbcr.y, ycbcr.y, ycbcr.y);
            setPixel(xx, yy + wysokosc/2, ycbcr.cb, ycbcr.cb, ycbcr.cb);
            setPixel(xx + szerokosc/2, yy + wysokosc/2, ycbcr.cr, ycbcr.cr, ycbcr.cr);
        }
    }


    SDL_UpdateWindowSurface(window);
}

void Funkcja8() {

    for (int yy=0; yy<wysokosc/2; yy++) {
        for (int xx=0; xx<szerokosc/2; xx++) {
            YCbCr ycbcr = getYCbCr(xx, yy);
            setYCbCr(xx + szerokosc/2, yy, ycbcr.y, ycbcr.cb, ycbcr.cr);
        }
    }


    SDL_UpdateWindowSurface(window);
}

void Funkcja9() {

    // save top left quarter to canvas
    Canvas canvas = getCanvasFromPixels(0, 0, szerokosc / 2, wysokosc / 2);

    NFHeaderUser header;
    header.type = ImageType::RGB565;
    header.filter = FilterType::None;
    header.compression = CompressionType::None;
    header.width = canvas[0].size();
    header.height = canvas.size();
    header.subsamplingEnabled = false;

    saveNFImage("test.nf", header, canvas);

    // read the image back, and put it in top right
    auto [loadedHeader, loadedImage] = loadNFImage("test.nf");

    // DEBUG print image dimensions
    std::cout << "Loaded image dimensions: " << loadedImage[0].size() << "x" << loadedImage.size() << std::endl;

    paintCanvasFromOrigin(loadedImage, szerokosc / 2, 0);

    SDL_UpdateWindowSurface(window);
}

Canvas getCanvasFromPixels(int x, int y, int width, int height) {
    Canvas canvas;
    canvas.reserve(height);
    for (int yy = y; yy < y + height; yy++) {
        std::vector<Color> row;
        row.reserve(width);
        for (int xx = x; xx < x + width; xx++) {
            SDL_Color sdlColor = getPixel(xx, yy);
            Color color = {sdlColor.r, sdlColor.g, sdlColor.b};
            row.push_back(color);
        }
        canvas.push_back(row);
    }
    return canvas;
}

void paintCanvasFromOrigin(const Canvas &canvas, int tlx, int tly) {
    for (int yy = 0; yy < canvas.size(); yy++) {
        for (int xx = 0; xx < canvas[0].size(); xx++) {
            setPixel(xx + tlx, yy + tly, canvas[yy][xx].r, canvas[yy][xx].g, canvas[yy][xx].b);
        }
    }
}

void paintCanvasInCorner(const Canvas &canvas, int l, int b) {
    int x = l * szerokosc / 2;
    int y = b * wysokosc / 2;
    for (int yy = 0; yy < canvas.size(); yy++) {
        for (int xx = 0; xx < canvas[0].size(); xx++) {
            setPixel(xx + x, yy + y, canvas[yy][xx].r, canvas[yy][xx].g, canvas[yy][xx].b);
        }
    }
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


void ladujBMP(char const* nazwa, int x, int y) {
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

int startupGui() {
  printf("Odpalam Mistrza GUI\n");

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
        return 1;
    }
    SDL_UpdateWindowSurface(window);


    bool done = false;
    SDL_Event event;
    // główna pętla programu
    while (SDL_WaitEvent(&event)) {
        // sprawdzamy czy pojawiło się zdarzenie
        switch (event.type) {
            case SDL_QUIT:
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
}