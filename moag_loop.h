
#ifndef MOAG_LOOP_H
#define MOAG_LOOP_H

enum
{
    MOAG_CB_DRAW,
    MOAG_CB_UPDATE
};

typedef void (*MOAG_LoopCallback) (void);

void MOAG_SetFps(int fps);
void MOAG_SetLoopCallback(MOAG_LoopCallback cb, int type);
void MOAG_MainLoop(void);

#endif

