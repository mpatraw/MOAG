#include "moag.h"
#include <unistd.h>

#define MAX_CLIENTS 8

MOAG_Connection clients[MAX_CLIENTS] = {NULL};
int numClients = 0;

void client_connect(void *arg)
{
    int i;
    
    for (i = 0; i < MAX_CLIENTS; i++)
    {
        if (!clients[i])
        {
            clients[i] = arg;
            numClients += 1;
            printf("\nClient connected!\n");
            break;
        }
    }
    
    /* Too many clients. */
    if (i == MAX_CLIENTS)
    {
        printf("\nClient failed to connect, too many clients.\n");
        MOAG_Disconnect(arg);
        return;
    }
    
    fflush(stdout);
}

void server_update(void *arg)
{
    Uint32 buf[1];
    Uint32 data;
    int i, j;
    int timeout = 200 / (numClients ? numClients : 1);
    
    /* Loop through each client. */
    for (i = 0; i < MAX_CLIENTS; ++i)
    {
        while (clients[i] && MOAG_HasActivity(clients[i], timeout))
        {
            if (MOAG_RecieveRaw(clients[i], buf, 4) != -1)
            {
                data = MOAG_Unpack32(buf);
                /* Dispatch to the clients. */
                for (j = 0; j < MAX_CLIENTS; ++j)
                {
                    if (j != i && clients[j])
                    {
                        if (MOAG_SendRaw(clients[j], buf, 4) == -1)
                        {
                            printf("Client DCed\n");
                            MOAG_Disconnect(clients[j]);
                            numClients -= 1;
                            clients[j] = NULL;
                        }
                    }
                }
                fflush(stdout);
            }
            else
            {
                printf("Client DCed\n");
                MOAG_Disconnect(clients[i]);
                numClients -= 1;
                clients[i] = NULL;
            }
        }
    }
    
    if (numClients == 0)
        usleep(200000);
    
    printf(".");
    fflush(stdout);
}

int main(int argc, char *argv[])
{
    if (MOAG_OpenServer(8080) == -1)
    {
        printf("Failed to start server\n");
        return 1;
    }
    
    MOAG_SetServerCallback(client_connect, MOAG_CB_CLIENT_CONNECT);
    MOAG_SetServerCallback(server_update, MOAG_CB_SERVER_UPDATE);
    
    while (1)
        MOAG_ServerTick();
    
    MOAG_CloseServer();

    return 0;
}

