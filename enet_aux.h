
#ifndef ENET_AUX_H
#define ENET_AUX_H

#include <string.h>

#include <enet/enet.h>
#include <netinet/in.h>

#define CONNECT_TIMEOUT 5000
#define MAX_CLIENTS     8
#define NUM_CHANNELS    2

void init_enet_client(const char *ip, unsigned port);
void init_enet_server(unsigned port);
void uninit_enet(void);

ENetHost *get_client_host(void);
ENetHost *get_server_host(void);
ENetPeer *get_peer(void);

static inline void broadcast_packet(unsigned char *buf, size_t len, bool reliable)
{
    ENetPacket *packet = enet_packet_create(NULL, len, reliable);
    memcpy(packet->data, buf, len);
    enet_host_broadcast(get_server_host(), 0, packet);
}

static inline void send_packet(unsigned char *buf, size_t len, bool reliable)
{
    ENetPacket *packet = enet_packet_create(NULL, len, reliable);
    memcpy(packet->data, buf, len);
    enet_peer_send(get_peer(), 1, packet);
}

static inline void write8(unsigned char *buf, size_t *pos, uint8_t val)
{
    *(unsigned char *)(&buf[*pos]) = val;
    (*pos) += 1;
}

static inline void write16(unsigned char *buf, size_t *pos, uint16_t val)
{
    *(uint16_t *)(&buf[*pos]) = htons(val);
    (*pos) += 2;
}

static inline void write32(unsigned char *buf, size_t *pos, uint32_t val)
{
    *(uint32_t *)(&buf[*pos]) = htonl(val);
    (*pos) += 4;
}

static inline uint8_t read8(unsigned char *buf, size_t *pos)
{
    uint8_t val = *(char *)(&buf[*pos]);
    (*pos) += 1;
    return val;
}

static inline uint16_t read16(unsigned char *buf, size_t *pos)
{
    uint16_t val = ntohs(*(uint16_t *)(&buf[*pos]));
    (*pos) += 2;
    return val;
}

static inline uint32_t read32(unsigned char *buf, size_t *pos)
{
    uint32_t val = ntohl(*(uint32_t *)(&buf[*pos]));
    (*pos) += 4;
    return val;
}

#endif
