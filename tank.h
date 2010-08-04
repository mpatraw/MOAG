#ifndef H_TANK
#define H_TANK

struct Tank{
    int x,y,lastx,lasty;
    int angle,power;
    char active;
    char facingLeft;
    char kLeft,kRight,kUp,kDown,kFire;
};

#endif

