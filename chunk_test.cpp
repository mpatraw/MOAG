#include "moag.h"

void client_update(void *arg)
{
    char string[256];
    int i;
    
    if (MOAG_HasActivity(arg, 0))
    {
        if (MOAG_ReceiveChunk(arg, 513) == -1)
        {
            printf("DCed\n");
            exit(1);
        }
        
        printf("%d\n", MOAG_ChunkDequeue8());
        for (i = 0; i < 256; ++i)
            printf("%d\n", MOAG_ChunkDequeue16());
    }
}

void client_connect(void *arg)
{
    int i;
    MOAG_ChunkEnqueue8(0);
    for (i = 0; i < 256; ++i)
        MOAG_ChunkEnqueue16(i);
    
    MOAG_SendChunk(arg, -1, 1);
    
    MOAG_Disconnect(arg);
}

void server_update(void *arg)
{

}

int main(int argc, char *argv[])
{
    if (argc == 1)
    {
        MOAG_OpenClient("localhost", 8080);
        
        MOAG_SetClientCallback(client_update, MOAG_CB_CLIENT_UPDATE);
        
        while (1)
            MOAG_ClientTick();
        
        MOAG_CloseClient();
    }
    else
    {
        MOAG_OpenServer(8080);
        
        MOAG_SetServerCallback(client_connect, MOAG_CB_CLIENT_CONNECT);
        MOAG_SetServerCallback(server_update, MOAG_CB_SERVER_UPDATE);
        
        while (1)
            MOAG_ServerTick();
        
        MOAG_CloseServer();
    }
    return 0;
}

