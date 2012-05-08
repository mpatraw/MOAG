
#include "sdl_aux.h"

void init_sdl(const char *title)
{
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
        die("%s\n", SDL_GetError());

    SDL_Surface *s = SDL_SetVideoMode(LAND_WIDTH, LAND_HEIGHT, 32, SDL_DOUBLEBUF);
    if (!s)
        die("%s\n", SDL_GetError());

    if (TTF_Init() != 0)
        die("%s\n", TTF_GetError());

    SDL_EnableUNICODE(true);
    SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);

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

    while (SDL_PollEvent(&ev)) {
        switch (ev.type) {
        case SDL_QUIT:
            _is_closed = true;
            break;

        case SDL_KEYDOWN:
            _key_table[ev.key.keysym.sym] = true;
            if (_inputmode) {
                if (ev.key.keysym.sym == SDLK_BACKSPACE) {
                    if (_inputlen > 0)
                        _input[--_inputlen] = '\0';
                }
                else {
                    if(_inputlen < 255){
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

void draw_string(int x, int y, Uint8 r, Uint8 g, Uint8 b, const char *str)
{
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

void draw_string_centered(int x, int y, Uint8 r, Uint8 g, Uint8 b, const char *str)
{
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

bool get_string_size(const char *str, int *w, int *h)
{
    if (!_font)
        return false;
    return TTF_SizeText(_font, str, w, h) == 0 ? true : false;
}
