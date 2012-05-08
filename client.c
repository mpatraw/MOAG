
#include "client.h"

const char tanksprite[14][19] = {
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

const char cratesprite[9][10] = {
    ".xxxxxxx.",
    "xx.....xx",
    "x.x...x.x",
    "x..x.x..x",
    "x...x...x",
    "x..x.x..x",
    "x.x...x.x",
    "xx.....xx",
    ".xxxxxxx.",
};

ENetHost *g_client = NULL;
ENetPeer *g_peer = NULL;

struct chatline chatlines[CHAT_LINES];
struct tank tanks[MAX_CLIENTS];
struct bullet bullets[MAX_BULLETS];
struct crate crate;
char land[LAND_WIDTH * LAND_HEIGHT];

char *typing_str = NULL;
bool typing_done = false;
bool kleft = false;
bool kright = false;
bool kup = false;
bool kdown = false;
bool kfire = false;
int num_bullets = 0;

void set_pixel(int x, int y, int r, int g, int b)
{
    SDL_Surface *surface = SDL_GetVideoSurface();
    Uint8 bpp = surface->format->BytesPerPixel;
    Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;
    Uint32 pixel = SDL_MapRGB(surface->format, r, g, b);

    switch (bpp) {
    case 1:
        *p = pixel;
        break;
    case 2:
        *(Uint16 *)p = pixel;
        break;
    case 3:
        if(SDL_BYTEORDER == SDL_BIG_ENDIAN) {
            p[0] = (pixel >> 16) & 0xff;
            p[1] = (pixel >> 8) & 0xff;
            p[2] = pixel & 0xff;
        } else {
            p[0] = pixel & 0xff;
            p[1] = (pixel >> 8) & 0xff;
            p[2] = (pixel >> 16) & 0xff;
        }
        break;
    case 4:
        *(Uint32 *)p = pixel;
        break;
    }
}

void draw_tank(int x, int y, int turretangle, bool facing_left)
{
    if (x<0 || x>LAND_WIDTH-18 || y<0 || y>LAND_HEIGHT-14)
        return;
    for (int iy=0;iy<14;iy++)
    for (int ix=0;ix<18;ix++)
        if(tanksprite[iy][ix]=='x')
            set_pixel(x+ix,y+iy,240,240,240);
    if(turretangle==90)
        turretangle=89;

    float step=tanf((float)turretangle*M_PI/180.0);
    int xlen=6.5*cosf((float)turretangle*M_PI/180.0);
    float rise=0;
    for(int ix=0;ix<=xlen;ix++){
        int next=rise+step;
        if(next>6) next=6;
        for(int iy=-(int)rise;iy>=-next;iy--)
            set_pixel(x+(facing_left?9-ix:8+ix),y+6+iy,240,240,240);
        rise+=step;
    }
}

void draw_crate(int x, int y){
    if(x<0 || x>LAND_WIDTH-9 || y<0 || y>LAND_HEIGHT-9)
        return;
    for(int iy=0;iy<9;iy++)
    for(int ix=0;ix<9;ix++)
        if(cratesprite[iy][ix]=='x')
            set_pixel(x+ix,y+iy,240,240,240);
}

void draw_bullets(void)
{
    for(int i=0;i<num_bullets;i++){
        if (bullets[i].x<1 || bullets[i].x>=LAND_WIDTH-1 ||
            bullets[i].y<1 || bullets[i].y>=LAND_HEIGHT-1)
            continue;
        set_pixel(bullets[i].x,bullets[i].y-1,255,255,255);
        set_pixel(bullets[i].x-1,bullets[i].y,255,255,255);
        set_pixel(bullets[i].x,  bullets[i].y,255,255,255);
        set_pixel(bullets[i].x+1,bullets[i].y,255,255,255);
        set_pixel(bullets[i].x,bullets[i].y+1,255,255,255);
    }
}

void del_chat_line(void)
{
    if(chatlines[0].str && chatlines[0].expire<SDL_GetTicks()){
        free(chatlines[0].str);
        for(int i=0;i<CHAT_LINES-1;i++){
            chatlines[i].expire=chatlines[i+1].expire;
            chatlines[i].str=chatlines[i+1].str;
        }
        chatlines[CHAT_LINES-1].str=NULL;
    }
}

void add_chat_line(char* str)
{
    int i=0;
    while(chatlines[i].str)
        if(++i>=CHAT_LINES){
            chatlines[0].expire=0;
            del_chat_line();
            i=CHAT_LINES-1;
            break;
        }
    chatlines[i].str=str;
    chatlines[i].expire=SDL_GetTicks()+CHAT_EXPIRETIME;
}

void draw(void)
{
    for(int i=0;i<MAX_CLIENTS;i++)
        if(tanks[i].active)
            draw_tank(tanks[i].x-9,tanks[i].y-13,tanks[i].angle,tanks[i].facingLeft);
            /*moag::SetStringCentered(tanks[i].x,tanks[i].y-36,tanks[i].name,240,240,240);
            int w, h;
            moag::GetStringSize(tanks[i].name, &w, &h);*/
    if(crate.x || crate.y)
        draw_crate(crate.x-4,crate.y-8);
    draw_bullets();
    del_chat_line();
    /*for(int i=0;i<CHAT_LINES;i++)
        if(chatlines[i].str)
        {
            moag::SetString(4,4+12*i,chatlines[i].str,255,255,255);
            int w, h;
            moag::GetStringSize(chatlines[i].str, &w, &h);
        }*/
    /*if(typingStr){
        moag::SetBlock(6,8+12*(CHAT_LINES),4,4,210,210,210);
        pushRedraw(6,8+12*(CHAT_LINES),4,4);
        moag::SetString(16,4+12*(CHAT_LINES),typingStr,210,210,210);
        int w, h;
        moag::GetStringSize(typingStr, &w, &h);
        pushRedraw(16,4+12*(CHAT_LINES),w,h);
    }*/
}

void initClient() {
    for(int i=0;i<CHAT_LINES;i++)
        chatlines[i].str=NULL;
    for(int i=0;i<MAX_CLIENTS;i++){
        tanks[i].active=0;
    }
    crate.x=0;
    crate.y=0;
    kleft=0;
    kright=0;
    kup=0;
    kdown=0;
    kfire=0;
}

int main(int argc, char *argv[])
{
    if (argc < 2) {
        printf("usage:  %s [address]\n", argv[0]);
        return EXIT_SUCCESS;
    }

    init_enet(argv[1]);
    init_sdl();

    ENetEvent enet_ev;
    SDL_Event sdl_ev;
    bool running = true;

    while (running) {
        while (SDL_PollEvent(&sdl_ev)) {
            switch (sdl_ev.type) {
            case SDL_QUIT:
                running = false;
                break;

            default:
                break;
            }
        }

        while (enet_host_service(g_client, &enet_ev, 10)) {
            switch (enet_ev.type) {
            case ENET_EVENT_TYPE_CONNECT:
                break;

            case ENET_EVENT_TYPE_DISCONNECT:
                break;

            case ENET_EVENT_TYPE_RECEIVE:
                enet_packet_destroy(enet_ev.packet);
                break;

            default:
                break;
            }
        }

        SDL_FillRect(SDL_GetVideoSurface(), NULL, 0);
        draw_tank(50, 50, 45, true);
        draw_crate(40, 40);
        SDL_Flip(SDL_GetVideoSurface());
    }

    enet_peer_disconnect(g_peer, 0);

    while (enet_host_service(g_client, &enet_ev, 3000)) {
        switch (enet_ev.type) {
        case ENET_EVENT_TYPE_RECEIVE:
            enet_packet_destroy(enet_ev.packet);
            break;

        case ENET_EVENT_TYPE_DISCONNECT:
            uninit_sdl();
            uninit_enet();
            exit(EXIT_SUCCESS);

        default:
            break;
        }
    }

    enet_peer_reset(g_peer);

    exit(EXIT_SUCCESS);
}

void init_enet(const char *ip)
{
    if (enet_initialize() != 0)
        die("An error occurred while initializing ENet.\n");
    atexit(enet_deinitialize);

    g_client = enet_host_create(NULL, MAX_CLIENTS, NUM_CHANNELS, 0, 0);
    if (!g_client)
        die("An error occurred while trying to create an ENet client host.\n");

    ENetAddress address;
    enet_address_set_host(&address, ip);
    address.port = PORT;

    g_peer = enet_host_connect(g_client, &address, NUM_CHANNELS, 0);
    if (!g_peer)
        die("No available peers for initiating an ENet connection.\n");

    ENetEvent enet_ev;

    if (enet_host_service(g_client, &enet_ev, CONNECT_TIMEOUT) == 0 ||
        enet_ev.type != ENET_EVENT_TYPE_CONNECT) {
        enet_peer_reset(g_peer);
        die("Connection to %s timed out.\n", ip);
    }
}

void uninit_enet(void)
{
    enet_host_destroy(g_client);
}

void init_sdl(void)
{
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
        die("%s\n", SDL_GetError());

    SDL_Surface *s = SDL_SetVideoMode(LAND_WIDTH, LAND_HEIGHT, 32, SDL_DOUBLEBUF);
    if (!s)
        die("%s\n", SDL_GetError());
}

void uninit_sdl(void)
{
    SDL_Quit();
}
