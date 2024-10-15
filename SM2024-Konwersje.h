#pragma once

#include <exception>
#include <string.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include <cassert>

// Struct Definitions
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
    Uint8 h; // Hue: 0-255 representing 0-360 degrees
    Uint8 s; // Saturation: 0-255 representing 0-1
    Uint8 l; // Lightness: 0-255 representing 0-1
};

// YUV Conversion Functions
void setYUV(int xx, int yy, Uint8 y, Uint8 u, Uint8 v);
YUV getYUV(int xx, int yy);
YUV rgbToYuv(int r, int g, int b);
SDL_Color yuvToRgb(Uint8 y, Uint8 u, Uint8 v);

// YIQ Conversion Functions
void setYIQ(int xx, int yy, Uint8 y, Uint8 i, Uint8 q);
YIQ getYIQ(int xx, int yy);
YIQ rgbToYiq(int r, int g, int b);
SDL_Color yiqToRgb(Uint8 y, Uint8 i, Uint8 q);

// YCbCr Conversion Functions
void setYCbCr(int xx, int yy, Uint8 y, Uint8 cb, Uint8 cr);
YCbCr getYCbCr(int xx, int yy); // Corrected from getCbCr to getYCbCr
YCbCr rgbToYcbcr(int r, int g, int b);
SDL_Color ycbcrToRgb(int y, int cb, int cr);

// HSL Conversion Functions
HSL rgbToHsl(int r, int g, int b);
SDL_Color hslToRgb(Uint8 h, Uint8 s, Uint8 l);

void setHSL(int x, int y, float h, float s, float l);
HSL getHSL(int x, int y);