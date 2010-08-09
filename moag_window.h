
#ifndef WINDOW_H
#define WINDOW_H

#include <SDL/SDL_types.h>

namespace moag
{

int OpenWindow(int width, int height, const char *title);
void CloseWindow();
int SetFont(const char *ttf, int ptsize);
void SetBlock(int x, int y, int w, int h, Uint8 r, Uint8 g, Uint8 b);
void SetPixel(int x, int y, Uint8 r, Uint8 g, Uint8 b);
void GetPixel(int x, int y, Uint8 *r, Uint8 *g, Uint8 *b);
void SetString(int x, int y, const char *str, Uint8 r, Uint8 g, Uint8 b);
void SetStringCentered(int x, int y, const char *str, Uint8 r, Uint8 g, Uint8 b);
int GetStringSize(const char *str, int *w, int *h);
void ClearWindow(Uint8 r, Uint8 g, Uint8 b);
void UpdateWindow();

}

#endif

