#include "moag_window.h"
#include <SDL/SDL.h>
#include <SDL/SDL_ttf.h>

namespace moag
{

const int BPP =     32;
const int FLAGS =   (SDL_SWSURFACE | SDL_DOUBLEBUF);

TTF_Font *_font = NULL;

int OpenWindow(int width, int height, const char *title) {
    if (SDL_Init(SDL_INIT_EVERYTHING) == -1 ||
        TTF_Init() == -1 ||
        !SDL_SetVideoMode(width, height, BPP, FLAGS))
        return -1;
    SDL_WM_SetCaption(title, NULL);
    // This is used for text input. Apparently, it slows down keypresses,
    // but enabling/disabling it during the game causes problems.
    SDL_EnableUNICODE(1);
    return 0;
}

void CloseWindow(void) {
    TTF_Quit();
    SDL_Quit();
}

int SetFont(const char *ttf, int ptsize) {
    if (_font)
        TTF_CloseFont(_font);
    if (!(_font = TTF_OpenFont(ttf, ptsize)))
        return -1;
    return 0;
}

void SetBlock(int x, int y, int w, int h, Uint8 r, Uint8 g, Uint8 b) {
    SDL_Rect rect;
    rect.x = x;
    rect.y = y;
    rect.w = w;
    rect.h = h;
    SDL_FillRect(SDL_GetVideoSurface(), &rect, SDL_MapRGB(SDL_GetVideoSurface()->format, r, g, b));
}

void SetPixel(int x, int y, Uint8 r, Uint8 g, Uint8 b) {
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

void GetPixel(int x, int y, Uint8 *r, Uint8 *g, Uint8 *b) {
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

void SetString(int x, int y, const char *str, Uint8 r, Uint8 g, Uint8 b) {
    if (!_font)
        return;
    SDL_Color color;
    color.r = r;
    color.g = g;
    color.b = b;
    SDL_Surface *text = TTF_RenderText_Solid(_font, str, color);
    if (!text)
        return;
    SDL_Rect pos;
    pos.x = x;
    pos.y = y;
    SDL_BlitSurface(text, NULL, SDL_GetVideoSurface(), &pos);
    SDL_FreeSurface(text);
}

void SetStringCentered(int x, int y, const char *str, Uint8 r, Uint8 g, Uint8 b) {
    if (!_font)
        return;
    SDL_Color color;
    color.r = r;
    color.g = g;
    color.b = b;
    SDL_Surface *text = TTF_RenderText_Solid(_font, str, color);
    if (!text)
        return;
    x -= text->w/2;
    SDL_Rect pos;
    pos.x = x;
    pos.y = y;
    SDL_BlitSurface(text, NULL, SDL_GetVideoSurface(), &pos);
    SDL_FreeSurface(text);
}

int GetStringSize(const char *str, int *w, int *h)
{
    if (!_font)
        return -1;
    return TTF_SizeText(_font, str, w, h);
}

void ClearWindow(Uint8 r, Uint8 g, Uint8 b) {
    SDL_FillRect(SDL_GetVideoSurface(), NULL, SDL_MapRGB(SDL_GetVideoSurface()->format, r, g, b));
}

void UpdateWindow(void) {
    SDL_Flip(SDL_GetVideoSurface());
}

}

