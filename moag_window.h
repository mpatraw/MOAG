
#ifndef MOAG_WINDOW_H
#define MOAG_WINDOW_H

#include <SDL/SDL_types.h>

int MOAG_OpenWindow(int width, int height, const char *title);
void MOAG_CloseWindow(void);
void MOAG_SetBlock(int x, int y, int w, int h, Uint8 r, Uint8 g, Uint8 b);
void MOAG_SetPixel(int x, int y, Uint8 r, Uint8 g, Uint8 b);
void MOAG_GetPixel(int x, int y, Uint8 *r, Uint8 *g, Uint8 *b);
void MOAG_ClearWindow(Uint8 r, Uint8 g, Uint8 b);
void MOAG_UpdateWindow(void);

#endif

