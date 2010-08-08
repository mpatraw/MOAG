
#include "moag_event.h"
#include <SDL/SDL.h>

bool _keyTable[SDLK_LAST] = {};
bool _keyTablePressed[SDLK_LAST] = {};
bool _keyTableReleased[SDLK_LAST] = {};

int _x = 0;
int _y = 0;

bool _quitting = false;

void MOAG_GrabEvents(void) {
    _quitting = 0;
    
    for(int i=0; i<SDLK_LAST; i++)
        _keyTablePressed[i]=false;
    for(int i=0; i<SDLK_LAST; i++)
        _keyTableReleased[i]=false;

    SDL_Event ev;
    while (SDL_PollEvent(&ev)) {
        switch (ev.type) {
        case SDL_KEYDOWN:
            _keyTable[ev.key.keysym.sym] = true;
            _keyTablePressed[ev.key.keysym.sym] = true;
            break;
        case SDL_KEYUP:
            _keyTable[ev.key.keysym.sym] = false;
            _keyTableReleased[ev.key.keysym.sym] = true;
            break;
        case SDL_MOUSEMOTION:
            _x = ev.motion.x;
            _y = ev.motion.y;
            break;
        case SDL_QUIT:
            _quitting = true;
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

int MOAG_IsButtonDown(int button) {
    Uint8 newbutton = 0;
    if (button & MOAG_BUTTON_LEFT)
        newbutton |= SDL_BUTTON(1);
    if (button & MOAG_BUTTON_RIGHT)
        newbutton |= SDL_BUTTON(2);
    return newbutton & SDL_GetMouseState(NULL, NULL);
}

void MOAG_GetMousePosition(int *x, int *y) {
    *x = _x;
    *y = _y;
}

int MOAG_IsQuitting(void) {
    return _quitting;
}

