
#ifndef MOAG_SERVER_H
#define MOAG_SERVER_H

enum
{
    MOAG_CB_CLIENT_CONNECT,
    MOAG_CB_SERVER_UPDATE
};

typedef void (*MOAG_ServerCallback) (void *arg);

int MOAG_OpenServer(int port);
void MOAG_CloseServer(void);
void MOAG_SetServerCallback(MOAG_ServerCallback cb, int type);
void MOAG_ServerTick(void);

#endif

