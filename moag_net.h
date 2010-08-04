
#ifndef MOAG_NET_H
#define MOAG_NET_H

#include <SDL/SDL_types.h>

typedef void *MOAG_Connection;

int MOAG_OpenNet(void);
void MOAG_CloseNet(void);

MOAG_Connection MOAG_ListenOn(int port);
MOAG_Connection MOAG_AcceptClient(MOAG_Connection server);

MOAG_Connection MOAG_ConnectTo(const char *host, int port);
void MOAG_Disconnect(MOAG_Connection con);

int MOAG_HasActivity(MOAG_Connection con, int timeout);

int MOAG_SendRaw(MOAG_Connection con, void *data, int len);
int MOAG_ReceiveRaw(MOAG_Connection con, void *data, int len);
void MOAG_Pack32(Uint32 i, void *buffer);
Uint32 MOAG_Unpack32(void *buffer);
void MOAG_Pack16(Uint16 i, void *buffer);
Uint16 MOAG_Unpack16(void *buffer);

#endif

