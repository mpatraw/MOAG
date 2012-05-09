
#include <time.h>
#include <unistd.h>

#include "server.h"

#define SQ(x) ((x) * (x))

static char land[LAND_WIDTH * LAND_HEIGHT] = {0};
struct tank tanks[MAX_CLIENTS];
struct bullet bullets[MAX_BULLETS];
struct crate crate;
int spawns;

void kill_tank(int id)
{
    tanks[id].x=-30;
    tanks[id].y=-30;
    tanks[id].spawntimer=RESPAWN_TIME;
    broadcast_tank_chunk(tanks,id);
}

void explode(int x, int y, int rad, char type)
{
    // type: 0=explode, 1=dirt, 2=clear dirt, 3=collapse
    if(type==3){
        // collapse
        for(int iy=-rad;iy<=rad;iy++)
        for(int ix=-rad;ix<=rad;ix++)
            if(ix*ix+iy*iy<rad*rad && get_land_at(land, x+ix,y+iy))
                set_land_at(land, x+ix,y+iy,2);
        int maxy=y+rad;
        for(int iy=-rad;iy<=rad;iy++)
        for(int ix=-rad;ix<=rad;ix++)
            if(get_land_at(land, x+ix,y+iy)==2){
                int count=0;
                int yy;
                for(yy=y+iy;yy<LAND_HEIGHT && get_land_at(land, x+ix,yy)!=1;yy++)
                    if(get_land_at(land, x+ix,yy)==2){
                        count++;
                        set_land_at(land, x+ix,yy,0);
                    }
                for(;count>0;count--)
                    set_land_at(land, x+ix,yy-count,1);
                if(yy>maxy)
                    maxy=yy;
            }
        broadcast_land_chunk(land, x-rad,y-rad,rad*2,maxy-(y-rad));
        return;
    }
    char p=type==1?1:0;
    for(int iy=-rad;iy<=rad;iy++)
    for(int ix=-rad;ix<=rad;ix++)
        if(ix*ix+iy*iy<rad*rad)
            set_land_at(land, x+ix,y+iy, p);
    if(type==0)
        for(int i=0;i<MAX_CLIENTS;i++)
            if(tanks[i].active && SQ(tanks[i].x-x)+SQ(tanks[i].y-3-y)<SQ(rad+4))
                kill_tank(i);

    broadcast_land_chunk(land, x-rad,y-rad,rad*2,rad*2);
}

void spawn_tank(int id)
{
    spawns++;
    tanks[id].active=1;
    tanks[id].spawntimer=0;
    tanks[id].x=(spawns*240)%(LAND_WIDTH-40)+20;
    tanks[id].y=60;
    tanks[id].angle=30;
    tanks[id].facingLeft=0;
    tanks[id].power=0;
    tanks[id].bullet=1;
    tanks[id].ladder=LADDER_TIME;
    tanks[id].kleft=0;
    tanks[id].kright=0;
    tanks[id].kup=0;
    tanks[id].kdown=0;
    tanks[id].kfire=0;
    explode(tanks[id].x,tanks[id].y-12, 12, 2);
}

void spawn_client(int id)
{
    sprintf(tanks[id].name,"p%d",id);
    char notice[64]="  ";
    strcat(notice,tanks[id].name);
    strcat(notice," has connected");
    broadcast_chat(-1,SERVER_NOTICE,notice,strlen(notice));

    spawn_tank(id);
    tanks[id].angle=35;
    tanks[id].facingLeft=0;
    broadcast_land_chunk(land, 0,0,LAND_WIDTH,LAND_HEIGHT);
    broadcast_chat(id,NAME_CHANGE,tanks[id].name,strlen(tanks[id].name));
}

void disconnect_client(int c)
{
    printf("Client DCed\n");
    tanks[c].active=0;
    broadcast_tank_chunk(tanks,c);
}

void launch_ladder(int x, int y)
{
    int i=0;
    while(bullets[i].active)
        if(++i>=MAX_BULLETS)
            return;
    bullets[i].active=LADDER_LENGTH;
    bullets[i].type=10;
    bullets[i].x=x;
    bullets[i].y=y;
    bullets[i].fx=x;
    bullets[i].fy=y;
    bullets[i].vx=0;
    bullets[i].vy=-1;
}

