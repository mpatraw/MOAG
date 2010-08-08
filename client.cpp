#include "moag.h"
#include <unistd.h>
#include <math.h>

#include "tank.h"

struct Bullet{
    Uint16 x,y;
};

#define MAX_CLIENTS 8
#define MAX_BULLETS 256
#define BUFLEN 256

#define WIDTH 800
#define HEIGHT 600

#define LAND_CHUNK   1
#define TANK_CHUNK   2
#define BULLET_CHUNK 3

static char land[WIDTH*HEIGHT];
static struct Tank tanks[MAX_CLIENTS];
static struct Bullet bullets[MAX_BULLETS];
static struct Tank keys;
static int numBullets=0;

inline char landAt(int x, int y){
    if(x<0 || x>=WIDTH || y<0 || y>=HEIGHT)
        return -1;
    return land[y*WIDTH+x];
}

inline void setLandAt(int x, int y, char to) {
    if(x<0 || x>=WIDTH || y<0 || y>=HEIGHT)
        return;
    land[y*WIDTH+x] = to;
}

void redrawLand(int x, int y, int w, int h){
    int ix,iy;
    if(x<0){ w+=x; x=0; }
    if(y<0){ h+=y; y=0; }
    if(x+w>WIDTH) w=WIDTH-x;
    if(y+h>HEIGHT) h=HEIGHT-y;
    if(w<=0 || h<=0 || x+w>WIDTH || y+h>HEIGHT)
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
    if(x<0 || x>WIDTH-18 || y<0 || y>HEIGHT-14)
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
        if(bullets[i].x<1 || bullets[i].x>=WIDTH-1 || bullets[i].y<1 || bullets[i].y>=HEIGHT-1)
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
        if(bullets[i].x<1 || bullets[i].x>=WIDTH-1 || bullets[i].y<1 || bullets[i].y>=HEIGHT-1)
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
            char str[2];
            str[0] = i+'0';
            str[1] = '\0';
            drawTank(tanks[i].x-9,tanks[i].y-13,tanks[i].angle,tanks[i].facingLeft);
            MOAG_SetString(tanks[i].x-4,tanks[i].y-32,str,255,255,255);
            tanks[i].lastx=tanks[i].x;
            tanks[i].lasty=tanks[i].y;
        }
    drawBullets();
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
    MOAG_ChunkEnqueue8(k);
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



void client_update(MOAG_Connection arg)
{
    char byte;

    while (MOAG_HasActivity(arg, 0)){
        if (MOAG_ReceiveChunk(arg, 1) == -1){
            printf("Disconnected from server!\n");
            exit(0);
        }
        
        byte = MOAG_ChunkDequeue8();
        
        switch(byte) {
        case LAND_CHUNK: { /* updated rectangle of land */
            /*XXX DOESN'T WORK XXX
            int x,y,w,h;
            int xx, yy;
            
            MOAG_ReceiveChunk(arg, 8);
            x = MOAG_ChunkDequeue16();
            y = MOAG_ChunkDequeue16();
            w = MOAG_ChunkDequeue16();
            h = MOAG_ChunkDequeue16();
            if(w<0) w=0;
            if(h<0) h=0;
            if(x<0 || y<0 || x+w>WIDTH || y+h>HEIGHT)
                break;
            
            MOAG_ReceiveChunk(arg, w*h);
            for (yy = y; yy < h + y; ++yy)
                for (xx = x; xx < w + x; ++xx)
                    setLandAt(xx, yy, MOAG_ChunkDequeue8());

            printf("%d, %d: %d, %d\n", x, y, w, h);
            printf("Size: %d\n", MOAG_IncomingChunkLength());
            fflush(stdout);
            redrawLand(x,y,w,h);*/
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
            if(x<0 || y<0 || x+w>WIDTH || y+h>HEIGHT)
                break;
            for(i=0;i<h;i++)
                MOAG_ReceiveRaw(arg, &land[(y+i)*WIDTH+x], w);
            redrawLand(x,y,w,h);
        } break;
        case TANK_CHUNK: { /* updated tank position */
            int id;
            short x,y;
            char angle;
            char facingLeft=0;
            
            MOAG_ReceiveChunk(arg, 6);
            id = MOAG_ChunkDequeue8();
            x = (short)MOAG_ChunkDequeue16();
            y = (short)MOAG_ChunkDequeue16();
            angle = MOAG_ChunkDequeue8();
            
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
        case BULLET_CHUNK: { /* bullets */
            int i;
            undrawBullets();
            
            MOAG_ReceiveChunk(arg, 2);
            numBullets = MOAG_ChunkDequeue16();
            
            if(numBullets<=0)
                break;
            if(numBullets>=MAX_BULLETS){ /* error! */
                numBullets=0;
                break;
            }
            
            MOAG_ReceiveChunk(arg, numBullets*4);
            for (i = 0; i < numBullets; i++) {
                bullets[i].x = MOAG_ChunkDequeue16();
                bullets[i].y = MOAG_ChunkDequeue16();
            }
        } break;
        default:
            printf("unknown byte: %d\n",byte);
            break;
        }
    }

    if (MOAG_SendChunk(arg, -1, 1) == -1) {
        printf("Disconnected from server!\n");
        exit(0);
    }

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

    if (MOAG_OpenWindow(WIDTH, HEIGHT, "MOAG") == -1) {
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

