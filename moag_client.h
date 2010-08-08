
#ifndef MOAG_CLIENT_H
#define MOAG_CLIENT_H

#include "moag_net.h"

enum
{
    MOAG_CB_CLIENT_UPDATE
};

typedef void (*MOAG_ClientCallback) (MOAG_Connection args);

int MOAG_OpenClient(const char *server, int port);
void MOAG_CloseClient(void);
void MOAG_SetClientCallback(MOAG_ClientCallback cb, int type);
void MOAG_ClientTick(void);

#endif