void fire_bullet(char type, float x, float y, float vx, float vy)
{
    int i=0;
    while(bullets[i].active)
        if(++i>=MAX_BULLETS)
            return;
    bullets[i].active=4;
    bullets[i].type=type;
    bullets[i].fx=x;
    bullets[i].fy=y;
    bullets[i].x=(int)bullets[i].fx;
    bullets[i].y=(int)bullets[i].fy;
    bullets[i].vx=vx;
    bullets[i].vy=vy;
}

void fire_bullet_ang(char type, int x, int y, float angle, float vel)
{
    fire_bullet(type,(float)x+5.0*cosf(angle*M_PI/180),
                    (float)y-5.0*sinf(angle*M_PI/180),
                    vel*cosf(angle*M_PI/180.0),
                    -vel*sinf(angle*M_PI/180.0));
}

void liquid(int x, int y, int n)
{
    if(x<0) x=0;
    if(y<0) y=0;
    if(x>=LAND_WIDTH) x=LAND_WIDTH-1;
    if(y>=LAND_HEIGHT) y=LAND_HEIGHT-1;
    int minx=LAND_WIDTH;
    int miny=LAND_HEIGHT;
    int maxx=0;
    int maxy=0;
    for(int i=0;i<n;i++){
        if(y<0) break;
        int nx=x;
        if(get_land_at(land, x,y+1)==0) y++;
        else if(get_land_at(land, x-1,y)==0) x--;
        else if(get_land_at(land, x+1,y)==0) x++;
        else
            for(int i=x;i<LAND_WIDTH+1;i++){
                if(nx==x && get_land_at(land, i,y-1)==0)
                    nx=i;
                if(get_land_at(land, i,y)==0){
                    x=i; break;
                }else if(get_land_at(land, i,y)!=3){
                    for(int i=x;i>=-1;i--){
                        if(nx==x && get_land_at(land, i,y-1)==0)
                            nx=i;
                        if(get_land_at(land, i,y)==0){
                            x=i; break;
                        }else if(get_land_at(land, i,y)!=3){
                            y--; x=nx; break;
                        }
                    } break;
                }
            }
        set_land_at(land, x,y,3);
        if(x<minx) minx=x;
        else if(x>maxx) maxx=x;
        if(y<miny) miny=y;
        else if(y>maxy) maxy=y;
    }
    for(int iy=miny;iy<=maxy;iy++)
    for(int ix=minx;ix<=maxx;ix++)
        if(get_land_at(land, ix,iy)==3)
            set_land_at(land, ix,iy,1);
    broadcast_land_chunk(land,minx,miny,maxx-minx+1,maxy-miny+1);
}

