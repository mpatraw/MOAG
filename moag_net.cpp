
#include "moag_net.h"
#include <SDL/SDL.h>
#include <SDL/SDL_net.h>

int MOAG_OpenNet(void) {
    if (SDL_WasInit(SDL_INIT_EVERYTHING) == 0 || SDLNet_Init() == -1)
        return -1;
    return 0;
}

void MOAG_CloseNet(void) {
    SDLNet_Quit();
}

MOAG_Connection MOAG_ListenOn(int port) {
    IPaddress ip;
    if (SDLNet_ResolveHost(&ip, NULL, port) == -1)
        return NULL;
    return SDLNet_TCP_Open(&ip);
}

MOAG_Connection MOAG_AcceptClient(MOAG_Connection server) {
    return SDLNet_TCP_Accept(server);
}

MOAG_Connection MOAG_ConnectTo(const char *host, int port) {
    IPaddress ip;
    if (SDLNet_ResolveHost(&ip, host, port) == -1)
        return NULL;
    return SDLNet_TCP_Open(&ip);
}

void MOAG_Disconnect(MOAG_Connection con) {
    SDLNet_TCP_Close((TCPsocket)con);
}

int MOAG_HasActivity(MOAG_Connection con, int timeout) {
    SDLNet_SocketSet ss = SDLNet_AllocSocketSet(1);
    SDLNet_TCP_AddSocket(ss, con);
    if (SDLNet_CheckSockets(ss, timeout) != 1)
        return 0;
    SDLNet_FreeSocketSet(ss);
    return 1;
}

int MOAG_SendRaw(MOAG_Connection con, void *data, int len) {
    if (SDLNet_TCP_Send((TCPsocket)con, data, len) < len)
        return -1;
    return 0;
}

int MOAG_ReceiveRaw(MOAG_Connection con, void *data, int len) {
    int recvd = 0;
    while (recvd < len) {
        int ret = SDLNet_TCP_Recv((TCPsocket)con, &((char *)data)[recvd], len-recvd);
        if (ret <= 0)
            return -1;
        recvd += ret;
    }
    return 0;
}

void MOAG_Pack32(Uint32 i, void *buffer) {
    SDLNet_Write32(i, buffer);
}

Uint32 MOAG_Unpack32(void *buffer) {
    return SDLNet_Read32(buffer);
}

void MOAG_Pack16(Uint16 i, void *buffer) {
    SDLNet_Write16(i, buffer);
}

Uint16 MOAG_Unpack16(void *buffer) {
    return SDLNet_Read16(buffer);
}

