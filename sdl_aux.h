
#ifndef SDL_AUX_H
#define SDL_AUX_H

#include <stdbool.h>

#include <SDL/SDL.h>
#include <SDL/SDL_ttf.h>

static inline void set_pixel(int x, int y, Uint8 r, Uint8 g, Uint8 b)
{
    SDL_Surface *surface = SDL_GetVideoSurface();
    Uint8 bpp = surface->format->BytesPerPixel;
    Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;
    Uint32 pixel = SDL_MapRGB(surface->format, r, g, b);

    switch (bpp) {
    case 1:
        *p = pixel;
        break;
    case 2:
        *(Uint16 *)p = pixel;
        break;
    case 3:
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
    case 4:
        *(Uint32 *)p = pixel;
        break;
    }
}

static inline void get_pixel(int x, int y, Uint8 *r, Uint8 *g, Uint8 *b)
{
    SDL_Surface *surface = SDL_GetVideoSurface();
    Uint8 bpp = surface->format->BytesPerPixel;
    Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;
    Uint32 pixel;

    switch (bpp) {
    case 1:
        pixel = *p;
        break;
    case 2:
        pixel = *(Uint16 *)p;
        break;
    case 3:
        if(SDL_BYTEORDER == SDL_BIG_ENDIAN)
            pixel = p[0] << 16 | p[1] << 8 | p[2];
        else
            pixel = p[0] | p[1] << 8 | p[2] << 16;
        break;
    case 4:
        pixel = *(Uint32 *)p;
        break;
    default:
        pixel = 0;
        break;
    }

    SDL_GetRGB(pixel, surface->format, r, g, b);
}

static inline void draw_block(int x, int y, int w, int h, Uint8 r, Uint8 g, Uint8 b)
{
    SDL_Rect rect;
    rect.x = x;
    rect.y = y;
    rect.w = w;
    rect.h = h;
    SDL_FillRect(SDL_GetVideoSurface(), &rect,
                 SDL_MapRGB(SDL_GetVideoSurface()->format, r, g, b));
}

void init_sdl(unsigned w, unsigned h, const char *title);
void uninit_sdl(void);

void grab_events(void);
bool is_key_down(int c);
bool is_closed(void);

char *start_text_input(void);
char *start_text_cmd_input(void);
void stop_text_input(void);
bool is_text_input(void);

bool set_font(const char *ttf, int ptsize);
void draw_string(int x, int y, Uint8 r, Uint8 g, Uint8 b, const char *str);
void draw_string_centered(int x, int y, Uint8 r, Uint8 g, Uint8 b, const char *str);
void draw_string_right(int x, int y, Uint8 r, Uint8 g, Uint8 b, const char *str);
bool get_string_size(const char *str, int *w, int *h);

#endif