void tank_update(int id){
    if(!tanks[id].active)
        return;
    if(tanks[id].spawntimer>0){
        if(--tanks[id].spawntimer<=0)
            spawn_tank(id);
        else
            return;
    }
    bool grav=true;
    if(tanks[id].ladder>=0 && (!tanks[id].kleft || !tanks[id].kright))
        tanks[id].ladder=LADDER_TIME;
    if(tanks[id].kleft && tanks[id].kright){
        if(tanks[id].ladder>0){
            if(get_land_at(land, tanks[id].x,tanks[id].y+1))
                tanks[id].ladder--;
            else
                tanks[id].ladder=LADDER_TIME;
        }else if(tanks[id].ladder==0){
            launch_ladder(tanks[id].x,tanks[id].y);
            tanks[id].ladder=-1;
        }
    }else if(tanks[id].kleft){
        tanks[id].facingLeft=1;
        if(get_land_at(land, tanks[id].x-1,tanks[id].y)==0 && tanks[id].x>=10){
            tanks[id].x--;
        }else if(get_land_at(land, tanks[id].x-1,tanks[id].y-1)==0 && tanks[id].x>=10){
            tanks[id].x--; tanks[id].y--;
        }else if(get_land_at(land, tanks[id].x,tanks[id].y-1)==0 || get_land_at(land, tanks[id].x,tanks[id].y-2)==0 || get_land_at(land, tanks[id].x,tanks[id].y-3)==0){
            grav=false; tanks[id].y--;
        }else
            grav=false;
    }else if(tanks[id].kright){
        tanks[id].facingLeft=0;
        if(get_land_at(land, tanks[id].x+1,tanks[id].y)==0 && tanks[id].x<LAND_WIDTH-10){
            tanks[id].x++;
        }else if(get_land_at(land, tanks[id].x+1,tanks[id].y-1)==0 && tanks[id].x<LAND_WIDTH-10){
            tanks[id].x++; tanks[id].y--;
        }else if(get_land_at(land, tanks[id].x,tanks[id].y-1)==0 || get_land_at(land, tanks[id].x,tanks[id].y-2)==0 || get_land_at(land, tanks[id].x,tanks[id].y-3)==0){
            grav=false; tanks[id].y--;
        }else
            grav=false;
    }
    // Physics
    if(tanks[id].y<20)
        tanks[id].y=20;
    if(grav){
        if(get_land_at(land, tanks[id].x,tanks[id].y+1)==0)
            tanks[id].y++;
        if(get_land_at(land, tanks[id].x,tanks[id].y+1)==0)
            tanks[id].y++;
    }
    // Pickup
    if(abs(tanks[id].x-crate.x)<14 && abs(tanks[id].y-crate.y)<14){
        tanks[id].ladder=LADDER_TIME;
        tanks[id].bullet=crate.type;
        crate.x=0;
        crate.y=0;
        char notice[64]="* ";
        strcat(notice,tanks[id].name);
        strcat(notice," got ");
        switch(tanks[id].bullet){
        case  1: strcat(notice,"Missile"); break;
        case  2: strcat(notice,"Baby Nuke"); break;
        case  3: strcat(notice,"Nuke"); break;
        case  4: strcat(notice,"Dirtball"); break;
        case  5: strcat(notice,"Dirtball"); break;
        case  6: strcat(notice,"Collapse"); break;
        case  7: strcat(notice,"Liquid Dirt"); break;
        case  8: strcat(notice,"Bouncer"); break;
        case  9: strcat(notice,"Tunneler"); break;
        case 11: strcat(notice,"MIRV"); break;
        case 13: strcat(notice,"Cluster Bomb"); break;
        default: strcat(notice,"???"); break;
        }
        broadcast_chat(-1,SERVER_NOTICE,notice,strlen(notice));
    }
    // Aim
    if(tanks[id].kup && tanks[id].angle<90)
        tanks[id].angle++;
    else if(tanks[id].kdown && tanks[id].angle>1)
        tanks[id].angle--;
    // Fire
    if(tanks[id].kfire){
        if(tanks[id].power<1000)
            tanks[id].power+=10;
    }else if(tanks[id].power){
        fire_bullet_ang(tanks[id].bullet, tanks[id].x, tanks[id].y-7,tanks[id].facingLeft?180-tanks[id].angle:tanks[id].angle, (float)tanks[id].power*0.01);
        tanks[id].bullet=1;
        tanks[id].power=0;
    }
    broadcast_tank_chunk(tanks, id);
}

void bounce_bullet(int b, float hitx, float hity)
{
    const int ix=(int)hitx;
    const int iy=(int)hity;
    if(get_land_at(land, ix,iy)==-1){
        bullets[b].vx=-bullets[b].vx;
        bullets[b].vy=-bullets[b].vy;
        return;
    }
    bullets[b].fx=hitx;
    bullets[b].fy=hity;
    bullets[b].x=ix;
    bullets[b].y=iy;
    unsigned char hit=0;
    if(get_land_at(land, ix-1,iy-1)) hit|=1<<7;
    if(get_land_at(land, ix  ,iy-1)) hit|=1<<6;
    if(get_land_at(land, ix+1,iy-1)) hit|=1<<5;
    if(get_land_at(land, ix-1,iy  )) hit|=1<<4;
    if(get_land_at(land, ix+1,iy  )) hit|=1<<3;
    if(get_land_at(land, ix-1,iy+1)) hit|=1<<2;
    if(get_land_at(land, ix  ,iy+1)) hit|=1<<1;
    if(get_land_at(land, ix+1,iy+1)) hit|=1;
    const float IRT2=0.70710678;
    const float vx=bullets[b].vx;
    const float vy=bullets[b].vy;
    switch(hit){
    case 0x00: break;
    case 0x07: case 0xe0: case 0x02: case 0x40:
        bullets[b].vy=-vy; break;
    case 0x94: case 0x29: case 0x10: case 0x08:
        bullets[b].vx=-vx; break;
    case 0x16: case 0x68: case 0x04: case 0x20:
        bullets[b].vy=vx; bullets[b].vx=vy; break;
    case 0xd0: case 0x0b: case 0x80: case 0x01:
        bullets[b].vy=-vx; bullets[b].vx=-vy; break;
    case 0x17: case 0xe8: case 0x06: case 0x60:
        bullets[b].vx=+vx*IRT2+vy*IRT2; bullets[b].vy=-vy*IRT2+vx*IRT2; break;
    case 0x96: case 0x69: case 0x14: case 0x28:
        bullets[b].vx=-vx*IRT2+vy*IRT2; bullets[b].vy=+vy*IRT2+vx*IRT2; break;
    case 0xf0: case 0x0f: case 0xc0: case 0x03:
        bullets[b].vx=+vx*IRT2-vy*IRT2; bullets[b].vy=-vy*IRT2-vx*IRT2; break;
    case 0xd4: case 0x2b: case 0x90: case 0x09:
        bullets[b].vx=-vx*IRT2-vy*IRT2; bullets[b].vy=+vy*IRT2-vx*IRT2; break;
    default:
        bullets[b].vx=-vx; bullets[b].vy=-vy; break;
    }
}

