
#ifndef CLIENT_H
#define CLIENT_H

#include "moag_net.h"

namespace moag
{

enum
{
    CB_CLIENT_UPDATE
};

typedef void (*ClientCallback) (moag::Connection args);

int OpenClient(const char *server, int port);
void CloseClient(void);
void SetClientCallback(ClientCallback cb, int type);
void ClientTick(void);

}

#endif

