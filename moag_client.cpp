
#include "moag_client.h"
#include "moag_net.h"
#include <SDL/SDL.h>

namespace moag
{

Connection _host = NULL;
ClientCallback _clientUpdate = NULL;

int OpenClient(const char *server, int port) {
    // We only need the timer.
    if (SDL_Init(SDL_INIT_TIMER) == -1 || OpenNet(1) == -1)
        return -1;

    _host = ConnectTo(server, port);
    if (!_host)
        return -1;

    return 0;
}

void CloseClient() {
    if (_host)
        Disconnect(_host);
    CloseNet();
    SDL_Quit();
}

void SetClientCallback(ClientCallback cb, int type) {
    switch (type) {
    case CB_CLIENT_UPDATE:
        _clientUpdate = cb;
        break;
    default:
        break;
    }
}

void ClientTick() {
    if (_clientUpdate)
        _clientUpdate(_host);
}

}


