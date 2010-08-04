
#include "moag_event.h"
#include <SDL/SDL.h>

static int _keyTable[SDLK_LAST] = {0};
static int _keyTablePressed[SDLK_LAST] = {0};
static int _keyTableReleased[SDLK_LAST] = {0};

static int _x = 0;
static int _y = 0;

static int _quitting = 0;

void MOAG_GrabEvents(void)
{
    static SDL_Event ev;
    int i;
    
    _quitting = 0;
    
    for(i=0; i<SDLK_LAST; i++)
        _keyTablePressed[i]=0;
    for(i=0; i<SDLK_LAST; i++)
        _keyTableReleased[i]=0;

    while (SDL_PollEvent(&ev))
    {
        switch (ev.type)
        {
        case SDL_KEYDOWN:
            _keyTable[ev.key.keysym.sym] = 1;
            _keyTablePressed[ev.key.keysym.sym] = 1;
            break;
        case SDL_KEYUP:
            _keyTable[ev.key.keysym.sym] = 0;
            _keyTableReleased[ev.key.keysym.sym] = 1;
            break;
        case SDL_MOUSEMOTION:
            _x = ev.motion.x;
            _y = ev.motion.y;
            break;
        case SDL_QUIT:
            _quitting = 1;
            break;
        default:
            break;
        }
    }
}

int MOAG_IsKeyDown(int key) {
    return _keyTable[key];
}

int MOAG_IsKeyPressed(int key) {
    return _keyTablePressed[key];
}

int MOAG_IsKeyReleased(int key) {
    return _keyTableReleased[key];
}

int MOAG_IsButtonDown(int button)
{
    Uint8 state;
    Uint8 newbutton = 0;
    
    if (button & MOAG_BUTTON_LEFT)
        newbutton |= SDL_BUTTON(1);
    if (button & MOAG_BUTTON_RIGHT)
        newbutton |= SDL_BUTTON(2);

    state = SDL_GetMouseState(NULL, NULL);
    
    return state & newbutton;
}

void MOAG_GetMousePosition(int *x, int *y)
{
    *x = _x;
    *y = _y;
}

int MOAG_IsQuitting(void)
{
    return _quitting;
}

