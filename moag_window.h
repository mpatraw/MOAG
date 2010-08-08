
#ifndef MOAG_WINDOW_H
#define MOAG_WINDOW_H

#include <SDL/SDL_types.h>

int MOAG_OpenWindow(int width, int height, const char *title);
void MOAG_CloseWindow();
int MOAG_SetFont(const char *ttf, int ptsize);
void MOAG_SetBlock(int x, int y, int w, int h, Uint8 r, Uint8 g, Uint8 b);
void MOAG_SetPixel(int x, int y, Uint8 r, Uint8 g, Uint8 b);
void MOAG_GetPixel(int x, int y, Uint8 *r, Uint8 *g, Uint8 *b);
void MOAG_SetString(int x, int y, const char *str, Uint8 r, Uint8 g, Uint8 b);
void MOAG_SetStringCentered(int x, int y, const char *str, Uint8 r, Uint8 g, Uint8 b);
void MOAG_ClearWindow(Uint8 r, Uint8 g, Uint8 b);
void MOAG_UpdateWindow();

#endif

