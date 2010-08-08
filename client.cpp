#include "moag.h"
#include <unistd.h>
#include <math.h>
#include <string.h>

const int MAX_CLIENTS = 8;
const int MAX_BULLETS = 256;
const int BUFLEN = 256;
const int CHAT_LINES = 7;
const int CHAT_EXPIRETIME = 14000;

const int WIDTH = 800;
const int HEIGHT = 600;

const int LAND_CHUNK   = 1;
const int TANK_CHUNK   = 2;
const int BULLET_CHUNK = 3;
const int MSG_CHUNK    = 4;

struct BulletPos{
    Uint16 x,y;
};

struct ChatLine{
    int expire;
    char* str;
};

struct Redraw{
    int x,y,w,h;
    Redraw* next;
    Redraw(int _x, int _y, int _w, int _h) : x(_x),y(_y),w(_w),h(_h),next(NULL) {}
};

ChatLine chatlines[CHAT_LINES];
char* typingStr=NULL;
bool typingDone=false;
Redraw redraws(0,0,0,0);
char land[WIDTH*HEIGHT];
Tank tanks[MAX_CLIENTS];
Tank keys;
BulletPos bullets[MAX_BULLETS];
int numBullets=0;

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
    if(x<0){ w+=x; x=0; }
    if(y<0){ h+=y; y=0; }
    if(x+w>WIDTH) w=WIDTH-x;
    if(y+h>HEIGHT) h=HEIGHT-y;
    if(w<=0 || h<=0 || x+w>WIDTH || y+h>HEIGHT)
        return;
    for(int iy=y;iy<y+h;iy++)
    for(int ix=x;ix<x+w;ix++)
        if(landAt(ix,iy)==0)
            MOAG_SetPixel(ix,iy,0,0,0);
        else
            MOAG_SetPixel(ix,iy,155,155,155);
}

void pushRedraw(int x, int y, int w, int h){
    Redraw* p=&redraws;
    while(p->next)
        p=p->next;
    p->next=new Redraw(x,y,w,h);
}

void redraw(){
    Redraw* p=redraws.next;
    while(p){
        redrawLand(p->x,p->y,p->w,p->h);
        Redraw* q=p;
        p=p->next;
        delete q;
    }
    redraws.next=NULL;
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
    if(x<0 || x>WIDTH-18 || y<0 || y>HEIGHT-14)
        return;
    for(int iy=0;iy<14;iy++)
    for(int ix=0;ix<18;ix++)
        if(tanksprite[iy][ix]=='x')
            MOAG_SetPixel(x+ix,y+iy,240,240,240);
    if(turretangle==90)
        turretangle=89;

    float step=tanf((float)turretangle*M_PI/180.0);
    int xlen=6.5*cosf((float)turretangle*M_PI/180.0);
    float rise=0;
    for(int ix=0;ix<=xlen;ix++){
        int next=rise+step;
        if(next>6) next=6;
        for(int iy=-(int)rise;iy>=-next;iy--)
            MOAG_SetPixel(x+(facingLeft?9-ix:8+ix),y+6+iy,240,240,240);
        rise+=step;
    }
}

