
#ifndef CLIENT_H
#define CLIENT_H

#include <SDL/SDL.h>

#include "common.h"

#define CONNECT_TIMEOUT 5000
#define BUFLEN          256
#define CHAT_LINES      7
#define CHAT_EXPIRETIME 18000

struct chatline {
    int expire;
    char *str;
};

extern ENetHost *g_client;
extern ENetPeer *g_peer;

void init_enet(const char *ip);
void uninit_enet(void);
void init_sdl(void);
void uninit_sdl(void);

#endif
