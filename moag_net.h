
#ifndef NET_H
#define NET_H

#include <SDL/SDL_types.h>
#include <SDL/SDL_net.h>

namespace moag
{

typedef TCPsocket Connection;

int OpenNet(void);
void CloseNet(void);

Connection ListenOn(int port);
Connection AcceptClient(Connection server);

Connection ConnectTo(const char *host, int port);
void Disconnect(Connection con);

int HasActivity(Connection con, int timeout);

int SendRaw(Connection con, void *data, int len);
int ReceiveRaw(Connection con, void *data, int len);
void Pack32(Uint32 i, void *buffer);
Uint32 Unpack32(void *buffer);
void Pack16(Uint16 i, void *buffer);
Uint16 Unpack16(void *buffer);

}

#endif