void bullet_detonate(int b)
{
    float d=sqrt(SQ(bullets[b].vx)+SQ(bullets[b].vy));
    if(d<0.001 && d>-0.001)
        d=d<0?-1:1;
    const float dx=bullets[b].vx/d;
    const float dy=bullets[b].vy/d;
    float hitx=bullets[b].fx;
    float hity=bullets[b].fy;
    for(int i=40; i>0 && get_land_at(land, (int)hitx,(int)hity); i--){
        hitx-=dx;
        hity-=dy;
    }
    switch(bullets[b].type){
    case 1: // missile
        explode(bullets[b].x,bullets[b].y, 12, 0);
        break;
    case 2: // baby nuke
        explode(bullets[b].x,bullets[b].y, 55, 0);
        break;
    case 3: // nuke
        explode(bullets[b].x,bullets[b].y, 150, 0);
        break;
    case 4: // dirt
        explode(bullets[b].x,bullets[b].y, 55, 1);
        break;
    case 5: // super dirt
        explode(bullets[b].x,bullets[b].y, 300, 1);
        break;
    case 6: // collapse
        explode(bullets[b].x,bullets[b].y, 120, 3);
        break;
    case 7: // liquid dirt
        liquid((int)hitx, (int)hity, 3000);
        break;
    case 8: { // bouncer
        if(bullets[b].active>0)
            bullets[b].active=-BOUNCER_BOUNCES;
        bullets[b].active++;
        bounce_bullet(b,hitx,hity);
        bullets[b].vx*=0.9;
        bullets[b].vy*=0.9;
        explode(bullets[b].x,bullets[b].y, 12, 0);
    } break;
    case 9: //tunneler
        if(bullets[b].active>0)
            bullets[b].active=-TUNNELER_TUNNELINGS;
        bullets[b].active++;
        explode(hitx,hity, 9, 0);
        explode(hitx+8*dx,hity+8*dy, 9, 0);
        break;
    case 10: { //ladder
        int x=bullets[b].x;
        int y=bullets[b].y;
        for(;y<LAND_HEIGHT;y++) if(get_land_at(land, x,y)==0) break;
        for(;y<LAND_HEIGHT;y++) if(get_land_at(land, x,y)) break;
        const int maxy=y+1;
        y=bullets[b].y;
        for(;y>0;y--) if(get_land_at(land, x,y)==0) break;
        const int miny=y;
        for(;y<maxy;y+=2){
            set_land_at(land, x-1,y,0);
            set_land_at(land, x  ,y,1);
            set_land_at(land, x+1,y,0);
            set_land_at(land, x-1,y+1,1);
            set_land_at(land, x  ,y+1,1);
            set_land_at(land, x+1,y+1,1);
        }
        broadcast_land_chunk(land, x-1,miny,3,maxy-miny+1);
    } break;
    case 11: //MIRV
        bounce_bullet(b,hitx,hity);
        explode(bullets[b].x,bullets[b].y, 12, 0);
        for(int i=-3;i<4;i++)
            fire_bullet(12,bullets[b].x,bullets[b].y,bullets[b].vx+i,bullets[b].vy);
        break;
    case 12: //MIRV warhead
        explode(bullets[b].x,bullets[b].y, 30, 0);
        break;
    case 13: //cluster bomb
        bounce_bullet(b,hitx,hity);
        explode(bullets[b].x,bullets[b].y, 20, 0);
        for(int i=0;i<11;i++)
            fire_bullet(1,hitx,hity, 1.5*cosf(i*M_PI/5.5)+0.25*bullets[b].vx,
                                    1.5*sinf(i*M_PI/5.5)+0.5*bullets[b].vy);
        break;
    default: break;
    }
    if(bullets[b].active>0)
        bullets[b].active=0;
}

