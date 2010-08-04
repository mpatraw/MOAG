#include "moag.h"
#include <unistd.h>
#include <math.h>

#include "tank.h"
#include "bullet.h"

#define MAX_CLIENTS 8
#define MAX_BULLETS 256
#define GRAVITY 0.1

void disconnect_client(int);

MOAG_Connection clients[MAX_CLIENTS] = {NULL};
int numClients = 0;

static char land[800*600];
static struct Tank tanks[MAX_CLIENTS];
static struct Bullet bullets[MAX_BULLETS];

inline char landAt(int x, int y){
    if(x<0 || x>=800 || y<0 || y>=600)
        return -1;
    return land[y*800+x];
}

void spawnTank(int id){
    tanks[id].x=400;
    tanks[id].y=80;
    tanks[id].angle=35;
    tanks[id].power=0;
    tanks[id].active=1;
    tanks[id].facingLeft=0;
    tanks[id].kLeft=0;
    tanks[id].kRight=0;
    tanks[id].kUp=0;
    tanks[id].kDown=0;
    tanks[id].kFire=0;
}

void sendLand(int to, int x, int y, int w, int h){
    char* buf;
    int len;
    int i;
    if(to<-1 || to>=MAX_CLIENTS)
        return;
    if(x<0){ w+=x; x=0; }
    if(y<0){ h+=y; y=0; }
    if(x+w>800) w=800-x;
    if(y+h>800) h=600-y;
    if(w<=0 || h<=0 || x+w>800 || y+h>600)
        return;        

    len=9+w*h;
    buf=malloc(len);
    buf[0]=1;
    MOAG_Pack16((short)x,&buf[1]);
    MOAG_Pack16((short)y,&buf[3]);
    MOAG_Pack16((short)w,&buf[5]);
    MOAG_Pack16((short)h,&buf[7]);
    for(i=0;i<h;i++)
        memcpy(&buf[9+i*w],&land[(y+i)*800+x],w);

    if(to!=-1){
        if(clients[to] && MOAG_SendRaw(clients[to], buf, len)==-1)
            disconnect_client(to);
    }else{
        for(i=0;i<MAX_CLIENTS;i++)
            if(clients[i] && MOAG_SendRaw(clients[i], buf, len)==-1)
                disconnect_client(i);
    }
    free(buf);
}

void sendTank(int to, int id){
    char buf[7];
    int i;
    if(to<-1 || to>=MAX_CLIENTS || (to>=0 && !clients[to]) || id<0 || id>=MAX_CLIENTS)
        return;

    buf[0]=2;
    buf[1]=(char)id;
    if(!tanks[id].active){
        tanks[id].x=-1;
        tanks[id].y=-1;
    }
    MOAG_Pack16((short)tanks[id].x,&buf[2]);
    MOAG_Pack16((short)tanks[id].y,&buf[4]);
    buf[6]=(char)tanks[id].angle;
    if(tanks[id].facingLeft)
        buf[6]=-buf[6];

    if(to!=-1){
        if(MOAG_SendRaw(clients[to], buf, 7)==-1)
            disconnect_client(to);
    }else{
        for(i=0;i<MAX_CLIENTS;i++)
            if(clients[i] && MOAG_SendRaw(clients[i], buf, 7)==-1)
                disconnect_client(i);
    }
}

void sendBullets(){
    char buf[MAX_BULLETS*4+3];
    int len;
    int i;
    int count=0;

    buf[0]=3;
    len=3;
    for(i=0;i<MAX_BULLETS;i++)
        if(bullets[i].active){
            MOAG_Pack16((short)bullets[i].x,&buf[len]);
            len+=2;
            MOAG_Pack16((short)bullets[i].y,&buf[len]);
            len+=2;
            count++;
        }
    MOAG_Pack16((short)count,&buf[1]);

    for(i=0;i<MAX_CLIENTS;i++)
        if(clients[i] && MOAG_SendRaw(clients[i], buf, len)==-1)
            disconnect_client(i);
}

void spawnClient(int id){
    int i;
    spawnTank(id);
    sendLand(id,0,0,800,600);
    for(i=0;i<MAX_CLIENTS;i++)
        sendTank(id,i);
}

void disconnect_client(int c){
    printf("Client DCed\n");
    MOAG_Disconnect(clients[c]);
    numClients -= 1;
    clients[c] = NULL;
    tanks[c].active=0;
    sendTank(-1,c);
}



void fireBullet(int x, int y, float angle, float vel){
    int i;
    for(i=0;i<MAX_BULLETS;i++)
        if(!bullets[i].active){
            bullets[i].active=1;
            bullets[i].x=x;
            bullets[i].y=y;
            bullets[i].fx=(float)x;
            bullets[i].fy=(float)y;
            bullets[i].vx=vel*cosf(angle*M_PI/180.0);
            bullets[i].vy=-vel*sinf(angle*M_PI/180.0);
            break;
        }
}

