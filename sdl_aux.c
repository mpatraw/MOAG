
#include <stdarg.h>
#include <stdio.h>

#include "sdl_aux.h"

static void die(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
    exit(EXIT_FAILURE);
}

void init_sdl(unsigned w, unsigned h, const char *title)
{
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
        die("%s\n", SDL_GetError());

    SDL_Surface *s = SDL_SetVideoMode(w, h, 32, SDL_DOUBLEBUF);
    if (!s)
        die("%s\n", SDL_GetError());

    if (TTF_Init() != 0)
        die("%s\n", TTF_GetError());

    SDL_EnableUNICODE(true);

    SDL_WM_SetCaption(title, NULL);
}

void uninit_sdl(void)
{
    TTF_Quit();
    SDL_Quit();
}

static bool _key_table[SDLK_LAST] = {false};
static bool _is_closed = false;

char _input[256];
size_t _inputlen = 0;
bool _inputmode = false;

void grab_events(void)
{
    static SDL_Event ev;

    while (SDL_PollEvent(&ev))
    {
        switch (ev.type)
        {
        case SDL_QUIT:
            _is_closed = true;
            break;

        case SDL_KEYDOWN:
            _key_table[ev.key.keysym.sym] = true;
            if (_inputmode)
            {
                if (ev.key.keysym.sym == SDLK_BACKSPACE)
                {
                    if (_inputlen > 0)
                        _input[--_inputlen] = '\0';
                }
                else
                {
                    if(_inputlen < 255)
                    {
                        char ch = ev.key.keysym.unicode;
                        if (ch >= 32 && ch <= 126)
                            _input[_inputlen++] = ch;
                        _input[_inputlen] = '\0';
                    }
                }
            }
            break;

        case SDL_KEYUP:
            _key_table[ev.key.keysym.sym] = false;
            break;

        default:
            break;
        }
    }
}

bool is_key_down(int c)
{
    return _key_table[c];
}

bool is_closed(void)
{
    return _is_closed;
}

void close_window(void)
{
    _is_closed = true;
}

char *start_text_input(void)
{
    SDL_EnableKeyRepeat(200, 50);
    _inputmode = true;
    _inputlen = 0;
    _input[0] = '\0';
    return _input;
}

char *start_text_cmd_input(void)
{
    start_text_input();
    _inputlen = 1;
    _input[0] = '/';
    _input[1] = '\0';
    return _input;
}

void stop_text_input(void)
{
    SDL_EnableKeyRepeat(0, 0);
    _inputmode = false;
}

bool is_text_input(void)
{
    return _inputmode;
}

static TTF_Font *_font = NULL;

bool set_font(const char *ttf, int ptsize)
{
    if (_font)
        TTF_CloseFont(_font);
    if (!(_font = TTF_OpenFont(ttf, ptsize)))
        return false;
    return true;
}

void draw_string(int x, int y, color_type color, const char *str)
{
    if (!_font)
        return;
    SDL_Surface *text = TTF_RenderText_Solid(_font, str,
                                             color_to_sdl_color(color));
    if (!text)
        return;
    SDL_Rect pos;
    pos.x = x;
    pos.y = y;
    SDL_BlitSurface(text, NULL, SDL_GetVideoSurface(), &pos);
    SDL_FreeSurface(text);
}

void draw_string_centered(int x, int y, color_type color, const char *str)
{
    if (!_font)
        return;
    SDL_Surface *text = TTF_RenderText_Solid(_font, str,
                                             color_to_sdl_color(color));
    if (!text)
        return;
    x -= text->w / 2;
    SDL_Rect pos;
    pos.x = x;
    pos.y = y;
    SDL_BlitSurface(text, NULL, SDL_GetVideoSurface(), &pos);
    SDL_FreeSurface(text);
}

void draw_string_right(int x, int y, color_type color, const char *str)
{
    if (!_font)
        return;
    SDL_Surface *text = TTF_RenderText_Solid(_font, str,
                                             color_to_sdl_color(color));
    if (!text)
        return;
    x -= text->w;
    SDL_Rect pos;
    pos.x = x;
    pos.y = y;
    SDL_BlitSurface(text, NULL, SDL_GetVideoSurface(), &pos);
    SDL_FreeSurface(text);
}

bool get_string_size(const char *str, int *w, int *h)
{
    if (!_font)
        return false;
    return TTF_SizeText(_font, str, w, h) == 0 ? true : false;
}

void draw_sprite(int x, int y, color_type color, const bool *sprite, int w, int h)
{
    SDL_Surface *s = SDL_GetVideoSurface();
    for (int ix = 0; ix < w; ++ix)
        for (int iy = 0; iy < h; ++iy)
            if (ix + x >= 0 && ix + x < s->w && iy + y >= 0 && iy + y < s->h)
                if (sprite[iy * w + ix])
                    set_pixel(ix + x, iy + y, color);
}

void draw_colored_sprite(int x, int y, const color_type *sprite, int w, int h)
{
    SDL_Surface *s = SDL_GetVideoSurface();
    for (int ix = 0; ix < w; ++ix)
        for (int iy = 0; iy < h; ++iy)
            if (ix + x >= 0 && ix + x < s->w && iy + y >= 0 && iy + y < s->h)
                set_pixel(ix + x, iy + y, sprite[iy * w + ix]);
}

void draw_line(int x1, int y1, int x2, int y2, color_type color)
{
    SDL_Surface *s = SDL_GetVideoSurface();
    int dx = abs(x2 - x1), sx = x1 < x2 ? 1 : -1;
    int dy = abs(y2 - y1), sy = y1 < y2 ? 1 : -1;
    int err = (dx > dy ? dx : -dy) / 2, e2;

    for (;;)
    {
        if (x1 >= 0 && x1 < s->w && y1 >= 0 && y1 < s->h)
            set_pixel(x1, y1, color);

        if (x1 == x2 && y1 == y2) break;
        e2 = err;
        if (e2 > -dx) { err -= dy; x1 += sx; }
        if (e2 <  dy) { err += dx; y1 += sy; }
    }
}
