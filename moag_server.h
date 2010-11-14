
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

class CallbackObject {
	public:
		virtual int operator()(Connection) = 0;
};

int OpenServer(int port, int maxConnections);
void CloseServer(void);
void SetServerCallback(CallbackObject* cb, int type);
void ServerTick(void);

}

#endif