inline int sqr(int n){ return n*n; }

void explode(int x, int y, int rad){
    int ix,iy,i;
    for(iy=-rad;iy<=rad;iy++)
    for(ix=-rad;ix<=rad;ix++)
        if(landAt(x+ix,y+iy)==1 && ix*ix+iy*iy<rad*rad)
            land[(y+iy)*800+x+ix]=0;
    for(i=0;i<MAX_CLIENTS;i++)
        if(tanks[i].active && sqr(tanks[i].x-x)+sqr(tanks[i].y-y)<rad*rad)
            spawnTank(i);
    sendLand(-1,x-rad,y-rad,rad*2,rad*2);
}

void tankUpdate(int id){
    struct Tank *t=&tanks[id];
    if(!t->active)
        return;
    /* Movement. */
    if(t->kLeft){
        int i;
        t->facingLeft=1;
        for(i=0;i<8;i++)
            if(landAt(t->x-1,t->y-i)==0){
                if((t->x--)<=10)
                    t->x=10;
                t->y-=i;
                break;
            }
    }else if(t->kRight){
        int i;
        t->facingLeft=0;
        for(i=0;i<8;i++)
            if(landAt(t->x+1,t->y-i)==0){
                if((t->x++)>=800-10)
                    t->x=800-10;
                t->y-=i;
                break;
            }
    }
    /* Aim. */
    if(t->kUp && t->angle<90)
        t->angle++;
    else if(t->kDown && t->angle>1)
        t->angle--;
    /* Fire */
    if(t->kFire){
        if(t->power<1000)
            t->power+=10;
    }else if(t->power){
        fireBullet(t->x,t->y-7,t->facingLeft?180-t->angle:t->angle, (float)t->power*0.01);
        t->power=0;
    }
    /* Physics. */
    if(landAt(t->x,t->y+1)==0)
        t->y++;
    if(landAt(t->x,t->y+1)==0)
        t->y++;
    sendTank(-1,id);
}

inline void bulletUpdate(int b){
    if(!bullets[b].active)
        return;
    bullets[b].fx+=bullets[b].vx;
    bullets[b].fy+=bullets[b].vy;
    bullets[b].vy+=GRAVITY;
    bullets[b].x=(int)bullets[b].fx;
    bullets[b].y=(int)bullets[b].fy;
    if(landAt(bullets[b].x,bullets[b].y)){
        explode(bullets[b].x,bullets[b].y, 12);
        bullets[b].active=0;
    }
}

void initGame(){
    int i;
    for(i=0;i<MAX_CLIENTS;i++)
        tanks[i].active=0;
    for(i=0;i<MAX_BULLETS;i++)
        bullets[i].active=0;
    for(i=0;i<800*600/2;i++)
        land[i]=0;
    for(;i<800*600;i++)
        land[i]=1;
    sendLand(-1,0,0,800,600);
}

void stepGame(){
    int i;
    for(i=0;i<MAX_CLIENTS;i++)
        tankUpdate(i);
    for(i=0;i<MAX_BULLETS;i++)
        bulletUpdate(i);
    sendBullets();
}



void client_connect(void *arg)
{
    int i;
    
    for (i = 0; i < MAX_CLIENTS; i++)
        if (!clients[i]) {
            clients[i] = arg;
            numClients += 1;
            printf("Client connected!\n");
            spawnClient(i);
            break;
        }
    
    /* Too many clients. */
    if (i == MAX_CLIENTS) {
        printf("Client failed to connect, too many clients.\n");
        MOAG_Disconnect(arg);
        return;
    }
    
    fflush(stdout);
}

void server_update(void *arg)
{
    char byte;
    /*int timeout = 200 / (numClients ? numClients : 1);*/
    int i;
    
    stepGame();

    /* For each client... */
    for (i = 0; i < MAX_CLIENTS; ++i)
        while (clients[i] && MOAG_HasActivity(clients[i], 0)){
            /* Get data. */
            if (MOAG_ReceiveRaw(clients[i], &byte, 1) == -1){
                disconnect_client(i);
                continue;
            }

            /* printf("client %d sent %d\n",i,byte); */
            switch(byte){
            case 1: tanks[i].kLeft=1; break;
            case 2: tanks[i].kLeft=0; break;
            case 3: tanks[i].kRight=1; break;
            case 4: tanks[i].kRight=0; break;
            case 5: tanks[i].kUp=1; break;
            case 6: tanks[i].kUp=0; break;
            case 7: tanks[i].kDown=1; break;
            case 8: tanks[i].kDown=0; break;
            case 9: tanks[i].kFire=1; break;
            case 10: tanks[i].kFire=0; break;
            default: break;
            }
        }

    if (numClients == 0)
        usleep(200000);
    else
        usleep(20000);

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
    
    initGame();
    printf("Started server\n");

    while (1)
        MOAG_ServerTick();
    
    MOAG_CloseServer();

    return 0;
}

