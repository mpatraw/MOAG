
#include "moag_window.h"
#include <SDL/SDL.h>
#include <SDL/SDL_ttf.h>

#define BPP    32
#define FLAGS  SDL_SWSURFACE | SDL_DOUBLEBUF

static TTF_Font *_font = NULL;

int MOAG_OpenWindow(int width, int height, const char *title)
{
    if (SDL_Init(SDL_INIT_EVERYTHING) == -1)
        return -1;
    
    if (TTF_Init() == -1)
        return -1;
    
    if (!SDL_SetVideoMode(width, height, BPP, FLAGS))
        return -1;
    
    SDL_WM_SetCaption(title, NULL);
    
    return 0;
}

void MOAG_CloseWindow(void)
{
    TTF_Quit();
    SDL_Quit();
}

int MOAG_SetFont(const char *ttf, int ptsize)
{
    if (_font)
        TTF_CloseFont(_font);

    _font = TTF_OpenFont(ttf, ptsize);
    
    if (!_font)
        return -1;
        
    
    return 0;
}

void MOAG_SetBlock(int x, int y, int w, int h, Uint8 r, Uint8 g, Uint8 b)
{
    Uint32 color;
    SDL_Rect rect;

    color = SDL_MapRGB(SDL_GetVideoSurface()->format, r, g, b);
    rect.x = x;
    rect.y = y;
    rect.w = w;
    rect.h = h;
    
    SDL_FillRect(SDL_GetVideoSurface(), &rect, color);
}

void MOAG_SetPixel(int x, int y, Uint8 r, Uint8 g, Uint8 b)
{
    SDL_Surface *surface;
    Uint32 pixel;
    Uint8 bpp;
    Uint8 *p;
    
    surface = SDL_GetVideoSurface();
    pixel = SDL_MapRGB(surface->format, r, g, b);
    bpp = surface->format->BytesPerPixel;

    p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;

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

void MOAG_GetPixel(int x, int y, Uint8 *r, Uint8 *g, Uint8 *b)
{
    SDL_Surface *surface;
    Uint8 bpp;
    Uint8 *p;
    Uint32 pixel;
    
    surface = SDL_GetVideoSurface();
    bpp = surface->format->BytesPerPixel;

    p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;

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

void MOAG_SetString(int x, int y, const char *str, Uint8 r, Uint8 g, Uint8 b)
{
    SDL_Surface *text;
    SDL_Color color;
    SDL_Rect pos;
    
    if (!_font)
        return;
    
    color.r = r;
    color.g = g;
    color.b = b;
    
    text = TTF_RenderText_Solid(_font, str, color);
    
    if (!text)
        return;
    
    pos.x = x;
    pos.y = y;
    
    SDL_BlitSurface(text, NULL, SDL_GetVideoSurface(), &pos);
    SDL_FreeSurface(text);
}

void MOAG_ClearWindow(Uint8 r, Uint8 g, Uint8 b)
{
    Uint32 color;

    color = SDL_MapRGB(SDL_GetVideoSurface()->format, r, g, b);
    
    SDL_FillRect(SDL_GetVideoSurface(), NULL, color);
}

void MOAG_UpdateWindow(void)
{
    SDL_Flip(SDL_GetVideoSurface());
}


