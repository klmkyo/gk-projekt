#include "SM2024-Konwersje.h"
#include "SM2024-Gui.h"
#include <algorithm>

Uint8 clamp(int value) {
    return (Uint8)(std::max(0, std::min(255, value)));
}


// 0.299 0.587 0.114
// -0.14713 -0.28886 0.436
// 0.615 -0.51499 -0.10001
YUV rgbToYuv(int r, int g, int b) {
    Uint8 y = clamp((int)(r * 0.299    + g * 0.587    + b * 0.114));
    Uint8 u = clamp((int)(r * -0.14713 + g * -0.28886 + b * 0.436) + 128);
    Uint8 v = clamp((int)(r * 0.615    + g * -0.51499 + b * -0.10001) + 128);
    return { y, u, v };
}

// 1 0 1.13983
// 1 -0.39465 -0.58060
// 1 2.03211 0
SDL_Color yuvToRgb(Uint8 y, Uint8 u, Uint8 v) {
    int u_adj = u - 128;
    int v_adj = v - 128;

    Uint8 r = clamp((int)(y + 1.13983 * v_adj));
    Uint8 g = clamp((int)(y - 0.39465 * u_adj - 0.58060 * v_adj));
    Uint8 b = clamp((int)(y + 2.03211 * u_adj));

    return { r, g, b };
}


// 0.299 0.587 0.114
// 0.5959 -0.2746 -0.3213
// 0.2115 -0.5227 0.3112
YIQ rgbToYiq(int r, int g, int b) {
    Uint8 y = clamp((int)(r * 0.299 + g * 0.587 + b * 0.114));
    Uint8 i = clamp((int)128 + (int)(r * 0.5959 + g * -0.2746 + b * -0.3213));
    Uint8 q = clamp((int)128 + (int)(r * 0.2115 + g * -0.5227 + b * 0.3112));

    YIQ yiq = { y, i, q };
    return yiq;
}

// 1 0.956 0.619
// 1 -0.272 -0.647
// 1 -1.106 1.703
SDL_Color yiqToRgb(Uint8 y, Uint8 i, Uint8 q) {
    int adj_i = i - 128;
    int adj_q = q - 128;

    Uint8 r = clamp(y + 0.956  * adj_i + 0.619 * adj_q);
    Uint8 g = clamp(y + -0.272 * adj_i + -0.647 * adj_q);
    Uint8 b = clamp(y + -1.106 * adj_i + 1.703 * adj_q);

    SDL_Color color = {r, g, b};
    return color;
}


// Y'  = 0 + (0.299 * R'_D) + (0.587 * G'_D) + (0.114 * B'_D)
// CB  = 128 - (0.168736 * R'_D) - (0.331264 * G'_D) + (0.5 * B'_D)
// CR  = 128 + (0.5 * R'_D) - (0.418688 * G'_D) - (0.081312 * B'_D)
YCbCr rgbToYcbcr(int r, int g, int b) {
    Uint8 y = clamp((int)(0 + (0.299 * r) + (0.587 * g) + (0.114 * b)));
    Uint8 cb = clamp((int)(128 - (0.168736 * r) - (0.331264 * g) + (0.5 * b)));
    Uint8 cr = clamp((int)(128 + (0.5 * r) - (0.418688 * g) - (0.081312 * b)));

    YCbCr ycbcr = {y, cb, cr};
    return ycbcr;
}
// R'_D = Y' + 1.402 * (CR - 128)
// G'_D = Y' - 0.344136 * (CB - 128) - 0.714136 * (CR - 128)
// B'_D = Y' + 1.772 * (CB - 128)
SDL_Color ycbcrToRgb(int y, int cb, int cr) {
    Uint8 r = clamp((int)(y + 1.402 * (cr - 128)));
    Uint8 g = clamp((int)(y - 0.344136 * (cb - 128) - 0.714136 * (cr - 128)));
    Uint8 b = clamp((int)(y + 1.772 * (cb - 128)));

    SDL_Color color = {r, g, b};
    return color;
}


void setYUV(int xx, int yy, Uint8 y, Uint8 u, Uint8 v) {
    SDL_Color color = yuvToRgb(y, u, v);
    setPixel(xx, yy, color.r, color.g, color.b);
}
YUV getYUV(int xx, int yy) {
    SDL_Color color = getPixel(xx, yy);
    return rgbToYuv(color.r, color.g, color.b);
}

void setYIQ(int xx, int yy, Uint8 y, Uint8 i, Uint8 q) {
    SDL_Color color = yiqToRgb(y, i, q);
    setPixel(xx, yy, color.r, color.g, color.b);
}
YIQ getYIQ(int xx, int yy) {
    SDL_Color color = getPixel(xx, yy);
    return rgbToYiq(color.r, color.g, color.b);
}

void setYCbCr(int xx, int yy, Uint8 y, Uint8 cb, Uint8 cr) {
    SDL_Color color = ycbcrToRgb(y, cb, cr);
    setPixel(xx, yy, color.r, color.g, color.b);
}
YCbCr getYCbCr(int xx, int yy) {
    SDL_Color color = getPixel(xx, yy);
    return rgbToYcbcr(color.r, color.g, color.b);
}



