
#include "moag_server.h"
#include "moag_net.h"
#include <SDL/SDL.h>

namespace moag
{

moag::Connection _server = NULL;

int OpenServer(int port) {
    // We only need the timer.
    if (SDL_Init(SDL_INIT_TIMER) == -1 || moag::OpenNet() == -1)
        return -1;

    _server = moag::ListenOn(port);
    if (!_server)
        return -1;

    return 0;
}

void CloseServer() {
    if (_server)
        moag::Disconnect(_server);
    moag::CloseNet();
    SDL_Quit();
}

ServerCallback _clientConnect = NULL;
ServerCallback _serverUpdate = NULL;

void SetServerCallback(ServerCallback cb, int type) {
    switch (type) {
    case CB_CLIENT_CONNECT:
        _clientConnect = cb;
        break;
    case CB_SERVER_UPDATE:
        _serverUpdate = cb;
        break;
    default:
        break;
    }
}

void ServerTick() {
    if (_serverUpdate)
        _serverUpdate(_server);

    moag::Connection con = moag::AcceptClient(_server);

    if (con) {
        if (_clientConnect)
            _clientConnect(con);
        else
            moag::Disconnect(con);
    }
}

}