void bullet_update(int b)
{
    if(!bullets[b].active)
        return;
    if(bullets[b].type==10){ //special (ladder)
        bullets[b].active--;
        bullets[b].fx+=bullets[b].vx;
        bullets[b].fy+=bullets[b].vy;
        bullets[b].x=(int)bullets[b].fx;
        bullets[b].y=(int)bullets[b].fy;
        if(get_land_at(land, bullets[b].x,bullets[b].y)==1){
            explode(bullets[b].x,bullets[b].y+LADDER_LENGTH-bullets[b].active, 1, 2);
            bullet_detonate(b);
        }
        return;
    }
    bullets[b].fx+=bullets[b].vx;
    bullets[b].fy+=bullets[b].vy;
    bullets[b].vy+=GRAVITY;
    bullets[b].x=(int)bullets[b].fx;
    bullets[b].y=(int)bullets[b].fy;
    if(get_land_at(land, bullets[b].x,bullets[b].y)){
        bullet_detonate(b);
        return;
    }
    if(bullets[b].active>1){
        bullets[b].active--;
        return;
    }
    if(bullets[b].type==8 && bullets[b].active==1)
        bullets[b].active=-BOUNCER_BOUNCES;
    if(bullets[b].type==9 && bullets[b].active==1)
        bullets[b].active=-TUNNELER_TUNNELINGS;
    for(int i=0;i<MAX_CLIENTS;i++)
        if(SQ(tanks[i].x-bullets[b].x)+SQ(tanks[i].y-3-bullets[b].y)<72){
            bullet_detonate(b);
            return;
        }
    if(SQ(crate.x-bullets[b].x)+SQ(crate.y-4-bullets[b].y)<30){
        bullet_detonate(b);
        fire_bullet(crate.type, crate.x, crate.y-4, crate.type==8?(bullets[b].vx<0?-0.2:0.2):0, -0.2);
        crate.x=0;
        crate.y=0;
        return;
    }
    if(bullets[b].type==11 && bullets[b].vy>0){
        bullet_detonate(b);
        return;
    }
}

void crate_update(void)
{
    if(crate.x==0 && crate.y==0){
        static int seed=99999;
        seed=seed*13841+time(NULL);
        crate.x=abs(seed)%(LAND_WIDTH-40)+20;
        crate.y=30;
        explode(crate.x,crate.y-12, 12, 2);

        const int PBABYNUKE=100;
        const int PNUKE=20;
        const int PDIRT=75;
        const int PSUPERDIRT=15;
        const int PLIQUIDDIRT=60;
        const int PCOLLAPSE=60;
        const int PBOUNCER=100;
        const int PTUNNELER=75;
        const int PMIRV=40;
        const int PCLUSTER=120;
        // add new ones here:
        const int TOTAL=PBABYNUKE+PNUKE+PDIRT+PSUPERDIRT+PLIQUIDDIRT+PCOLLAPSE
                        +PBOUNCER+PTUNNELER+PMIRV+PCLUSTER;
        int r=abs(seed*12347+11863)%TOTAL;
             if((r-=PBABYNUKE)<0)   crate.type=2;
        else if((r-=PNUKE)<0)       crate.type=3;
        else if((r-=PSUPERDIRT)<0)  crate.type=5;
        else if((r-=PLIQUIDDIRT)<0) crate.type=7;
        else if((r-=PCOLLAPSE)<0)   crate.type=6;
        else if((r-=PBOUNCER)<0)    crate.type=8;
        else if((r-=PTUNNELER)<0)   crate.type=9;
        else if((r-=PMIRV)<0)       crate.type=11;
        else if((r-=PCLUSTER)<0)    crate.type=13;
        else                        crate.type=4;
        broadcast_crate_chunk(crate);
    }
    if(get_land_at(land, crate.x,crate.y+1)==0) {
        crate.y++;
        broadcast_crate_chunk(crate);
    }
}

