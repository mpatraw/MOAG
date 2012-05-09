
#include <time.h>
#include <unistd.h>

#include "server.h"

#define SQ(x) ((x) * (x))

void kill_tank(struct moag *m, int id)
{
    m->tanks[id].x=-30;
    m->tanks[id].y=-30;
    m->tanks[id].spawntimer=RESPAWN_TIME;
    broadcast_tank_chunk(m, KILL, id);
}

void explode(struct moag *m, int x, int y, int rad, char type)
{
    // type: 0=explode, 1=dirt, 2=clear dirt, 3=collapse
    if(type==3){
        // collapse
        for(int iy=-rad;iy<=rad;iy++)
        for(int ix=-rad;ix<=rad;ix++)
            if(ix*ix+iy*iy<rad*rad && get_land_at(m, x+ix,y+iy))
                set_land_at(m, x+ix,y+iy,2);
        int maxy=y+rad;
        for(int iy=-rad;iy<=rad;iy++)
        for(int ix=-rad;ix<=rad;ix++)
            if(get_land_at(m, x+ix,y+iy)==2){
                int count=0;
                int yy;
                for(yy=y+iy;yy<LAND_HEIGHT && get_land_at(m, x+ix,yy)!=1;yy++)
                    if(get_land_at(m, x+ix,yy)==2){
                        count++;
                        set_land_at(m, x+ix,yy,0);
                    }
                for(;count>0;count--)
                    set_land_at(m, x+ix,yy-count,1);
                if(yy>maxy)
                    maxy=yy;
            }
        broadcast_land_chunk(m, x-rad,y-rad,rad*2,maxy-(y-rad));
        return;
    }
    char p=type==1?1:0;
    for(int iy=-rad;iy<=rad;iy++)
    for(int ix=-rad;ix<=rad;ix++)
        if(ix*ix+iy*iy<rad*rad)
            set_land_at(m, x+ix,y+iy, p);
    if(type==0)
        for(int i=0;i<MAX_CLIENTS;i++)
            if(m->tanks[i].active && SQ(m->tanks[i].x-x)+SQ(m->tanks[i].y-3-y)<SQ(rad+4))
                kill_tank(m, i);

    broadcast_land_chunk(m, x-rad,y-rad,rad*2,rad*2);
}

void spawn_tank(struct moag *m, int id)
{
    m->tanks[id].active=1;
    m->tanks[id].spawntimer=0;
    m->tanks[id].x= rng_range(&m->rng, 20, LAND_WIDTH - 20);
    m->tanks[id].y=60;
    m->tanks[id].angle=30;
    m->tanks[id].facingleft=0;
    m->tanks[id].power=0;
    m->tanks[id].bullet=1;
    m->tanks[id].ladder=LADDER_TIME;
    m->tanks[id].kleft=0;
    m->tanks[id].kright=0;
    m->tanks[id].kup=0;
    m->tanks[id].kdown=0;
    m->tanks[id].kfire=0;
    explode(m, m->tanks[id].x,m->tanks[id].y-12, 12, 2);
    broadcast_tank_chunk(m, SPAWN, id);
}

void spawn_client(struct moag *m, int id)
{
    sprintf(m->tanks[id].name,"p%d",id);
    char notice[64]="  ";
    strcat(notice,m->tanks[id].name);
    strcat(notice," has connected");
    broadcast_chat(-1,SERVER_NOTICE,notice,strlen(notice));

    spawn_tank(m, id);
    m->tanks[id].angle=35;
    m->tanks[id].facingleft=0;
    broadcast_land_chunk(m, 0,0,LAND_WIDTH,LAND_HEIGHT);
    broadcast_chat(id,NAME_CHANGE,m->tanks[id].name,strlen(m->tanks[id].name));
}

void disconnect_client(struct moag *m, int id)
{
    printf("Client DCed\n");
    m->tanks[id].active = 0;
    broadcast_tank_chunk(m, KILL, id);
}

void launch_ladder(struct moag *m, int x, int y)
{
    int i=0;
    while(m->bullets[i].active)
        if(++i>=MAX_BULLETS)
            return;
    m->bullets[i].active=LADDER_LENGTH;
    m->bullets[i].type=10;
    m->bullets[i].x=x;
    m->bullets[i].y=y;
    m->bullets[i].fx=x;
    m->bullets[i].fy=y;
    m->bullets[i].vx=0;
    m->bullets[i].vy=-1;
    broadcast_bullet_chunk(m, SPAWN, i);
}

