#include "moag_loop.h"
#include "moag_window.h"
#include <SDL/SDL.h>

namespace moag
{

const int _MAX_LOOPSTATES = 20;

struct _LoopState {
    LoopCallback drawFunc;
    LoopCallback updateFunc;
    int type;
};

_LoopState _loopStates[_MAX_LOOPSTATES] = {
    {NULL, NULL, LS_BLOCKING}
};

int _currentLoopState = 0;

int SetLoopCallback(LoopCallback cb, int type) {
    if (_currentLoopState < 0 || _currentLoopState >= _MAX_LOOPSTATES)
        return -1;
        
    switch (type) {
    case CB_DRAW:
        _loopStates[_currentLoopState].drawFunc = cb;
        break;
    case CB_UPDATE:
        _loopStates[_currentLoopState].updateFunc = cb;
        break;
    default:
        break;
    }
    
    return 0;
}

int SetLoopState(int type) {
    if (_currentLoopState >= _MAX_LOOPSTATES)
        return -1;

    _loopStates[_currentLoopState].type = type;
    
    return 0;
}

int PushLoopState(int type) {
    if (_currentLoopState + 1 >= _MAX_LOOPSTATES)
        return -1;
    _currentLoopState++;
    _loopStates[_currentLoopState].drawFunc = NULL;
    _loopStates[_currentLoopState].updateFunc = NULL;
    _loopStates[_currentLoopState].type = type;
    
    return 0;
}

void PopLoopState() {
    if (_currentLoopState <= 0)
        return;
    _loopStates[_currentLoopState].drawFunc = NULL;
    _loopStates[_currentLoopState].updateFunc = NULL;
    _currentLoopState--;
}

int _running = 1;
int _fps = 1000;

void SetFps(int fps) {
    _fps = fps;
}

int GetTicks() {
    return SDL_GetTicks();
}

void MainLoop() {
    const int skipTicks = 1000 / _fps;
    const int maxFrameSkip = 10;

    Uint32 nextGameTick = SDL_GetTicks();
    
    while (_running && _currentLoopState > -1) {
        int loops = 0;
        while (SDL_GetTicks() > nextGameTick && loops < maxFrameSkip) {
            if (_loopStates[_currentLoopState].updateFunc)
                _loopStates[_currentLoopState].updateFunc();
            nextGameTick += skipTicks;
            loops++;
        }
        
        // Draw from the last BLOCKING state upward.
        int blockState = _currentLoopState;
        while (_loopStates[blockState].type != LS_BLOCKING && blockState >= 0)
            blockState--;
        while (blockState <= _currentLoopState) {
            _loopStates[blockState].drawFunc();
            blockState++;
        }
        
        UpdateWindow();
    }
}

void QuitMainLoop(void) {
    _running = 0;
}

}

