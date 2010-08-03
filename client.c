#include "moag.h"
#include <unistd.h>

Uint32 position = 0;
/* 0xXXRRGGBB */
Uint32 color = 0x00FFFFFF;
int informServer = 0;

Uint8 r = 255;
Uint8 g = 255;
Uint8 b = 255;

void color_select_draw(void)
{
    char buf[64];
    
    MOAG_SetBlock(0, 0, 70, 30, 0, 0, 0);
    
    sprintf(buf, "Red %d", r);
    MOAG_SetString(0, 0, buf, 255, 0, 0);
    sprintf(buf, "Green %d", g);
    MOAG_SetString(0, 10, buf, 0, 255, 0);
    sprintf(buf, "Blue %d", b);
    MOAG_SetString(0, 20, buf, 0, 0, 255);
}

void color_select_update(void)
{
    MOAG_ClientTick();
    
    MOAG_GrabEvents();
    
    if (MOAG_IsKeyDown('q'))
    {
        MOAG_PopLoopState();
        MOAG_SetBlock(0, 0, 70, 30, 0, 0, 0);
        return;
    }
    
    if (MOAG_IsKeyDown('r'))
        if (r-- <= 0)
            r = 0;
    if (MOAG_IsKeyDown('t'))
        if (r++ >= 255)
            r = 255;
    
    if (MOAG_IsKeyDown('g'))
        if (g-- <= 0)
            g = 0;
    if (MOAG_IsKeyDown('h'))
        if (g++ >= 255)
            g = 255;
    
    if (MOAG_IsKeyDown('b'))
        if (b-- <= 0)
            b = 0;
    if (MOAG_IsKeyDown('n'))
        if (b++ >= 255)
            b = 255;
    
    color = (r << 16) | (g << 8) | (b << 0);
    
    if (MOAG_IsQuitting())
        MOAG_QuitMainLoop();
}

void draw(void)
{

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
        MOAG_SetBlock(x + 1, y + 1, 3, 3, r, g, b);
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
    
    if (MOAG_IsKeyDown('s'))
    {
        MOAG_PushLoopState(MOAG_LS_NONBLOCKING);
        MOAG_SetLoopCallback(color_select_draw, MOAG_CB_DRAW);
        MOAG_SetLoopCallback(color_select_update, MOAG_CB_UPDATE);
        return;
    }
    
    if (MOAG_IsQuitting())
        MOAG_QuitMainLoop();
}

void client_update(void *arg)
{
    Uint32 buf[1];
    Uint32 xy;
    Uint32 c;
    int x, y;
    
    while (MOAG_HasActivity(arg, 0))
    {
        if (MOAG_RecieveRaw(arg, buf, 8) == -1)
        {
            printf("Disconnected from server!\n");
            exit(0);
        }
        
        xy = MOAG_Unpack32(buf);
        c = MOAG_Unpack32(buf + 1);
        
        if (xy == 0xFFFFFFFF)
        {
            MOAG_ClearWindow(0, 0, 0);
        }
        else
        {
            x = (xy >> 16) & 0xFFFF;
            y = (xy >> 0) & 0xFFFF;
            MOAG_SetBlock(x + 1, y + 1, 3, 3, (c >> 16), (c >> 8) & 0xFF, c & 0xFF);
        }
    }
    
    if (informServer)
    {
        MOAG_Pack32(position, buf);
        MOAG_Pack32(color, buf + 1);
        if (MOAG_SendRaw(arg, buf, 8) == -1)
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
    
    if (MOAG_SetFont("Nouveau_IBM.ttf", 12) == -1)
    {
        printf("Failed to open font\n");
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

