
#ifndef MOAG_EVENT_H
#define MOAG_EVENT_H

#include <SDL/SDL_keysym.h>

#define MOAG_BUTTON_LEFT (1 << 0)
#define MOAG_BUTTON_RIGHT (1 << 0)

void MOAG_GrabEvents(void);
int MOAG_IsKeyDown(int key);
int MOAG_IsKeyPressed(int key);
int MOAG_IsKeyReleased(int key);
int MOAG_IsButtonDown(int button);
void MOAG_GetMousePosition(int *x, int *y);
char* MOAG_StartTextInput();
char* MOAG_StartTextCmdInput();
void MOAG_StopTextInput();
int MOAG_IsQuitting(void);

#endif

