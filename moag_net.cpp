
#include "moag_net.h"
#include <SDL/SDL.h>
#include <SDL/SDL_net.h>

namespace moag
{

int OpenNet(void) {
    if (SDL_WasInit(SDL_INIT_EVERYTHING) == 0 || SDLNet_Init() == -1)
        return -1;
    return 0;
}

void CloseNet(void) {
    SDLNet_Quit();
}

Connection ListenOn(int port) {
    IPaddress ip;
    if (SDLNet_ResolveHost(&ip, NULL, port) == -1)
        return NULL;
    return SDLNet_TCP_Open(&ip);
}

Connection AcceptClient(Connection server) {
    return SDLNet_TCP_Accept(server);
}

Connection ConnectTo(const char *host, int port) {
    IPaddress ip;
    if (SDLNet_ResolveHost(&ip, host, port) == -1)
        return NULL;
    return SDLNet_TCP_Open(&ip);
}

void Disconnect(Connection con) {
    SDLNet_TCP_Close((TCPsocket)con);
}

int HasActivity(Connection con, int timeout) {
    SDLNet_SocketSet ss = SDLNet_AllocSocketSet(1);
    SDLNet_TCP_AddSocket(ss, con);
    if (SDLNet_CheckSockets(ss, timeout) != 1)
        return 0;
    SDLNet_FreeSocketSet(ss);
    return 1;
}

int SendRaw(Connection con, void *data, int len) {
    if (SDLNet_TCP_Send((TCPsocket)con, data, len) < len)
        return -1;
    return 0;
}

int ReceiveRaw(Connection con, void *data, int len) {
    int recvd = 0;
    while (recvd < len) {
        int ret = SDLNet_TCP_Recv((TCPsocket)con, &((char *)data)[recvd], len-recvd);
        if (ret <= 0)
            return -1;
        recvd += ret;
    }
    return 0;
}

void Pack32(Uint32 i, void *buffer) {
    SDLNet_Write32(i, buffer);
}

Uint32 Unpack32(void *buffer) {
    return SDLNet_Read32(buffer);
}

void Pack16(Uint16 i, void *buffer) {
    SDLNet_Write16(i, buffer);
}

Uint16 Unpack16(void *buffer) {
    return SDLNet_Read16(buffer);
}

}

