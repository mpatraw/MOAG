
#ifndef SERVER_H
#define SERVER_H

#include "moag_net.h"

namespace moag
{

enum
{
    CB_CLIENT_CONNECT,
    CB_SERVER_UPDATE
};

typedef void (*ServerCallback) (moag::Connection arg);

int OpenServer(int port);
void CloseServer(void);
void SetServerCallback(ServerCallback cb, int type);
void ServerTick(void);

}

#endif

