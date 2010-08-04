#include "moag.h"
#include <unistd.h>
#include <math.h>

#include "tank.h"

struct Bullet{
    short x,y;
};

#define MAX_CLIENTS 8
#define MAX_BULLETS 256
#define BUFLEN 256

static char land[800*600];
static struct Tank tanks[MAX_CLIENTS];
static struct Bullet bullets[MAX_BULLETS];
static struct Tank keys;
static char sendbuf[BUFLEN];
static int sendbuflen=0;
static int numBullets=0;

inline char landAt(int x, int y){
    if(x<0 || x>=800 || y<0 || y>=600)
        return -1;
    return land[y*800+x];
}

void redrawLand(int x, int y, int w, int h){
    int ix,iy;
    if(x<0){ w+=x; x=0; }
    if(y<0){ h+=y; y=0; }
    if(x+w>800) w=800-x;
    if(y+h>800) h=600-y;
    if(w<=0 || h<=0 || x+w>800 || y+h>600)
        return;
    for(iy=y;iy<y+h;iy++)
    for(ix=x;ix<x+w;ix++)
        if(landAt(ix,iy)==0)
            MOAG_SetPixel(ix,iy,0,0,0);
        else
            MOAG_SetPixel(ix,iy,160,160,160);
}

const char tanksprite[14][19]={
"..................",
"..................",
"..................",
"..................",
"..................",
"........xx........",
".......xxxx.......",
"......xxxxxx......",
"...xxxxxxxxxxxx...",
".xxxxxxxxxxxxxxxx.",
"xxxxxxxxxxxxxxxxxx",
"xxx.xx.xx.xx.xx.xx",
".x..x..x..x..x..x.",
"..xxxxxxxxxxxxxx..",
};

void drawTank(int x, int y, int turretangle, char facingLeft){
    int ix,iy;
    float step;
    float rise=0;
    int next;
    int xlen;
    if(x<0 || x>800-18 || y<0 || y>600-14)
        return;
    for(iy=0;iy<14;iy++)
    for(ix=0;ix<18;ix++)
        if(tanksprite[iy][ix]=='x')
            MOAG_SetPixel(x+ix,y+iy,255,255,255);
    if(turretangle==90)
        turretangle=89;
    step=tanf((float)turretangle*M_PI/180.0);
    xlen=6.5*cosf((float)turretangle*M_PI/180.0);
    for(ix=0;ix<=xlen;ix++){
        next=rise+step;
        if(next>6) next=6;
        for(iy=-(int)rise;iy>=-next;iy--)
            MOAG_SetPixel(x+(facingLeft?9-ix:8+ix),y+6+iy,255,255,255);
        rise+=step;
    }
}

void drawBullets(){
    int i;
    for(i=0;i<numBullets;i++){
        if(bullets[i].x<1 || bullets[i].x>=800-1 || bullets[i].y<1 || bullets[i].y>=600-1)
            continue;
        MOAG_SetPixel(bullets[i].x,bullets[i].y-1,255,255,255);
        MOAG_SetPixel(bullets[i].x-1,bullets[i].y,255,255,255);
        MOAG_SetPixel(bullets[i].x,  bullets[i].y,255,255,255);
        MOAG_SetPixel(bullets[i].x+1,bullets[i].y,255,255,255);
        MOAG_SetPixel(bullets[i].x,bullets[i].y+1,255,255,255);
    }
}

void undrawBullets(){
    int i;
    for(i=0;i<numBullets;i++){
        if(bullets[i].x<1 || bullets[i].x>=800-1 || bullets[i].y<1 || bullets[i].y>=600-1)
            return;
        MOAG_SetPixel(bullets[i].x,bullets[i].y-1,40,40,40);
        MOAG_SetPixel(bullets[i].x-1,bullets[i].y,40,40,40);
        MOAG_SetPixel(bullets[i].x,  bullets[i].y,40,40,40);
        MOAG_SetPixel(bullets[i].x+1,bullets[i].y,40,40,40);
        MOAG_SetPixel(bullets[i].x,bullets[i].y+1,40,40,40);
    }
    numBullets=0;
}

void draw(void) {
    int i;
    for(i=0;i<MAX_CLIENTS;i++)
        if(tanks[i].active)
            redrawLand(tanks[i].lastx-9,tanks[i].lasty-32,18,33);
    for(i=0;i<MAX_CLIENTS;i++)
        if(tanks[i].active){
            const char str[]={i+'0','\0'};
            drawTank(tanks[i].x-9,tanks[i].y-13,tanks[i].angle,tanks[i].facingLeft);
            MOAG_SetString(tanks[i].x-4,tanks[i].y-32,str,255,255,255);
            tanks[i].lastx=tanks[i].x;
            tanks[i].lasty=tanks[i].y;
        }
}



void initClient(void) {
    int i;
    for(i=0;i<MAX_CLIENTS;i++){
        tanks[i].active=0;
        tanks[i].lastx=0;
        tanks[i].lasty=0;
    }
    keys.kLeft=0;
    keys.kRight=0;
    keys.kUp=0;
    keys.kDown=0;
    keys.kFire=0;
}

void sendByte(char k){
    if(sendbuflen>=BUFLEN)
        return;
    sendbuf[sendbuflen++]=k;
}

