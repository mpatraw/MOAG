
#ifndef CLIENT_H
#define CLIENT_H

#include "common.h"
#include "enet_aux.h"
#include "sdl_aux.h"

#define BUFLEN          256
#define CHAT_LINES      7
#define CHAT_EXPIRETIME 18000

struct chatline {
    int expire;
    char *str;
};

static inline void send_chunk(unsigned char *buf, size_t len, bool reliable)
{
    ENetPacket *packet = enet_packet_create(NULL, len, reliable);
    memcpy(packet->data, buf, len);
    enet_peer_send(get_peer(), 1, packet);
}

static inline void send_byte(unsigned char c)
{
    send_chunk(&c, 1, true);
}

#endif
