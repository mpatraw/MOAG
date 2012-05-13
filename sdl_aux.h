
#ifndef SDL_AUX_H
#define SDL_AUX_H

#include <stdbool.h>

#include <SDL/SDL.h>
#include <SDL/SDL_ttf.h>

typedef Uint32 color_type;

#define RED(c) (((c) >> 16) & 0xff)
#define GREEN(c) (((c) >> 8) & 0xff)
#define BLUE(c) (((c) >> 0) & 0xff)

#define COLOR(r, g, b) \
    (((r) & 0xff) << 16 | \
     ((g) & 0xff) <<  8 | \
     ((b) & 0xff) <<  0)

#define COLOR_BLACK     COLOR(  0,   0,   0)
#define COLOR_WHITE     COLOR(255, 255, 255)
#define COLOR_RED       COLOR(255,   0,   0)
#define COLOR_GREEN     COLOR(  0, 255,   0)
#define COLOR_BLUE      COLOR(  0,   0, 255)

#define COLOR_BROWN     COLOR(150,  75,   0)

static inline SDL_Color color_to_sdl_color(color_type color)
{
    SDL_Color c;
    c.r = RED(color);
    c.g = GREEN(color);
    c.b = BLUE(color);
    return c;
}

static inline Uint32 color_to_uint(color_type c)
{
    return SDL_MapRGB(SDL_GetVideoSurface()->format, RED(c), GREEN(c), BLUE(c));
}

static inline color_type uint_to_color(Uint32 u)
{
    Uint8 r, g, b;
    SDL_GetRGB(u, SDL_GetVideoSurface()->format, &r, &g, &b);
    return COLOR(r, g, b);
}

static inline void set_pixel(int x, int y, color_type color)
{
    SDL_Surface *surface = SDL_GetVideoSurface();
    Uint8 bpp = surface->format->BytesPerPixel;
    Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;
    Uint32 pixel = color_to_uint(color);

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

static inline void get_pixel(int x, int y, color_type *color)
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

    *color = uint_to_color(pixel);
}

static inline void draw_block(int x, int y, int w, int h, color_type color)
{
    SDL_Rect rect;
    rect.x = x;
    rect.y = y;
    rect.w = w;
    rect.h = h;
    SDL_FillRect(SDL_GetVideoSurface(), &rect, color_to_uint(color));
}

void init_sdl(unsigned w, unsigned h, const char *title);
void uninit_sdl(void);

void grab_events(void);
bool is_key_down(int c);
bool is_closed(void);
void close_window(void);

char *start_text_input(void);
char *start_text_cmd_input(void);
void stop_text_input(void);
bool is_text_input(void);

bool set_font(const char *ttf, int ptsize);
void draw_string(int x, int y, color_type color, const char *str);
void draw_string_centered(int x, int y, color_type color, const char *str);
void draw_string_right(int x, int y, color_type color, const char *str);
bool get_string_size(const char *str, int *w, int *h);

void draw_sprite(int x, int y, color_type color, const bool *sprite, int w, int h);
void draw_colored_sprite(int x, int y, const color_type *sprite, int w, int h);

void draw_line(int x1, int y1, int x2, int y2, color_type color);

#endif
