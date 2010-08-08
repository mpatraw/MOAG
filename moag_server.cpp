
#include "moag_server.h"
#include "moag_net.h"
#include <SDL/SDL.h>

static MOAG_Connection _server = NULL;

int MOAG_OpenServer(int port)
{
    /* We only need the timer. */
    if (SDL_Init(SDL_INIT_TIMER) == -1)
        return -1;

    if (MOAG_OpenNet() == -1)
        return -1;
    
    _server = MOAG_ListenOn(port);
    if (!_server)
        return -1;
    
    return 0;
}

void MOAG_CloseServer(void)
{
    if (_server)
        MOAG_Disconnect(_server);
    MOAG_CloseNet();
    SDL_Quit();
}

static MOAG_ServerCallback _clientConnect = NULL;
static MOAG_ServerCallback _serverUpdate = NULL;

void MOAG_SetServerCallback(MOAG_ServerCallback cb, int type)
{
    switch (type)
    {
    case MOAG_CB_CLIENT_CONNECT:
        _clientConnect = cb;
        break;
    case MOAG_CB_SERVER_UPDATE:
        _serverUpdate = cb;
        break;
    default:
        break;
    }
}

void MOAG_ServerTick(void)
{
    MOAG_Connection con = NULL;

    if (_serverUpdate)
        _serverUpdate(_server);
            
    if (!con)
    {
        con = MOAG_AcceptClient(_server);
        if (con)
        {
            if (_clientConnect)
            {
                /* This function should use the client
                   or save it. */
                _clientConnect(con);
                con = NULL;
            }
            else
            {
                MOAG_Disconnect(con);
                con = NULL;
            }
        }
    }
}

