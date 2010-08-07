
#include "moag_net.h"
#include <SDL/SDL.h>
#include <SDL/SDL_net.h>

int MOAG_OpenNet(void)
{
    Uint32 systems;
    
    systems = SDL_WasInit(SDL_INIT_EVERYTHING);
    
    if (systems == 0)
        return -1;
    
    if (SDLNet_Init() == -1)
        return -1;
    
    return 0;
}

void MOAG_CloseNet(void)
{
    SDLNet_Quit();
}

MOAG_Connection MOAG_ListenOn(int port)
{
    MOAG_Connection serv;
    IPaddress ip;

    if (SDLNet_ResolveHost(&ip, NULL, port) == -1)
        return NULL;
    
    serv = SDLNet_TCP_Open(&ip);
    if (!serv)
        return NULL;
    
    return serv;
}

MOAG_Connection MOAG_AcceptClient(MOAG_Connection server)
{
    TCPsocket sock;

    sock = SDLNet_TCP_Accept(server);
    if (!sock)
        return NULL;
    
    return sock;
}

MOAG_Connection MOAG_ConnectTo(const char *host, int port)
{
    TCPsocket sock;
    IPaddress ip;

    if (SDLNet_ResolveHost(&ip, host, port) == -1)
        return NULL;
    
    sock = SDLNet_TCP_Open(&ip);
    if (!sock)
        return NULL;
    
    return sock;
}

void MOAG_Disconnect(MOAG_Connection con)
{
    SDLNet_TCP_Close((TCPsocket)con);
}

int MOAG_HasActivity(MOAG_Connection con, int timeout)
{
    SDLNet_SocketSet ss;
    ss = SDLNet_AllocSocketSet(1);
    
    SDLNet_AddSocket(ss, con);
    if (SDLNet_CheckSockets(ss, timeout) != 1)
        return 0;
    
    SDLNet_FreeSocketSet(ss);
    
    return 1;
}

int MOAG_SendRaw(MOAG_Connection con, void *data, int len)
{
    TCPsocket sock;
    int sent;
    
    sock = (TCPsocket)con;
    
    sent = SDLNet_TCP_Send(sock, data, len);
    
    if (sent < len)
        return -1;
    
    return 0;
}

int MOAG_ReceiveRaw(MOAG_Connection con, void *data, int len)
{
    TCPsocket sock;
    int recvd = 0;
    
    sock = (TCPsocket)con;
    
    while (recvd < len)
    {
        int ret = SDLNet_TCP_Recv(sock, &((char *)data)[recvd], len-recvd);
        if (ret <= 0)
            return -1;
        recvd += ret;
    }
    
    return 0;
}

void MOAG_Pack32(Uint32 i, void *buffer)
{
    SDLNet_Write32(i, buffer);
}

Uint32 MOAG_Unpack32(void *buffer)
{
    return SDLNet_Read32(buffer);
}

void MOAG_Pack16(Uint16 i, void *buffer)
{
    SDLNet_Write16(i, buffer);
}

Uint16 MOAG_Unpack16(void *buffer)
{
    return SDLNet_Read16(buffer);
}

