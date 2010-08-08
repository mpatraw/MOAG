
#include "moag_loop.h"
#include "moag_window.h"
#include <SDL/SDL.h>

#define _MAX_LOOPSTATES 20

typedef struct _LoopState _LoopState;
struct _LoopState
{
    MOAG_LoopCallback drawFunc;
    MOAG_LoopCallback updateFunc;
    int type;
};

static _LoopState _loopStates[_MAX_LOOPSTATES] = {
    {NULL, NULL, MOAG_LS_BLOCKING}
};

static int _currentLoopState = 0;

int MOAG_SetLoopCallback(MOAG_LoopCallback cb, int type)
{
    if (_currentLoopState < 0 || _currentLoopState >= _MAX_LOOPSTATES)
        return -1;
        
    switch (type)
    {
    case MOAG_CB_DRAW:
        _loopStates[_currentLoopState].drawFunc = cb;
        break;
    case MOAG_CB_UPDATE:
        _loopStates[_currentLoopState].updateFunc = cb;
        break;
    default:
        break;
    }
    
    return 0;
}

int MOAG_SetLoopState(int type)
{
    if (_currentLoopState >= _MAX_LOOPSTATES)
        return -1;

    _loopStates[_currentLoopState].type = type;
    
    return 0;
}

int MOAG_PushLoopState(int type)
{
    _currentLoopState += 1;
    if (_currentLoopState >= _MAX_LOOPSTATES)
        return -1;
    
    _loopStates[_currentLoopState].drawFunc = NULL;
    _loopStates[_currentLoopState].updateFunc = NULL;
    _loopStates[_currentLoopState].type = type;
    
    return 0;
}

void MOAG_PopLoopState(void)
{
    _loopStates[_currentLoopState].drawFunc = NULL;
    _loopStates[_currentLoopState].updateFunc = NULL;
    _currentLoopState -= 1;
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
    int blockState;
    Uint32 nextGameTick;
    
    nextGameTick = SDL_GetTicks();
    
    while (_running && _currentLoopState > -1)
    {
        loops = 0;
        while (SDL_GetTicks() > nextGameTick && loops < maxFrameSkip)
        {
            if (_loopStates[_currentLoopState].updateFunc)
                _loopStates[_currentLoopState].updateFunc();
            
            nextGameTick += skipTicks;
            loops += 1;
        }
        
        /* Draw from the last BLOCKING state upward. */
        blockState = _currentLoopState;
        
        while (_loopStates[blockState].type != MOAG_LS_BLOCKING &&
               blockState >= 0)
            blockState--;
        
        while (blockState <= _currentLoopState)
        {
            _loopStates[blockState].drawFunc();
            blockState += 1;
        }
        
        MOAG_UpdateWindow();
    }
}

void MOAG_QuitMainLoop(void)
{
    _running = 0;
}

