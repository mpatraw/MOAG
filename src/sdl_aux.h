
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

#define COLOR_ALICE_BLUE COLOR(0xf0, 0xf8, 0xff)
#define COLOR_ANTIQUE_WHITE COLOR(0xfa, 0xeb, 0xd7)
#define COLOR_AQUA COLOR(0x00, 0xff, 0xff)
#define COLOR_AQUAMARINE COLOR(0x7f, 0xff, 0xd4)
#define COLOR_AZURE COLOR(0xf0, 0xff, 0xff)
#define COLOR_BEIGE COLOR(0xf5, 0xf5, 0xdc)
#define COLOR_BISQUE COLOR(0xff, 0xe4, 0xc4)
#define COLOR_BLACK COLOR(0x00, 0x00, 0x00)
#define COLOR_BLANCHED_ALMOND COLOR(0xff, 0xeb, 0xcd)
#define COLOR_BLUE COLOR(0x00, 0x00, 0xff)
#define COLOR_BLUE_VIOLET COLOR(0x8a, 0x2b, 0xe2)
#define COLOR_BROWN COLOR(0xa5, 0x2a, 0x2a)
#define COLOR_BURLY_WOOD COLOR(0xde, 0xb8, 0x87)
#define COLOR_CADET_BLUE COLOR(0x5f, 0x9e, 0xa0)
#define COLOR_CHARTREUSE COLOR(0x7f, 0xff, 0x00)
#define COLOR_CHOCOLATE COLOR(0xd2, 0x69, 0x1e)
#define COLOR_CORAL COLOR(0xff, 0x7f, 0x50)
#define COLOR_CORNFLOWER_BLUE COLOR(0x64, 0x95, 0xed)
#define COLOR_CORNSILK COLOR(0xff, 0xf8, 0xdc)
#define COLOR_CRIMSON COLOR(0xdc, 0x14, 0x3c)
#define COLOR_CYAN COLOR(0x00, 0xff, 0xff)
#define COLOR_DARK_BLUE COLOR(0x00, 0x00, 0x8b)
#define COLOR_DARK_CYAN COLOR(0x00, 0x8b, 0x8b)
#define COLOR_DARK_GOLDENROD COLOR(0xb8, 0x86, 0x0b)
#define COLOR_DARK_GRAY COLOR(0xa9, 0xa9, 0xa9)
#define COLOR_DARK_GREY COLOR(0xa9, 0xa9, 0xa9)
#define COLOR_DARK_GREEN COLOR(0x00, 0x64, 0x00)
#define COLOR_DARK_KHAKI COLOR(0xbd, 0xb7, 0x6b)
#define COLOR_DARK_MAGENTA COLOR(0x8b, 0x00, 0x8b)
#define COLOR_DARK_OLIVE_GREEN COLOR(0x55, 0x6b, 0x2f)
#define COLOR_DARK_ORANGE COLOR(0xff, 0x8c, 0x00)
#define COLOR_DARK_ORCHID COLOR(0x99, 0x32, 0xcc)
#define COLOR_DARK_RED COLOR(0x8b, 0x00, 0x00)
#define COLOR_DARK_SALMON COLOR(0xe9, 0x96, 0x7a)
#define COLOR_DARK_SEA_GREEN COLOR(0x8f, 0xbc, 0x8f)
#define COLOR_DARK_SLATE_BLUE COLOR(0x48, 0x3d, 0x8b)
#define COLOR_DARK_SLATE_GRAY COLOR(0x2f, 0x4f, 0x4f)
#define COLOR_DARK_SLATE_GREY COLOR(0x2f, 0x4f, 0x4f)
#define COLOR_DARK_TURQUOISE COLOR(0x00, 0xce, 0xd1)
#define COLOR_DARK_VIOLET COLOR(0x94, 0x00, 0xd3)
#define COLOR_DEEP_PINK COLOR(0xff, 0x14, 0x93)
#define COLOR_DEEP_SKY_BLUE COLOR(0x00, 0xbf, 0xff)
#define COLOR_DIM_GRAY COLOR(0x69, 0x69, 0x69)
#define COLOR_DIM_GREY COLOR(0x69, 0x69, 0x69)
#define COLOR_DODGER_BLUE COLOR(0x1e, 0x90, 0xff)
#define COLOR_FIRE_BRICK COLOR(0xb2, 0x22, 0x22)
#define COLOR_FLORAL_WHITE COLOR(0xff, 0xfa, 0xf0)
#define COLOR_FOREST_GREEN COLOR(0x22, 0x8b, 0x22)
#define COLOR_FUCHSIA COLOR(0xff, 0x00, 0xff)
#define COLOR_GAINSBORO COLOR(0xdc, 0xdc, 0xdc)
#define COLOR_GHOST_WHITE COLOR(0xf8, 0xf8, 0xff)
#define COLOR_GOLD COLOR(0xff, 0xd7, 0x00)
#define COLOR_GOLDENROD COLOR(0xda, 0xa5, 0x20)
#define COLOR_GRAY COLOR(0x80, 0x80, 0x80)
#define COLOR_GREY COLOR(0x80, 0x80, 0x80)
#define COLOR_GREEN COLOR(0x00, 0x80, 0x00)
#define COLOR_GREEN_YELLOW COLOR(0xad, 0xff, 0x2f)
#define COLOR_HONEYDEW COLOR(0xf0, 0xff, 0xf0)
#define COLOR_HOT_PINK COLOR(0xff, 0x69, 0xb4)
#define COLOR_INDIAN_RED COLOR(0xcd, 0x5c, 0x5c)
#define COLOR_INDIGO COLOR(0x4b, 0x00, 0x82)
#define COLOR_IVORY COLOR(0xff, 0xff, 0xf0)
#define COLOR_KHAKI COLOR(0xf0, 0xe6, 0x8c)
#define COLOR_LAVENDER COLOR(0xe6, 0xe6, 0xfa)
#define COLOR_LAVENDER_BLUSH COLOR(0xff, 0xf0, 0xf5)
#define COLOR_LAWN_GREEN COLOR(0x7c, 0xfc, 0x00)
#define COLOR_LEMON_CHIFFON COLOR(0xff, 0xfa, 0xcd)
#define COLOR_LIGHT_BLUE COLOR(0xad, 0xd8, 0xe6)
#define COLOR_LIGHT_CORAL COLOR(0xf0, 0x80, 0x80)
#define COLOR_LIGHT_CYAN COLOR(0xe0, 0xff, 0xff)
#define COLOR_LIGHT_GOLDENROD_YELLOW COLOR(0xfa, 0xfa, 0xd2)
#define COLOR_LIGHT_GREEN COLOR(0x90, 0xee, 0x90)
#define COLOR_LIGHT_GRAY COLOR(0xd3, 0xd3, 0xd3)
#define COLOR_LIGHT_GREY COLOR(0xd3, 0xd3, 0xd3)
#define COLOR_LIGHT_PINK COLOR(0xff, 0xb6, 0xc1)
#define COLOR_LIGHT_SALMON COLOR(0xff, 0xa0, 0x7a)
#define COLOR_LIGHT_SEA_GREEN COLOR(0x20, 0xb2, 0xaa)
#define COLOR_LIGHT_SKY_BLUE COLOR(0x87, 0xce, 0xfa)
#define COLOR_LIGHT_SLATE_GRAY COLOR(0x77, 0x88, 0x99)
#define COLOR_LIGHT_SLATE_GREY COLOR(0x77, 0x88, 0x99)
#define COLOR_LIGHT_STEEL_BLUE COLOR(0xb0, 0xc4, 0xde)
#define COLOR_LIGHT_YELLOW COLOR(0xff, 0xff, 0xe0)
#define COLOR_LIME COLOR(0x00, 0xff, 0x00)
#define COLOR_LIME_GREEN COLOR(0x32, 0xcd, 0x32)
#define COLOR_LINEN COLOR(0xfa, 0xf0, 0xe6)
#define COLOR_MAGENTA COLOR(0xff, 0x00, 0xff)
#define COLOR_MAROON COLOR(0x80, 0x00, 0x00)
#define COLOR_MEDIUM_AQUAMARINE COLOR(0x66, 0xcd, 0xaa)
#define COLOR_MEDIUM_BLUE COLOR(0x00, 0x00, 0xcd)
#define COLOR_MEDIUM_ORCHID COLOR(0xba, 0x55, 0xd3)
#define COLOR_MEDIUM_PURPLE COLOR(0x93, 0x70, 0xdb)
#define COLOR_MEDIUM_SEA_GREEN COLOR(0x3c, 0xb3, 0x71)
#define COLOR_MEDIUM_SLATE_BLUE COLOR(0x7b, 0x68, 0xee)
#define COLOR_MEDIUM_SPRING_GREEN COLOR(0x00, 0xfa, 0x9a)
#define COLOR_MEDIUM_TURQUOISE COLOR(0x48, 0xd1, 0xcc)
#define COLOR_MEDIUM_VIOLET_RED COLOR(0xc7, 0x15, 0x85)
#define COLOR_MIDNIGHT_BLUE COLOR(0x19, 0x19, 0x70)
#define COLOR_MINT_CREAM COLOR(0xf5, 0xff, 0xfa)
#define COLOR_MISTY_ROSE COLOR(0xff, 0xe4, 0xe1)
#define COLOR_MOCCASIN COLOR(0xff, 0xe4, 0xb5)
#define COLOR_NAVAJO_WHITE COLOR(0xff, 0xde, 0xad)
#define COLOR_NAVY COLOR(0x00, 0x00, 0x80)
#define COLOR_OLD_LACE COLOR(0xfd, 0xf5, 0xe6)
#define COLOR_OLIVE COLOR(0x80, 0x80, 0x00)
#define COLOR_OLIVE_DRAB COLOR(0x6b, 0x8e, 0x23)
#define COLOR_ORANGE COLOR(0xff, 0xa5, 0x00)
#define COLOR_ORANGE_RED COLOR(0xff, 0x45, 0x00)
#define COLOR_ORCHID COLOR(0xda, 0x70, 0xd6)
#define COLOR_PALE_GOLDENROD COLOR(0xee, 0xe8, 0xaa)
#define COLOR_PALE_GREEN COLOR(0x98, 0xfb, 0x98)
#define COLOR_PALE_TURQUOISE COLOR(0xaf, 0xee, 0xee)
#define COLOR_PALE_VIOLET_RED COLOR(0xdb, 0x70, 0x93)
#define COLOR_PAPAYA_WHIP COLOR(0xff, 0xef, 0xd5)
#define COLOR_PEACH_PUFF COLOR(0xff, 0xda, 0xb9)
#define COLOR_PERU COLOR(0xcd, 0x85, 0x3f)
#define COLOR_PINK COLOR(0xff, 0xc0, 0xcb)
#define COLOR_PLUM COLOR(0xdd, 0xa0, 0xdd)
#define COLOR_POWDER_BLUE COLOR(0xb0, 0xe0, 0xe6)
#define COLOR_PURPLE COLOR(0x80, 0x00, 0x80)
#define COLOR_RED COLOR(0xff, 0x00, 0x00)
#define COLOR_ROSY_BROWN COLOR(0xbc, 0x8f, 0x8f)
#define COLOR_ROYAL_BLUE COLOR(0x41, 0x69, 0xe1)
#define COLOR_SADDLE_BROWN COLOR(0x8b, 0x45, 0x13)
#define COLOR_SALMON COLOR(0xfa, 0x80, 0x72)
#define COLOR_SANDY_BROWN COLOR(0xf4, 0xa4, 0x60)
#define COLOR_SEA_GREEN COLOR(0x2e, 0x8b, 0x57)
#define COLOR_SEASHELL COLOR(0xff, 0xf5, 0xee)
#define COLOR_SIENNA COLOR(0xa0, 0x52, 0x2d)
#define COLOR_SILVER COLOR(0xc0, 0xc0, 0xc0)
#define COLOR_SKY_BLUE COLOR(0x87, 0xce, 0xeb)
#define COLOR_SLATE_BLUE COLOR(0x6a, 0x5a, 0xcd)
#define SlateGray COLOR(0x70, 0x80, 0x90)
#define COLOR_SLATE_GREY COLOR(0x70, 0x80, 0x90)
#define COLOR_SNOW COLOR(0xff, 0xfa, 0xfa)
#define COLOR_SPRING_GREEN COLOR(0x00, 0xff, 0x7f)
#define COLOR_STEEL_BLUE COLOR(0x46, 0x82, 0xb4)
#define COLOR_TAN COLOR(0xd2, 0xb4, 0x8c)
#define COLOR_TEAL COLOR(0x00, 0x80, 0x80)
#define COLOR_THISTLE COLOR(0xd8, 0xbf, 0xd8)
#define COLOR_TOMATO COLOR(0xff, 0x63, 0x47)
#define COLOR_TURQUOISE COLOR(0x40, 0xe0, 0xd0)
#define COLOR_VIOLET COLOR(0xee, 0x82, 0xee)
#define COLOR_WHEAT COLOR(0xf5, 0xde, 0xb3)
#define COLOR_WHITE COLOR(0xff, 0xff, 0xff)
#define COLOR_WHITE_SMOKE COLOR(0xf5, 0xf5, 0xf5)
#define COLOR_YELLOW COLOR(0xff, 0xff, 0x00)
#define COLOR_YELLOW_GREEN COLOR(0x9a, 0xcd, 0x32)

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
