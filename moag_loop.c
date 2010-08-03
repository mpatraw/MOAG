
#include "moag_loop.h"
#include <SDL/SDL.h>

static MOAG_LoopCallback _drawFunc = NULL;
static MOAG_LoopCallback _updateFunc = NULL;

void MOAG_SetLoopCallback(MOAG_LoopCallback cb, int type)
{
    switch (type)
    {
    case MOAG_CB_DRAW:
        _drawFunc = cb;
        break;
    case MOAG_CB_UPDATE:
        _updateFunc = cb;
        break;
    default:
        break;
    }
}

static int _running = 1;
static int _fps = 1000;

void MOAG_SetFps(int fps)
{
    _fps = fps;
}

void MOAG_MainLoop(void)
{
    int skipTicks = 1000 / _fps;
    int maxFrameSkip = 10;
    int loops;
    Uint32 nextGameTick;
    
    nextGameTick = SDL_GetTicks();
    
    while (_running)
    {
        loops = 0;
        while (SDL_GetTicks() > nextGameTick && loops < maxFrameSkip)
        {
            SDL_PumpEvents();
            if (SDL_PeepEvents(NULL, ~((unsigned int)0), SDL_PEEKEVENT, SDL_QUITMASK) > 0)
                _running = 0;
        
            if (_updateFunc)
                _updateFunc();
            
            nextGameTick += skipTicks;
            loops += 1;
        }
        
        if (_drawFunc)
            _drawFunc();
    }
}