void drawBullets(){
    for(int i=0;i<numBullets;i++){
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
    for(int i=0;i<numBullets;i++){
        if(bullets[i].x<1 || bullets[i].x>=WIDTH-1 || bullets[i].y<1 || bullets[i].y>=HEIGHT-1)
            return;
        MOAG_SetPixel(bullets[i].x,bullets[i].y-1,30,30,30);
        MOAG_SetPixel(bullets[i].x-1,bullets[i].y,30,30,30);
        MOAG_SetPixel(bullets[i].x,  bullets[i].y,30,30,30);
        MOAG_SetPixel(bullets[i].x+1,bullets[i].y,30,30,30);
        MOAG_SetPixel(bullets[i].x,bullets[i].y+1,30,30,30);
    }
    numBullets=0;
}

void delChatLine(){
    if(chatlines[0].str && chatlines[0].expire<MOAG_GetTicks()){
        delete[] chatlines[0].str;
        for(int i=0;i<CHAT_LINES-1;i++){
            chatlines[i].expire=chatlines[i+1].expire;
            chatlines[i].str=chatlines[i+1].str;
        }
        chatlines[CHAT_LINES-1].str=NULL;
    }
}

void addChatLine(char* str){
    int i=0;
    while(chatlines[i].str)
        if(++i>=CHAT_LINES){
            chatlines[0].expire=0;
            delChatLine();
            i=CHAT_LINES-1;
            break;
        }
    chatlines[i].str=str;
    chatlines[i].expire=MOAG_GetTicks()+CHAT_EXPIRETIME;
}

void draw() {
    redraw();
    for(int i=0;i<MAX_CLIENTS;i++)
        if(tanks[i].active)
            redrawLand(tanks[i].lastx-9,tanks[i].lasty-13,18,14);
    for(int i=0;i<MAX_CLIENTS;i++)
        if(tanks[i].active){
            drawTank(tanks[i].x-9,tanks[i].y-13,tanks[i].angle,tanks[i].facingLeft);
            MOAG_SetStringCentered(tanks[i].x,tanks[i].y-36,tanks[i].name,240,240,240);
            tanks[i].lastx=tanks[i].x;
            tanks[i].lasty=tanks[i].y;
        }
    drawBullets();
    delChatLine();
    for(int i=0;i<CHAT_LINES;i++)
        if(chatlines[i].str)
            MOAG_SetString(4,4+12*i,chatlines[i].str,255,255,255);
    if(typingStr){
        MOAG_SetBlock(6,8+12*(CHAT_LINES),4,4,210,210,210);
        pushRedraw(6,8+12*(CHAT_LINES),4,4);
        MOAG_SetString(16,4+12*(CHAT_LINES),typingStr,210,210,210);
    }
}



void initClient() {
    redraw();
    for(int i=0;i<CHAT_LINES;i++)
        chatlines[i].str=NULL;
    for(int i=0;i<MAX_CLIENTS;i++){
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

void update() {
    MOAG_ClientTick();
    MOAG_GrabEvents();
    if(MOAG_IsKeyPressed(SDLK_ESCAPE) || MOAG_IsQuitting())
        MOAG_QuitMainLoop();

    if(typingStr && !typingDone){
        if(MOAG_IsKeyPressed(SDLK_LEFT)
            || MOAG_IsKeyPressed(SDLK_RIGHT)
            || MOAG_IsKeyPressed(SDLK_UP)
            || MOAG_IsKeyPressed(SDLK_DOWN)){
            typingStr=NULL;
            MOAG_StopTextInput();
            return;
        }
        if(MOAG_IsKeyPressed(SDLK_RETURN)){
            typingDone=true;
            MOAG_StopTextInput();
            return;
        }
        return;
    }
    
    if(MOAG_IsKeyPressed('t')){
        typingStr=MOAG_StartTextInput();
        return;
    }
    if(MOAG_IsKeyPressed('/')){
        typingStr=MOAG_StartTextCmdInput();
        return;
    }

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
}



void validate_str(char* s){
    while(*s){
        if(*s<20 || *s>126)
            *s='?';
        s++;
    }
}

void client_update(MOAG_Connection arg)
{
    while (MOAG_HasActivity(arg, 0)){
        if (MOAG_ReceiveChunk(arg, 1) == -1){
            printf("Disconnected from server!\n");
            exit(0);
        }
        
        char byte = MOAG_ChunkDequeue8();
        
        switch(byte) {
        case LAND_CHUNK: {
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
            MOAG_ReceiveRaw(arg, buf, 8);
            int x = MOAG_Unpack16(&buf[0]);
            int y = MOAG_Unpack16(&buf[2]);
            int w = MOAG_Unpack16(&buf[4]);
            int h = MOAG_Unpack16(&buf[6]);
            if(w<0) w=0;
            if(h<0) h=0;
            if(x<0 || y<0 || x+w>WIDTH || y+h>HEIGHT)
                break;
            for(int i=0;i<h;i++)
                MOAG_ReceiveRaw(arg, &land[(y+i)*WIDTH+x], w);
            redrawLand(x,y,w,h);
        } break;
        case TANK_CHUNK: {
            MOAG_ReceiveChunk(arg, 6);
            int id = MOAG_ChunkDequeue8();
            short x = (short)MOAG_ChunkDequeue16();
            short y = (short)MOAG_ChunkDequeue16();
            char angle = MOAG_ChunkDequeue8();
            char facingLeft=0;
            
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
        case BULLET_CHUNK: {
            undrawBullets();
            
            MOAG_ReceiveChunk(arg, 2);
            numBullets = MOAG_ChunkDequeue16();
            
            if(numBullets<=0)
                break;
            if(numBullets>=MAX_BULLETS){ // error!
                numBullets=0;
                break;
            }
            
            MOAG_ReceiveChunk(arg, numBullets*4);
            for (int i=0;i<numBullets;i++) {
                bullets[i].x = MOAG_ChunkDequeue16();
                bullets[i].y = MOAG_ChunkDequeue16();
            }
        } break;
        case MSG_CHUNK: {
            MOAG_ReceiveChunk(arg, 3);
            int id = MOAG_ChunkDequeue8();
            char cmd = MOAG_ChunkDequeue8();
            unsigned char len = MOAG_ChunkDequeue8();
            switch(cmd){
            case 1: { //chat message
                int namelen=strlen(tanks[id].name);
                int linelen=namelen+len+4;
                char* line=new char[linelen];
                line[0]='<';
                for(int i=0;i<namelen;i++)
                    line[i+1]=tanks[id].name[i];
                line[namelen+1]='>';
                line[namelen+2]=' ';
                MOAG_ReceiveRaw(arg, &line[namelen+3], len);
                line[linelen-1]='\0';
                validate_str(line);
                addChatLine(line);
            } break;
            case 2: { //name change
                if(len<1 || len>15){ // error!
                    break;
                }
                MOAG_ReceiveRaw(arg, tanks[id].name, len);
                tanks[id].name[len]='\0';
                validate_str(tanks[id].name);
            } break;
            default:
                break;
            }
        } break;
        default:
            printf("unknown byte: %d\n",byte);
            break;
        }
    }

    if(MOAG_SendChunk(arg, -1, 1) == -1) {
        printf("Disconnected from server!\n");
        exit(0);
    }

    if(typingDone){
        unsigned char len=strlen(typingStr);
        char buf[2]={11,len};
        MOAG_SendRaw(arg, buf, 2);
        MOAG_SendRaw(arg, typingStr, len);
        typingDone=false;
        typingStr=NULL;
    }

    fflush(stdout);
}

int main(int argc, char *argv[])
{
    if (argc<2) {
        printf("usage:  %s [address] {[port]}\n", argv[0]);
        exit(0);
    }

    int port=8080;

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

