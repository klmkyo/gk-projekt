#pragma once

#include <exception>
#include <string.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include <cassert>

struct YUV {
    Uint8 y;
    Uint8 u;
    Uint8 v;
};

struct YIQ {
    Uint8 y;
    Uint8 i;
    Uint8 q;
};

struct YCbCr {
    Uint8 y;
    Uint8 cb;
    Uint8 cr;
};

struct HSL {
    Uint8 h; 
    Uint8 s; 
    Uint8 l; 
};


void setYUV(int xx, int yy, Uint8 y, Uint8 u, Uint8 v);
YUV getYUV(int xx, int yy);
YUV rgbToYuv(int r, int g, int b);
SDL_Color yuvToRgb(Uint8 y, Uint8 u, Uint8 v);


void setYIQ(int xx, int yy, Uint8 y, Uint8 i, Uint8 q);
YIQ getYIQ(int xx, int yy);
YIQ rgbToYiq(int r, int g, int b);
SDL_Color yiqToRgb(Uint8 y, Uint8 i, Uint8 q);

void setYCbCr(int xx, int yy, Uint8 y, Uint8 cb, Uint8 cr);
YCbCr getYCbCr(int xx, int yy); 

YCbCr rgbToYcbcr(int r, int g, int b);
SDL_Color ycbcrToRgb(int y, int cb, int cr);

HSL rgbToHsl(int r, int g, int b);
SDL_Color hslToRgb(Uint8 h, Uint8 s, Uint8 l);

void setHSL(int x, int y, float h, float s, float l);
HSL getHSL(int x, int y);

Uint16 sdlColorToRGB555(SDL_Color color);
SDL_Color RGB555ToSdlColor(Uint16 rgb555);
Uint16 sdlColorToRGB565(SDL_Color color);
SDL_Color RGB565ToSdlColor(Uint16 rgb565);

void setRGB555(int xx, int yy, Uint8 r, Uint8 g, Uint8 b);
void setRGB555(int xx, int yy, Uint16 rgb555);
SDL_Color getRGB555(int xx, int yy);
Uint16 getRGB555_(int xx, int yy);
void setRGB565(int xx, int yy, Uint8 r, Uint8 g, Uint8 b);
void setRGB565(int xx, int yy, Uint16 rgb565);
SDL_Color getRGB565(int xx, int yy);
Uint16 getRGB565_(int xx, int yy);