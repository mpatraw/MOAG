
#include "moag_event.h"
#include <SDL/SDL.h>

bool _keyTable[SDLK_LAST] = {};
bool _keyTablePressed[SDLK_LAST] = {};
bool _keyTableReleased[SDLK_LAST] = {};

char _input[256];
int _inputlen=0;
bool _inputmodep=false;

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
            if(_inputmodep){
                switch (ev.key.keysym.sym) {
                case SDLK_BACKSPACE:
                    if(_inputlen>0)
                        _input[--_inputlen]='\0';
                    break;
                default:
                    int ch=ev.key.keysym.sym;
                    if(ch>=32 && ch<=122){
                        if(ev.key.keysym.mod&KMOD_SHIFT){
                            switch(ch){
                            case '`': ch='~'; break;
                            case '1': ch='!'; break;
                            case '2': ch='@'; break;
                            case '3': ch='#'; break;
                            case '4': ch='$'; break;
                            case '5': ch='%'; break;
                            case '6': ch='^'; break;
                            case '7': ch='&'; break;
                            case '8': ch='*'; break;
                            case '9': ch='('; break;
                            case '0': ch=')'; break;
                            case '-': ch='_'; break;
                            case '=': ch='+'; break;
                            case '[': ch='{'; break;
                            case ']': ch='}'; break;
                            case ';': ch=':'; break;
                            case '\'':ch='"'; break;
                            case ',': ch='<'; break;
                            case '.': ch='>'; break;
                            case '/': ch='?'; break;
                            case '\\':ch='|'; break;
                            default: if(ch>='a' && ch<='z') ch-=32; break;
                            }
                        }
                        _input[_inputlen++]=(char)ch;
                        _input[_inputlen]='\0';
                    }
                    /*if(_inputlen<255){
                        char ch=ev.key.keysym.unicode&0x7f;
                        if((ev.key.keysym.unicode&0xff80)!=0)
                            ch='?';
                        if(ch>=32 && ch<=126)
                            _input[_inputlen++]=ch;
                            _input[_inputlen]='\0';
                    }
                    break;*/
                }
            }
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

char* MOAG_StartTextInput(){
    //SDL_EnableUNICODE(1);
    SDL_EnableKeyRepeat(200,50);
    _inputmodep=true;
    _inputlen=0;
    _input[0]='\0';
    return _input;
}

char* MOAG_StartTextCmdInput(){
    MOAG_StartTextInput();
    _inputlen=1;
    _input[0]='/';
    _input[1]='\0';
    return _input;
}

void MOAG_StopTextInput(){
    //SDL_EnableUNICODE(0);
    SDL_EnableKeyRepeat(0,0);
    _inputmodep=false;
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

