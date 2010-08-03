#include "moag.h"
#include <unistd.h>

Uint32 position = 0;
int informServer = 0;

void draw(void)
{
    MOAG_UpdateWindow();
}

void update(void)
{
    int x, y;
    Uint32 ux, uy;
    
    
    
    MOAG_ClientTick();
    
    MOAG_GrabEvents();
    MOAG_GetMousePosition(&x, &y);
    if (MOAG_IsButtonDown(MOAG_BUTTON_LEFT))
    {
        MOAG_SetPixel(x, y, 255, 255, 255);
        ux = x;
        uy = y;
        position = (ux << 16) | (uy << 0);
        informServer = 1;
    }
    
    if (MOAG_IsKeyDown('c'))
    {
        MOAG_ClearWindow(0, 0, 0);
        position = 0xFFFFFFFF;
        informServer = 1;
    }
}

void client_update(void *arg)
{
    Uint32 buf[1];
    Uint32 data;
    int x, y;
    
    while (MOAG_HasActivity(arg, 0))
    {
        printf("Recieved\n");
        if (MOAG_RecieveRaw(arg, buf, 4) == -1)
        {
            printf("Disconnected from server!\n");
            exit(0);
        }
        
        data = MOAG_Unpack32(buf);
        
        if (data == 0xFFFFFFFF)
        {
            MOAG_ClearWindow(0, 0, 0);
        }
        else
        {
            x = (data >> 16) & 0xFFFF;
            y = (data >> 0) & 0xFFFF;
            MOAG_SetPixel(x, y, 255, 255, 255);
        }
    }
    
    if (informServer)
    {
        printf("%x\n", position);
        MOAG_Pack32(position, buf);
        if (MOAG_SendRaw(arg, buf, 4) == -1)
        {
            printf("Disconnected from server!\n");
            exit(0);
        }
        
        informServer = 0;
    }

    fflush(stdout);
}

int main(int argc, char *argv[])
{
    if (MOAG_OpenClient("localhost", 8080) == -1)
    {
        printf("Failed to open client\n");
        return 1;
    }

    if (MOAG_OpenWindow(640, 480, "sandbox") == -1)
    {
        printf("Failed to start window\n");
        return 1;
    }
    
    MOAG_SetClientCallback(client_update, MOAG_CB_CLIENT_UPDATE);
    MOAG_SetLoopCallback(draw, MOAG_CB_DRAW);
    MOAG_SetLoopCallback(update, MOAG_CB_UPDATE);
    
    MOAG_MainLoop();
    
    MOAG_CloseWindow();
    MOAG_CloseClient();
    return 0;
}

