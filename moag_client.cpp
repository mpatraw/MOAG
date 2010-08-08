
#include "moag_client.h"
#include "moag_net.h"
#include <SDL/SDL.h>

MOAG_Connection _host = NULL;
MOAG_ClientCallback _clientUpdate = NULL;

int MOAG_OpenClient(const char *server, int port) {
    // We only need the timer.
    if (SDL_Init(SDL_INIT_TIMER) == -1 || MOAG_OpenNet() == -1)
        return -1;

    _host = MOAG_ConnectTo(server, port);
    if (!_host)
        return -1;

    return 0;
}

void MOAG_CloseClient() {
    if (_host)
        MOAG_Disconnect(_host);
    MOAG_CloseNet();
    SDL_Quit();
}

void MOAG_SetClientCallback(MOAG_ClientCallback cb, int type) {
    switch (type) {
    case MOAG_CB_CLIENT_UPDATE:
        _clientUpdate = cb;
        break;
    default:
        break;
    }
}

void MOAG_ClientTick() {
    if (_clientUpdate)
        _clientUpdate(_host);
}