void step_game(void)
{
    crate_update();
    for(int i=0;i<MAX_CLIENTS;i++)
        tank_update(i);
    for(int i=0;i<MAX_BULLETS;i++)
        bullet_update(i);
    broadcast_bullets(bullets);
}

int client_connect(void)
{
    int i=0;
    while(tanks[i].active)
        if(++i>=MAX_CLIENTS) {
            printf("Client failed to connect, too many clients.\n");
            return -1;
        }

    printf("Client connected!\n");
    spawn_client(i);

    return i;
}

void handle_msg(int id, const char* msg, int len){
    if(msg[0]=='/' && msg[1]=='n' && msg[2]==' '){
        char notice[64]="  ";
        strcat(notice,tanks[id].name);
        len-=3;
        if(len>15)
            len=15;
        for(int i=0;i<len;i++)
            tanks[id].name[i]=msg[i+3];
        tanks[id].name[len]='\0';
        strcat(notice," is now known as ");
        strcat(notice,tanks[id].name);
        broadcast_chat(id,NAME_CHANGE,tanks[id].name,strlen(tanks[id].name));
        broadcast_chat(-1,SERVER_NOTICE,notice,strlen(notice));
        return;
    }
    broadcast_chat(id,CHAT,msg,len);
}

void init_game(void)
{
    spawns=0;
    for(int i=0;i<MAX_CLIENTS;i++)
        tanks[i].active=0;
    for(int i=0;i<MAX_BULLETS;i++)
        bullets[i].active=0;
    crate.x=0;
    crate.y=0;

    for (int y = 0; y < LAND_HEIGHT; ++y) {
        for (int x = 0; x < LAND_WIDTH; ++x){
            if (y < LAND_HEIGHT / 3)
                set_land_at(land, x, y, 0);
            else
                set_land_at(land, x, y, 1);
        }
    }
}

void on_receive(ENetEvent *ev)
{
    unsigned char *packet = ev->packet->data;
    size_t pos = 0;
    int i = (int)ev->peer->data;

    char byte = read8(packet, &pos);

    switch(byte){
    case KLEFT_PRESSED_CHUNK: tanks[i].kleft=1; break;
    case KLEFT_RELEASED_CHUNK: tanks[i].kleft=0; break;
    case KRIGHT_PRESSED_CHUNK: tanks[i].kright=1; break;
    case KRIGHT_RELEASED_CHUNK: tanks[i].kright=0; break;
    case KUP_PRESSED_CHUNK: tanks[i].kup=1; break;
    case KUP_RELEASED_CHUNK: tanks[i].kup=0; break;
    case KDOWN_PRESSED_CHUNK: tanks[i].kdown=1; break;
    case KDOWN_RELEASED_CHUNK: tanks[i].kdown=0; break;
    case KFIRE_PRESSED_CHUNK: tanks[i].kfire=1; break;
    case KFIRE_RELEASED_CHUNK: tanks[i].kfire=0; break;
    case CLIENT_MSG_CHUNK: { //msg
        unsigned char len = read8(packet, &pos);
        char* msg=malloc(len);
        for(int j=0;j<len;j++)
            msg[j] = read8(packet, &pos);
        handle_msg(i,msg,len);
        free(msg);
        break;
    }
    default: break;
    }
}

int main(int argc, char *argv[])
{
    init_enet_server(PORT);

    init_game();

    ENetEvent event;

    for (;;) {
        while (enet_host_service(get_server_host(), &event, 10)) {
            switch (event.type) {
            case ENET_EVENT_TYPE_CONNECT:
                event.peer->data = (void *)client_connect();
                break;

            case ENET_EVENT_TYPE_DISCONNECT:
                disconnect_client((int)event.peer->data);
                break;

            case ENET_EVENT_TYPE_RECEIVE:
                on_receive(&event);
                enet_packet_destroy(event.packet);
                break;

            default:
                break;
            }
        }

        struct timespec t;
        t.tv_sec = 0;
        t.tv_nsec = 10000000;
        while (nanosleep(&t, &t) == -1)
            continue;

        step_game();
    }

    uninit_enet();

    return EXIT_SUCCESS;
}
