#include "moag.h"
#include <unistd.h>
#include <math.h>
#include <string.h>

struct Redraw{
    int x,y,w,h;
    Redraw* next;
    Redraw(int _x, int _y, int _w, int _h) : x(_x),y(_y),w(_w),h(_h),next(NULL) {}
};

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
            moag::SetPixel(ix,iy,0,0,0);
        else
            moag::SetPixel(ix,iy,155,155,155);
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

void sendByte(char k){
    moag::ChunkEnqueue8(k);
}

void update() {
    moag::ClientTick();
    moag::GrabEvents();

    if(moag::IsQuitting() || (!typingStr && moag::IsKeyPressed(SDLK_ESCAPE))){
        moag::QuitMainLoop();
        return;
    }

    if(typingStr && !typingDone){
        if(moag::IsKeyPressed(SDLK_ESCAPE)
            || moag::IsKeyPressed(SDLK_LEFT)
            || moag::IsKeyPressed(SDLK_RIGHT)
            || moag::IsKeyPressed(SDLK_UP)
            || moag::IsKeyPressed(SDLK_DOWN)){
            typingStr=NULL;
            moag::StopTextInput();
            return;
        }
        if(moag::IsKeyPressed(SDLK_RETURN)){
            if(typingStr[0]=='\0'){
                typingStr=NULL;
                moag::StopTextInput();
                return;
            }
            typingDone=true;
            moag::StopTextInput();
            return;
        }
        return;
    }

    if(moag::IsKeyPressed('t')){
        typingStr=moag::StartTextInput();
        return;
    }
    if(moag::IsKeyPressed('/')){
        typingStr=moag::StartTextCmdInput();
        return;
    }

    if(moag::IsKeyPressed(SDLK_LEFT)){
        sendByte(1); keys.kLeft=1;
    }else if(moag::IsKeyReleased(SDLK_LEFT)){
        sendByte(2); keys.kLeft=0;
    }
    if(moag::IsKeyPressed(SDLK_RIGHT)){
        sendByte(3); keys.kRight=1;
    }else if(moag::IsKeyReleased(SDLK_RIGHT)){
        sendByte(4); keys.kRight=0;
    }
    if(moag::IsKeyPressed(SDLK_UP)){
        sendByte(5); keys.kUp=1;
    }else if(moag::IsKeyReleased(SDLK_UP)){
        sendByte(6); keys.kUp=0;
    }
    if(moag::IsKeyPressed(SDLK_DOWN)){
        sendByte(7); keys.kDown=1;
    }else if(moag::IsKeyReleased(SDLK_DOWN)){
        sendByte(8); keys.kDown=0;
    }
    if(moag::IsKeyPressed(SDLK_SPACE)){
        sendByte(9); keys.kFire=1;
    }else if(moag::IsKeyReleased(SDLK_SPACE)){
        sendByte(10); keys.kFire=0;
    }
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

    if (moag::OpenClient(argv[1], port) == -1) {
        printf("Failed to open client\n");
        return 1;
    }

    if (moag::OpenWindow(WIDTH, HEIGHT, "MOAG") == -1) {
        printf("Failed to start window\n");
        return 1;
    }

    if (moag::SetFont("Nouveau_IBM.ttf", 12) == -1) {
        printf("Failed to open font\n");
        return 1;
    }

    moag::SetClientCallback(client_update, moag::CB_CLIENT_UPDATE);
    moag::SetLoopCallback(draw, moag::CB_DRAW);
    moag::SetLoopCallback(update, moag::CB_UPDATE);

    initClient();

    moag::MainLoop();

    moag::CloseWindow();
    moag::CloseClient();
    return 0;
}

