
#ifndef LOOP_H
#define LOOP_H

namespace moag
{

enum
{
    LS_BLOCKING = 0,
    LS_NONBLOCKING = 1
};

enum
{
    CB_DRAW,
    CB_UPDATE
};

typedef void (*LoopCallback) (void);

void SetFps(int fps);
int GetTicks();
int SetLoopCallback(LoopCallback cb, int type);
int SetLoopState(int type);
int PushLoopState(int type);
void PopLoopState();
void MainLoop();
void QuitMainLoop();

}

#endif

