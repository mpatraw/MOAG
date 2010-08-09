
#include "moag_net.h"
#include <algorithm>
#include <SDL/SDL.h>
#include <SDL/SDL_net.h>
#include <vector>

using std::vector;
using std::find;

namespace moag
{

static SDLNet_SocketSet _socketSet;

int OpenNet(int maxConnections) {
    if (SDL_WasInit(SDL_INIT_EVERYTHING) == 0 || SDLNet_Init() == -1)
        return -1;
    
    _socketSet = SDLNet_AllocSocketSet(maxConnections);
    if (!_socketSet) {
        SDLNet_Quit();
        return -1;
    }
    
    return 0;
}

void CloseNet(void) {
    SDLNet_FreeSocketSet(_socketSet);
    SDLNet_Quit();
}

Connection ListenOn(int port) {
    IPaddress ip;
    
    if (SDLNet_ResolveHost(&ip, NULL, port) == -1)
        return NULL;
    
    TCPsocket con = SDLNet_TCP_Open(&ip);
    if (con)
        SDLNet_TCP_AddSocket(_socketSet, con);
        
    return con;
}

Connection AcceptClient(Connection server) {
    TCPsocket con = SDLNet_TCP_Accept(server);
    if (con)
        SDLNet_TCP_AddSocket(_socketSet, con);
    return con;
}

Connection ConnectTo(const char *host, int port) {
    IPaddress ip;
    
    if (SDLNet_ResolveHost(&ip, host, port) == -1)
        return NULL;
    
    TCPsocket con = SDLNet_TCP_Open(&ip);
    if (con)
        SDLNet_TCP_AddSocket(_socketSet, con);
    
    return con;
}

void Disconnect(Connection con) {
    SDLNet_TCP_DelSocket(_socketSet, con);
    SDLNet_TCP_Close((TCPsocket)con);
}

int HasActivity(Connection con, int timeout) {
    if (SDLNet_CheckSockets(_socketSet, timeout) < 1)
        return 0;
    if (SDLNet_SocketReady(con) != 0)
        return 1;
    return 0;
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