void update(void) {
    MOAG_ClientTick();
    MOAG_GrabEvents();
    
    if(MOAG_IsKeyPressed(SDLK_LEFT)){
        sendByte(1); keys.kLeft=1;
    }else if(MOAG_IsKeyReleased(SDLK_LEFT)){
        sendByte(2); keys.kLeft=0;
    }
    if(MOAG_IsKeyPressed(SDLK_RIGHT)){
        sendByte(3); keys.kRight=1;
    }else if(MOAG_IsKeyReleased(SDLK_RIGHT)){
        sendByte(4); keys.kRight=0;
    }
    if(MOAG_IsKeyPressed(SDLK_UP)){
        sendByte(5); keys.kUp=1;
    }else if(MOAG_IsKeyReleased(SDLK_UP)){
        sendByte(6); keys.kUp=0;
    }
    if(MOAG_IsKeyPressed(SDLK_DOWN)){
        sendByte(7); keys.kDown=1;
    }else if(MOAG_IsKeyReleased(SDLK_DOWN)){
        sendByte(8); keys.kDown=0;
    }
    if(MOAG_IsKeyPressed(SDLK_SPACE)){
        sendByte(9); keys.kFire=1;
    }else if(MOAG_IsKeyReleased(SDLK_SPACE)){
        sendByte(10); keys.kFire=0;
    }
    
    if (MOAG_IsKeyPressed(SDLK_ESCAPE) || MOAG_IsQuitting())
        MOAG_QuitMainLoop();
}



void client_update(void *arg)
{
    char byte;

    while (MOAG_HasActivity(arg, 0)){
        if (MOAG_ReceiveRaw(arg, &byte, 1) == -1){
            printf("Disconnected from server!\n");
            exit(0);
        }

        /* printf("server sent %d\n",byte); */
        switch(byte){
        case 1: { /* updated rectangle of land */
            char buf[8];
            int x,y,w,h;
            int i;
            MOAG_ReceiveRaw(arg, buf, 8);
            x = MOAG_Unpack16(&buf[0]);
            y = MOAG_Unpack16(&buf[2]);
            w = MOAG_Unpack16(&buf[4]);
            h = MOAG_Unpack16(&buf[6]);
            if(w<0) w=0;
            if(h<0) h=0;
            if(x<0 || y<0 || x+w>800 || y+h>600)
                break;
            for(i=0;i<h;i++)
                MOAG_ReceiveRaw(arg, &land[(y+i)*800+x], w);
            redrawLand(x,y,w,h);
        } break;
        case 2: { /* updated tank position */
            char buf[6];
            int id;
            short x,y;
            char angle;
            char facingLeft=0;
            MOAG_ReceiveRaw(arg, buf, 6);
            id = buf[0];
            x = MOAG_Unpack16(&buf[1]);
            y = MOAG_Unpack16(&buf[3]);
            angle = buf[5];
            if(id<0 || id>=MAX_CLIENTS)
                break;
            if(angle<0){
                angle=-angle;
                facingLeft=1;
            }
            if(x==-1 && y==-1){
                redrawLand(tanks[id].lastx-9,tanks[id].lasty-32,18,33);
                tanks[id].active=0;
                break;
            }
            tanks[id].active=1;
            tanks[id].x=x;
            tanks[id].y=y;
            tanks[id].angle=angle;
            tanks[id].facingLeft=facingLeft;
        } break;
        case 3: { /* bullets */
            char buf[2];
            char* data;
            int i;
            undrawBullets();
            MOAG_ReceiveRaw(arg, buf, 2);
            numBullets = MOAG_Unpack16(&buf[0]);
            if(numBullets<=0)
                break;
            if(numBullets>=MAX_BULLETS){ /* error! */
                numBullets=0;
                break;
            }
            data = malloc(numBullets*4);
            MOAG_ReceiveRaw(arg, data, numBullets*4);
            for(i=0;i<numBullets;i++){
                bullets[i].x = MOAG_Unpack16(&data[i*4]);
                bullets[i].y = MOAG_Unpack16(&data[i*4+2]);
            }
            free(data);
            drawBullets();
        } break;
        default: break;
        }
    }

    if(sendbuflen>0 && MOAG_SendRaw(arg, &sendbuf, sendbuflen) == -1){
        printf("Disconnected from server!\n");
        exit(0);
    }
    sendbuflen=0;

    fflush(stdout);
}

int main(int argc, char *argv[])
{
    int port=8080;

    if (argc<2) {
        printf("usage:  %s [address] {[port]}\n", argv[0]);
        exit(0);
    }

    if (argc>=3)
        port = atoi(argv[2]);

    if (MOAG_OpenClient(argv[1], port) == -1) {
        printf("Failed to open client\n");
        return 1;
    }

    if (MOAG_OpenWindow(800, 600, "MOAG") == -1) {
        printf("Failed to start window\n");
        return 1;
    }
    
    if (MOAG_SetFont("Nouveau_IBM.ttf", 12) == -1) {
        printf("Failed to open font\n");
        return 1;
    }
    
    MOAG_SetClientCallback(client_update, MOAG_CB_CLIENT_UPDATE);
    MOAG_SetLoopCallback(draw, MOAG_CB_DRAW);
    MOAG_SetLoopCallback(update, MOAG_CB_UPDATE);

    initClient();
    
    MOAG_MainLoop();
    
    MOAG_CloseWindow();
    MOAG_CloseClient();
    return 0;
}