void fire_bullet(struct moag *m, char type, float x, float y, float vx, float vy)
{
    int i=0;
    while(m->bullets[i].active)
        if(++i>=MAX_BULLETS)
            return;
    m->bullets[i].active=4;
    m->bullets[i].type=type;
    m->bullets[i].fx=x;
    m->bullets[i].fy=y;
    m->bullets[i].x=(int)m->bullets[i].fx;
    m->bullets[i].y=(int)m->bullets[i].fy;
    m->bullets[i].vx=vx;
    m->bullets[i].vy=vy;
    broadcast_bullet_chunk(m, SPAWN, i);
}

void fire_bullet_ang(struct moag *m, char type, int x, int y, float angle, float vel)
{
    fire_bullet(m, type,(float)x+5.0*cosf(angle*M_PI/180),
                    (float)y-5.0*sinf(angle*M_PI/180),
                    vel*cosf(angle*M_PI/180.0),
                    -vel*sinf(angle*M_PI/180.0));
}

void liquid(struct moag *m, int x, int y, int n)
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
        if(get_land_at(m, x,y+1)==0) y++;
        else if(get_land_at(m, x-1,y)==0) x--;
        else if(get_land_at(m, x+1,y)==0) x++;
        else
            for(int i=x;i<LAND_WIDTH+1;i++){
                if(nx==x && get_land_at(m, i,y-1)==0)
                    nx=i;
                if(get_land_at(m, i,y)==0){
                    x=i; break;
                }else if(get_land_at(m, i,y)!=3){
                    for(int i=x;i>=-1;i--){
                        if(nx==x && get_land_at(m, i,y-1)==0)
                            nx=i;
                        if(get_land_at(m, i,y)==0){
                            x=i; break;
                        }else if(get_land_at(m, i,y)!=3){
                            y--; x=nx; break;
                        }
                    } break;
                }
            }
        set_land_at(m, x,y,3);
        if(x<minx) minx=x;
        else if(x>maxx) maxx=x;
        if(y<miny) miny=y;
        else if(y>maxy) maxy=y;
    }
    for(int iy=miny;iy<=maxy;iy++)
    for(int ix=minx;ix<=maxx;ix++)
        if(get_land_at(m, ix,iy)==3)
            set_land_at(m, ix,iy,1);
    broadcast_land_chunk(m,minx,miny,maxx-minx+1,maxy-miny+1);
}

