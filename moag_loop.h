
#ifndef MOAG_LOOP_H
#define MOAG_LOOP_H

#define MOAG_LS_BLOCKING 0
#define MOAG_LS_NONBLOCKING 1

enum
{
    MOAG_CB_DRAW,
    MOAG_CB_UPDATE
};

typedef void (*MOAG_LoopCallback) (void);

void MOAG_SetFps(int fps);
int MOAG_GetTicks();
int MOAG_SetLoopCallback(MOAG_LoopCallback cb, int type);
int MOAG_SetLoopState(int type);
int MOAG_PushLoopState(int type);
void MOAG_PopLoopState();
void MOAG_MainLoop();
void MOAG_QuitMainLoop();

#endif

