
#ifndef MOAG_H
#define MOAG_H

#include "moag_chunk.h"
#include "moag_client.h"
#include "moag_event.h"
#include "moag_loop.h"
#include "moag_net.h"
#include "moag_server.h"
#include "moag_window.h"

struct Tank{
    int x,y,lastx,lasty;
    int angle,power;
    char active;
    char name[16];
    char bullet;
    char facingLeft;
    char kLeft,kRight,kUp,kDown,kFire;
};

struct Bullet{
    int x,y;
    float fx,fy,vx,vy;
    char active;
    char type;
};

struct Crate{
    int x,y;
    char type;
};

#endif

