
#ifndef EVENT_H
#define EVENT_H

#include <SDL/SDL_keysym.h>

namespace moag
{

const int BUTTON_LEFT = (1 << 0);
const int BUTTON_RIGHT = (1 << 0);

void GrabEvents(void);
int IsKeyDown(int key);
int IsKeyPressed(int key);
int IsKeyReleased(int key);
int IsButtonDown(int button);
void GetMousePosition(int *x, int *y);
char* StartTextInput();
char* StartTextCmdInput();
void StopTextInput();
int IsQuitting(void);

}

#endif

