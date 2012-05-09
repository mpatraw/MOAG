
#ifndef ENET_AUX_H
#define ENET_AUX_H

#include <enet/enet.h>

#define CONNECT_TIMEOUT 5000
#define MAX_CLIENTS     8
#define NUM_CHANNELS    2

void init_enet_client(const char *ip, unsigned port);
void init_enet_server(unsigned port);
void uninit_enet(void);

ENetHost *get_client_host(void);
ENetHost *get_server_host(void);
ENetPeer *get_peer(void);

#endif