void tank_update(struct moag *m, int id)
{
    bool moved = false;
    if(!m->tanks[id].active)
        return;
    if (m->tanks[id].spawntimer > 0){
        if (--m->tanks[id].spawntimer <= 0)
            spawn_tank(m, id);
        return;
    }
    bool grav=true;
    if(m->tanks[id].ladder>=0 && (!m->tanks[id].kleft || !m->tanks[id].kright))
        m->tanks[id].ladder=LADDER_TIME;
    if(m->tanks[id].kleft && m->tanks[id].kright){
        if(m->tanks[id].ladder>0){
            if(get_land_at(m, m->tanks[id].x,m->tanks[id].y+1))
                m->tanks[id].ladder--;
            else
                m->tanks[id].ladder=LADDER_TIME;
        }else if(m->tanks[id].ladder==0){
            launch_ladder(m, m->tanks[id].x,m->tanks[id].y);
            m->tanks[id].ladder=-1;
        }
    }else if(m->tanks[id].kleft){
        m->tanks[id].facingleft=1;
        if(get_land_at(m, m->tanks[id].x-1,m->tanks[id].y)==0 && m->tanks[id].x>=10){
            m->tanks[id].x--;
        }else if(get_land_at(m, m->tanks[id].x-1,m->tanks[id].y-1)==0 && m->tanks[id].x>=10){
            m->tanks[id].x--; m->tanks[id].y--;
        }else if(get_land_at(m, m->tanks[id].x,m->tanks[id].y-1)==0 || get_land_at(m, m->tanks[id].x,m->tanks[id].y-2)==0 || get_land_at(m, m->tanks[id].x,m->tanks[id].y-3)==0){
            grav=false; m->tanks[id].y--;
        }else
            grav=false;
        moved = true;
    }else if(m->tanks[id].kright){
        m->tanks[id].facingleft=0;
        if(get_land_at(m, m->tanks[id].x+1,m->tanks[id].y)==0 && m->tanks[id].x<LAND_WIDTH-10){
            m->tanks[id].x++;
        }else if(get_land_at(m, m->tanks[id].x+1,m->tanks[id].y-1)==0 && m->tanks[id].x<LAND_WIDTH-10){
            m->tanks[id].x++; m->tanks[id].y--;
        }else if(get_land_at(m, m->tanks[id].x,m->tanks[id].y-1)==0 || get_land_at(m, m->tanks[id].x,m->tanks[id].y-2)==0 || get_land_at(m, m->tanks[id].x,m->tanks[id].y-3)==0){
            grav=false; m->tanks[id].y--;
        }else
            grav=false;
        moved = true;
    }
    // Physics
    if(m->tanks[id].y<20) {
        m->tanks[id].y=20;
        moved = true;
    }
    if(grav){
        if(get_land_at(m, m->tanks[id].x,m->tanks[id].y+1)==0) {
            m->tanks[id].y++;
            moved = true;
        }
        if(get_land_at(m, m->tanks[id].x,m->tanks[id].y+1)==0) {
            m->tanks[id].y++;
            moved = true;
        }
    }
    // Pickup
    if(abs(m->tanks[id].x-m->crate.x)<14 && abs(m->tanks[id].y-m->crate.y)<14){
        m->tanks[id].ladder=LADDER_TIME;
        m->tanks[id].bullet=m->crate.type;
        m->crate.active = false;
        broadcast_crate_chunk(m, KILL);
        char notice[64]="* ";
        strcat(notice,m->tanks[id].name);
        strcat(notice," got ");
        switch(m->tanks[id].bullet){
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
    if(m->tanks[id].kup && m->tanks[id].angle<90) {
        m->tanks[id].angle++;
        moved = true;
    }
    else if(m->tanks[id].kdown && m->tanks[id].angle>1) {
        m->tanks[id].angle--;
        moved = true;
    }
    // Fire
    if(m->tanks[id].kfire){
        if(m->tanks[id].power<1000)
            m->tanks[id].power+=10;
    }else if(m->tanks[id].power){
        fire_bullet_ang(m, m->tanks[id].bullet, m->tanks[id].x, m->tanks[id].y-7,m->tanks[id].facingleft?180-m->tanks[id].angle:m->tanks[id].angle, (float)m->tanks[id].power*0.01);
        m->tanks[id].bullet=1;
        m->tanks[id].power=0;
    }
    if (moved)
        broadcast_tank_chunk(m, MOVE, id);
}

void bounce_bullet(struct moag *m, int b, float hitx, float hity)
{
    const int ix=(int)hitx;
    const int iy=(int)hity;
    if(get_land_at(m, ix,iy)==-1){
        m->bullets[b].vx=-m->bullets[b].vx;
        m->bullets[b].vy=-m->bullets[b].vy;
        return;
    }
    m->bullets[b].fx=hitx;
    m->bullets[b].fy=hity;
    m->bullets[b].x=ix;
    m->bullets[b].y=iy;
    unsigned char hit=0;
    if(get_land_at(m, ix-1,iy-1)) hit|=1<<7;
    if(get_land_at(m, ix  ,iy-1)) hit|=1<<6;
    if(get_land_at(m, ix+1,iy-1)) hit|=1<<5;
    if(get_land_at(m, ix-1,iy  )) hit|=1<<4;
    if(get_land_at(m, ix+1,iy  )) hit|=1<<3;
    if(get_land_at(m, ix-1,iy+1)) hit|=1<<2;
    if(get_land_at(m, ix  ,iy+1)) hit|=1<<1;
    if(get_land_at(m, ix+1,iy+1)) hit|=1;
    const float IRT2=0.70710678;
    const float vx=m->bullets[b].vx;
    const float vy=m->bullets[b].vy;
    switch(hit){
    case 0x00: break;
    case 0x07: case 0xe0: case 0x02: case 0x40:
        m->bullets[b].vy=-vy; break;
    case 0x94: case 0x29: case 0x10: case 0x08:
        m->bullets[b].vx=-vx; break;
    case 0x16: case 0x68: case 0x04: case 0x20:
        m->bullets[b].vy=vx; m->bullets[b].vx=vy; break;
    case 0xd0: case 0x0b: case 0x80: case 0x01:
        m->bullets[b].vy=-vx; m->bullets[b].vx=-vy; break;
    case 0x17: case 0xe8: case 0x06: case 0x60:
        m->bullets[b].vx=+vx*IRT2+vy*IRT2; m->bullets[b].vy=-vy*IRT2+vx*IRT2; break;
    case 0x96: case 0x69: case 0x14: case 0x28:
        m->bullets[b].vx=-vx*IRT2+vy*IRT2; m->bullets[b].vy=+vy*IRT2+vx*IRT2; break;
    case 0xf0: case 0x0f: case 0xc0: case 0x03:
        m->bullets[b].vx=+vx*IRT2-vy*IRT2; m->bullets[b].vy=-vy*IRT2-vx*IRT2; break;
    case 0xd4: case 0x2b: case 0x90: case 0x09:
        m->bullets[b].vx=-vx*IRT2-vy*IRT2; m->bullets[b].vy=+vy*IRT2-vx*IRT2; break;
    default:
        m->bullets[b].vx=-vx; m->bullets[b].vy=-vy; break;
    }
}

void bullet_detonate(struct moag *m, int b)
{
    float d=sqrt(SQ(m->bullets[b].vx)+SQ(m->bullets[b].vy));
    if(d<0.001 && d>-0.001)
        d=d<0?-1:1;
    const float dx=m->bullets[b].vx/d;
    const float dy=m->bullets[b].vy/d;
    float hitx=m->bullets[b].fx;
    float hity=m->bullets[b].fy;
    for(int i=40; i>0 && get_land_at(m, (int)hitx,(int)hity); i--){
        hitx-=dx;
        hity-=dy;
    }
    switch(m->bullets[b].type){
    case 1: // missile
        explode(m, m->bullets[b].x,m->bullets[b].y, 12, 0);
        break;
    case 2: // baby nuke
        explode(m, m->bullets[b].x,m->bullets[b].y, 55, 0);
        break;
    case 3: // nuke
        explode(m, m->bullets[b].x,m->bullets[b].y, 150, 0);
        break;
    case 4: // dirt
        explode(m, m->bullets[b].x,m->bullets[b].y, 55, 1);
        break;
    case 5: // super dirt
        explode(m, m->bullets[b].x,m->bullets[b].y, 300, 1);
        break;
    case 6: // collapse
        explode(m, m->bullets[b].x,m->bullets[b].y, 120, 3);
        break;
    case 7: // liquid dirt
        liquid(m, (int)hitx, (int)hity, 3000);
        break;
    case 8: { // bouncer
        if(m->bullets[b].active>0)
            m->bullets[b].active=-BOUNCER_BOUNCES;
        m->bullets[b].active++;
        bounce_bullet(m, b,hitx,hity);
        m->bullets[b].vx*=0.9;
        m->bullets[b].vy*=0.9;
        explode(m, m->bullets[b].x,m->bullets[b].y, 12, 0);
    } break;
    case 9: //tunneler
        if(m->bullets[b].active>0)
            m->bullets[b].active=-TUNNELER_TUNNELINGS;
        m->bullets[b].active++;
        explode(m, hitx,hity, 9, 0);
        explode(m, hitx+8*dx,hity+8*dy, 9, 0);
        break;
    case 10: { //ladder
        int x=m->bullets[b].x;
        int y=m->bullets[b].y;
        for(;y<LAND_HEIGHT;y++) if(get_land_at(m, x,y)==0) break;
        for(;y<LAND_HEIGHT;y++) if(get_land_at(m, x,y)) break;
        const int maxy=y+1;
        y=m->bullets[b].y;
        for(;y>0;y--) if(get_land_at(m, x,y)==0) break;
        const int miny=y;
        for(;y<maxy;y+=2){
            set_land_at(m, x-1,y,0);
            set_land_at(m, x  ,y,1);
            set_land_at(m, x+1,y,0);
            set_land_at(m, x-1,y+1,1);
            set_land_at(m, x  ,y+1,1);
            set_land_at(m, x+1,y+1,1);
        }
        broadcast_land_chunk(m, x-1,miny,3,maxy-miny+1);
    } break;
    case 11: //MIRV
        bounce_bullet(m, b,hitx,hity);
        explode(m, m->bullets[b].x,m->bullets[b].y, 12, 0);
        for(int i=-3;i<4;i++)
            fire_bullet(m, 12,m->bullets[b].x,m->bullets[b].y,m->bullets[b].vx+i,m->bullets[b].vy);
        break;
    case 12: //MIRV warhead
        explode(m, m->bullets[b].x,m->bullets[b].y, 30, 0);
        break;
    case 13: //cluster bomb
        bounce_bullet(m, b,hitx,hity);
        explode(m, m->bullets[b].x,m->bullets[b].y, 20, 0);
        for(int i=0;i<11;i++)
            fire_bullet(m, 1,hitx,hity, 1.5*cosf(i*M_PI/5.5)+0.25*m->bullets[b].vx,
                                    1.5*sinf(i*M_PI/5.5)+0.5*m->bullets[b].vy);
        break;
    default: break;
    }
    if(m->bullets[b].active>0) {
        m->bullets[b].active=0;
        broadcast_bullet_chunk(m, KILL, b);
    }
}

void bullet_update(struct moag *m, int b)
{
    if(!m->bullets[b].active)
        return;
    if(m->bullets[b].type==10){ //special (ladder)
        m->bullets[b].active--;
        m->bullets[b].fx+=m->bullets[b].vx;
        m->bullets[b].fy+=m->bullets[b].vy;
        m->bullets[b].x=(int)m->bullets[b].fx;
        m->bullets[b].y=(int)m->bullets[b].fy;
        if(get_land_at(m, m->bullets[b].x,m->bullets[b].y)==1){
            explode(m, m->bullets[b].x,m->bullets[b].y+LADDER_LENGTH-m->bullets[b].active, 1, 2);
            bullet_detonate(m, b);
        }
        return;
    }
    m->bullets[b].fx+=m->bullets[b].vx;
    m->bullets[b].fy+=m->bullets[b].vy;
    m->bullets[b].vy+=GRAVITY;
    m->bullets[b].x=(int)m->bullets[b].fx;
    m->bullets[b].y=(int)m->bullets[b].fy;
    if(get_land_at(m, m->bullets[b].x,m->bullets[b].y)){
        bullet_detonate(m, b);
        return;
    }
    if(m->bullets[b].active>1){
        m->bullets[b].active--;
        return;
    }
    if(m->bullets[b].type==8 && m->bullets[b].active==1)
        m->bullets[b].active=-BOUNCER_BOUNCES;
    if(m->bullets[b].type==9 && m->bullets[b].active==1)
        m->bullets[b].active=-TUNNELER_TUNNELINGS;
    for(int i=0;i<MAX_CLIENTS;i++)
        if(SQ(m->tanks[i].x-m->bullets[b].x)+SQ(m->tanks[i].y-3-m->bullets[b].y)<72){
            bullet_detonate(m, b);
            return;
        }
    if(SQ(m->crate.x-m->bullets[b].x)+SQ(m->crate.y-4-m->bullets[b].y)<30){
        bullet_detonate(m, b);
        fire_bullet(m, m->crate.type, m->crate.x, m->crate.y-4, m->crate.type==8?(m->bullets[b].vx<0?-0.2:0.2):0, -0.2);
        m->crate.x=0;
        m->crate.y=0;
        return;
    }
    if(m->bullets[b].type==11 && m->bullets[b].vy>0){
        bullet_detonate(m, b);
        return;
    }

    if (m->bullets[b].active)
        broadcast_bullet_chunk(m, MOVE, b);
}

void crate_update(struct moag *m)
{
    if(!m->crate.active){
        m->crate.active = true;
        m->crate.x=rng_range(&m->rng, 20, LAND_WIDTH - 20);
        m->crate.y=30;
        explode(m, m->crate.x,m->crate.y-12, 12, 2);

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
        int r= rng_range(&m->rng, 0, TOTAL);
             if((r-=PBABYNUKE)<0)   m->crate.type=2;
        else if((r-=PNUKE)<0)       m->crate.type=3;
        else if((r-=PSUPERDIRT)<0)  m->crate.type=5;
        else if((r-=PLIQUIDDIRT)<0) m->crate.type=7;
        else if((r-=PCOLLAPSE)<0)   m->crate.type=6;
        else if((r-=PBOUNCER)<0)    m->crate.type=8;
        else if((r-=PTUNNELER)<0)   m->crate.type=9;
        else if((r-=PMIRV)<0)       m->crate.type=11;
        else if((r-=PCLUSTER)<0)    m->crate.type=13;
        else                        m->crate.type=4;
        broadcast_crate_chunk(m, SPAWN);
    }
    if(get_land_at(m, m->crate.x,m->crate.y+1)==0) {
        m->crate.y++;
        broadcast_crate_chunk(m, MOVE);
    }
}

void step_game(struct moag *m)
{
    crate_update(m);
    for(int i=0;i<MAX_CLIENTS;i++)
        tank_update(m, i);
    for(int i=0;i<MAX_BULLETS;i++)
        bullet_update(m, i);
}

int client_connect(struct moag *m)
{
    int i=0;
    while(m->tanks[i].active)
        if(++i>=MAX_CLIENTS) {
            printf("Client failed to connect, too many clients.\n");
            return -1;
        }

    printf("Client connected!\n");
    spawn_client(m, i);

    return i;
}

void handle_msg(struct moag *m, int id, const char* msg, int len)
{
    if(msg[0]=='/' && msg[1]=='n' && msg[2]==' '){
        char notice[64]="  ";
        strcat(notice,m->tanks[id].name);
        len-=3;
        if(len>15)
            len=15;
        for(int i=0;i<len;i++)
            m->tanks[id].name[i]=msg[i+3];
        m->tanks[id].name[len]='\0';
        strcat(notice," is now known as ");
        strcat(notice,m->tanks[id].name);
        broadcast_chat(id,NAME_CHANGE,m->tanks[id].name,strlen(m->tanks[id].name));
        broadcast_chat(-1,SERVER_NOTICE,notice,strlen(notice));
        return;
    }
    broadcast_chat(id,CHAT,msg,len);
}

void init_game(struct moag *m)
{
    for(int i=0;i<MAX_CLIENTS;i++)
        m->tanks[i].active=0;
    for(int i=0;i<MAX_BULLETS;i++)
        m->bullets[i].active=0;
    m->crate.active = false;

    rng_seed(&m->rng, time(NULL));

    for (int y = 0; y < LAND_HEIGHT; ++y) {
        for (int x = 0; x < LAND_WIDTH; ++x){
            if (y < LAND_HEIGHT / 3)
                set_land_at(m, x, y, 0);
            else
                set_land_at(m, x, y, 1);
        }
    }
}

void on_receive(struct moag *m, ENetEvent *ev)
{
    unsigned char *packet = ev->packet->data;
    size_t pos = 0;
    int i = (int)ev->peer->data;

    char byte = read8(packet, &pos);

    switch(byte){
    case KLEFT_PRESSED_CHUNK: m->tanks[i].kleft=1; break;
    case KLEFT_RELEASED_CHUNK: m->tanks[i].kleft=0; break;
    case KRIGHT_PRESSED_CHUNK: m->tanks[i].kright=1; break;
    case KRIGHT_RELEASED_CHUNK: m->tanks[i].kright=0; break;
    case KUP_PRESSED_CHUNK: m->tanks[i].kup=1; break;
    case KUP_RELEASED_CHUNK: m->tanks[i].kup=0; break;
    case KDOWN_PRESSED_CHUNK: m->tanks[i].kdown=1; break;
    case KDOWN_RELEASED_CHUNK: m->tanks[i].kdown=0; break;
    case KFIRE_PRESSED_CHUNK: m->tanks[i].kfire=1; break;
    case KFIRE_RELEASED_CHUNK: m->tanks[i].kfire=0; break;
    case CLIENT_MSG_CHUNK: { //msg
        unsigned char len = read8(packet, &pos);
        char* msg=malloc(len);
        for(int j=0;j<len;j++)
            msg[j] = read8(packet, &pos);
        handle_msg(m, i,msg,len);
        free(msg);
        break;
    }
    default: break;
    }
}

int main(int argc, char *argv[])
{
    init_enet_server(PORT);


    struct moag moag;
    init_game(&moag);

    ENetEvent event;

    for (;;) {
        while (enet_host_service(get_server_host(), &event, 10)) {
            switch (event.type) {
            case ENET_EVENT_TYPE_CONNECT:
                event.peer->data = (void *)client_connect(&moag);
                break;

            case ENET_EVENT_TYPE_DISCONNECT:
                disconnect_client(&moag, (int)event.peer->data);
                break;

            case ENET_EVENT_TYPE_RECEIVE:
                on_receive(&moag, &event);
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

        step_game(&moag);
    }

    uninit_enet();

    return EXIT_SUCCESS;
}