// Function to convert RGB to HSL with Uint8 components
HSL rgbToHsl(int in_r, int in_g, int in_b) {
    float r = in_r / 255.0f;
    float g = in_g / 255.0f;
    float b = in_b / 255.0f;

    float max_val = std::max(r, std::max(g, b));
    float min_val = std::min(r, std::min(g, b));

    float l = (max_val + min_val) / 2.0f;
    float s = 0.0f;
    float h = 0.0f;

    if (min_val != max_val) {
        if (l < 0.5f) {
            s = (max_val - min_val) / (max_val + min_val);
        } else {
            s = (max_val - min_val) / (2.0f - max_val - min_val);
        }

        if (max_val == r) {
            h = (g - b) / (max_val - min_val);
        } else if (max_val == g) {
            h = 2.0f + (b - r) / (max_val - min_val);
        } else {
            h = 4.0f + (r - g) / (max_val - min_val);
        }

        h *= 60.0f;
        if (h < 0.0f) h += 360.0f;
    }

    // Scale H, S, L to Uint8
    HSL hsl;
    hsl.h = (Uint8)((h / 360.0f) * 255.0f);
    hsl.s = (Uint8)(s * 255.0f);
    hsl.l = (Uint8)(l * 255.0f);

    return hsl;
}

// Helper function to fix hue values
float fix(float value) {
    if (value < 0.0f) return value + 1.0f;
    if (value > 1.0f) return value - 1.0f;
    return value;
}

float third = 1.0f / 3.0f;

// Process function remains unchanged
float process(float var1, float var2, float in) {
    if (6.0f * in < 1.0f) {
        return var2 + (var1 - var2) * 6.0f * in; 
    } else if (2.0f * in < 1.0f) {
        return var1;
    } else if (3.0f * in < 2.0f) {
        return var2 + (var1 - var2) * (2.0f / 3.0f - in) * 6.0f;
    }
    return var2;
}

// Function to convert HSL with Uint8 components to RGB
SDL_Color hslToRgb(Uint8 in_h, Uint8 in_s, Uint8 in_l) {
    // Convert Uint8 HSL to float
    float h = ((float)in_h / 255.0f) * 360.0f;
    float s = (float)in_s / 255.0f;
    float l = (float)in_l / 255.0f;

    float var1;
    if (l < 0.5f) {
        var1 = l * (1.0f + s);
    } else {
        var1 = (l + s) - (l * s);
    }

    float var2 = 2.0f * l - var1;

    float h_norm = h / 360.0f;
    float r_norm = fix(h_norm + third);
    float g_norm = fix(h_norm);
    float b_norm = fix(h_norm - third);

    SDL_Color color;
    color.r = (Uint8)(process(var1, var2, r_norm) * 255.0f);
    color.g = (Uint8)(process(var1, var2, g_norm) * 255.0f);
    color.b = (Uint8)(process(var1, var2, b_norm) * 255.0f);
    color.a = 255; // Assuming full opacity

    return color;
}


void setHSL(int x, int y, float h, float s, float l) {
    SDL_Color color = hslToRgb(h, s, l);
    setPixel(x, y, color.r, color.g, color.b);
}

HSL getHSL(int x, int y) {
    SDL_Color color = getPixel(x, y);
    return rgbToHsl(color.r, color.g, color.b);
}

Uint16 sdlColorToRGB555(SDL_Color color) {
    Uint8 r = color.r >> 3;
    Uint8 g = color.g >> 3;
    Uint8 b = color.b >> 3;

    Uint16 rgb555 = 0;
    rgb555 |= r;
    rgb555 |= (g << 5);
    rgb555 |= (b << 10);

    return rgb555;
}

SDL_Color RGB555ToSdlColor(Uint16 rgb555) {
    Uint8 r = rgb555 & 0b11111;
    Uint8 g = (rgb555 >> 5) & 0b11111;
    Uint8 b = (rgb555 >> 10) & 0b11111;

    r = r << 3;
    g = g << 3;
    b = b << 3;

    return SDL_Color { r, g, b };
}

Uint16 sdlColorToRGB565(SDL_Color color) {
    Uint8 r = color.r >> 3;
    Uint8 g = color.g >> 2;
    Uint8 b = color.b >> 3;

    Uint16 rgb565 = 0;
    rgb565 |= r;
    rgb565 |= (g << 5);
    rgb565 |= (b << 11);

    return rgb565;

}

SDL_Color RGB565ToSdlColor(Uint16 rgb565) {
    Uint8 r = rgb565 & 0b11111;
    Uint8 g = (rgb565 >> 5) & 0b111111;
    Uint8 b = (rgb565 >> 11) & 0b11111;

    r = r << 3;
    g = g << 2;
    b = b << 3;

    return SDL_Color { r, g, b };
}

void setRGB555(int xx, int yy, Uint8 r, Uint8 g, Uint8 b) {
    Uint16 rgb555 = sdlColorToRGB555(SDL_Color { r, g, b });
    setRGB555(xx, yy, rgb555);
}
void setRGB555(int xx, int yy, Uint16 rgb555) {
    SDL_Color color = RGB555ToSdlColor(rgb555);
    setPixel(xx, yy, color.r, color.g, color.b);
}
SDL_Color getRGB555(int xx, int yy) {
    SDL_Color color = getPixel(xx, yy);
    return RGB555ToSdlColor(sdlColorToRGB555(color));
}
Uint16 getRGB555_(int xx, int yy) {
    SDL_Color color = getPixel(xx, yy);
    return sdlColorToRGB555(color);
}

void setRGB565(int xx, int yy, Uint8 r, Uint8 g, Uint8 b) {
    Uint16 rgb565 = sdlColorToRGB565(SDL_Color { r, g, b });
    setRGB565(xx, yy, rgb565);
}
void setRGB565(int xx, int yy, Uint16 rgb565) {
    SDL_Color color = RGB565ToSdlColor(rgb565);
    setPixel(xx, yy, color.r, color.g, color.b);
}
SDL_Color getRGB565(int xx, int yy) {
    SDL_Color color = getPixel(xx, yy);
    return RGB565ToSdlColor(sdlColorToRGB565(color));
}
Uint16 getRGB565_(int xx, int yy) {
    SDL_Color color = getPixel(xx, yy);
    return sdlColorToRGB565(color);
}