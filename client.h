
#ifndef CLIENT_H
#define CLIENT_H

#include "common.h"
#include "sdl_aux.h"

#define BUFLEN          256
#define CHAT_LINES      7
#define CHAT_EXPIRETIME 18000

struct chatline
{
    int expire;
    char *str;
};

static inline void send_input_chunk(int key, uint16_t t)
{
    struct input_chunk chunk;
    chunk._.type = INPUT_CHUNK;
    chunk.key = key;
    chunk.ms = t;

    send_chunk((void *)&chunk, sizeof chunk, false, true);

    LOG("%u: %s: %zu\n", (unsigned)time(NULL), __PRETTY_FUNCTION__, sizeof chunk);
}

#endif
