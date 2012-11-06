
#include "client.h"

#define X true
#define _ false

#define TANK_WIDTH  18
#define TANK_HEIGHT 14
const bool tanksprite[TANK_WIDTH * TANK_HEIGHT] =
{
    _,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,
    _,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,
    _,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,
    _,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,
    _,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,
    _,_,_,_,_,_,_,_,X,X,_,_,_,_,_,_,_,_,
    _,_,_,_,_,_,_,X,X,X,X,_,_,_,_,_,_,_,
    _,_,_,_,_,_,X,X,X,X,X,X,_,_,_,_,_,_,
    _,_,_,X,X,X,X,X,X,X,X,X,X,X,X,_,_,_,
    _,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,_,
    X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,
    X,X,X,_,X,X,_,X,X,_,X,X,_,X,X,_,X,X,
    _,X,_,_,X,X,_,X,_,_,X,_,_,X,_,_,X,_,
    _,_,X,X,X,X,X,X,X,X,X,X,X,X,X,X,_,_,
};

#define CRATE_WIDTH  9
#define CRATE_HEIGHT 9
bool cratesprite[CRATE_WIDTH * CRATE_HEIGHT] =
{
    _,X,X,X,X,X,X,X,_,
    X,X,_,_,_,_,_,X,X,
    X,_,X,_,_,_,X,_,X,
    X,_,_,X,_,X,_,_,X,
    X,_,_,_,X,_,_,_,X,
    X,_,_,X,_,X,_,_,X,
    X,_,X,_,_,_,X,_,X,
    X,X,_,_,_,_,_,X,X,
    _,X,X,X,X,X,X,X,_,
};

#define BULLET_WIDTH  3
#define BULLET_HEIGHT 3
bool bulletsprite[BULLET_WIDTH * BULLET_HEIGHT] =
{
    _,X,_,
    X,X,X,
    _,X,_,
};

struct chatline chatlines[CHAT_LINES] = {{0}};

char *typing_str = NULL;
bool kleft = false;
bool kright = false;
bool kup = false;
bool kdown = false;
bool kfire = false;
uint32_t kfire_held_start = 0;

void draw_tank(int x, int y, int turretangle, bool facingleft)
{
    draw_sprite(x, y, COLOR_WHITE, tanksprite, TANK_WIDTH, TANK_HEIGHT);

    /* 9 is the length of the cannon. */
    int ex = 9 * cos(DEG2RAD(turretangle)) * (facingleft ? -1 : 1);
    int ey = 9 * sin(DEG2RAD(turretangle)) * -1;
    draw_line(x + TANK_WIDTH / 2, y + TANK_HEIGHT / 2,
              x + TANK_WIDTH / 2 + ex, y + TANK_HEIGHT / 2 + ey,
              COLOR_WHITE);
}

void draw_crate(int x, int y)
{
    draw_sprite(x, y, COLOR_BROWN, cratesprite, CRATE_WIDTH, CRATE_HEIGHT);
}

void draw_bullets(struct moag *m)
{
    for (int i = 0; i < MAX_BULLETS; i++)
    {
        struct bullet *b = &m->bullets[i];
        if (b->active)
            draw_sprite(b->x, b->y, COLOR_RED, bulletsprite, BULLET_WIDTH, BULLET_HEIGHT);
    }
}

void del_chat_line(void)
{
    if (chatlines[0].str && chatlines[0].expire < SDL_GetTicks())
    {
        free(chatlines[0].str);
        for (int i = 0; i < CHAT_LINES - 1; i++){
            chatlines[i].expire = chatlines[i + 1].expire;
            chatlines[i].str = chatlines[i + 1].str;
        }
        chatlines[CHAT_LINES - 1].str = NULL;
    }
}

void add_chat_line(char* str)
{
    int i = 0;
    while (chatlines[i].str)
    {
        if (++i >= CHAT_LINES)
        {
            chatlines[0].expire = 0;
            del_chat_line();
            i = CHAT_LINES - 1;
            break;
        }
    }
    chatlines[i].str = str;
    chatlines[i].expire = SDL_GetTicks() + CHAT_EXPIRETIME;
}

void draw(struct moag *m)
{
    for (int x = 0; x < LAND_WIDTH; ++x)
    {
        for (int y = 0; y < LAND_HEIGHT; ++y)
        {
            if (get_land_at(m, x, y))
                set_pixel(x, y, COLOR_GRAY);
        }
    }

    if (m->crate.active)
        draw_crate(m->crate.x-4,m->crate.y-8);

    draw_bullets(m);

    for (int i = 0; i < MAX_PLAYERS; i++)
    {
        if (m->players[i].connected)
        {
            draw_tank(m->players[i].tank.x - 9,
                      m->players[i].tank.y - 13,
                      m->players[i].tank.angle,
                      m->players[i].tank.facingleft);
            draw_string_centered(m->players[i].tank.x,
                                 m->players[i].tank.y - 36,
                                 COLOR_WHITE,
                                 m->players[i].name);
        }
    }

    for (int i = 0; i < 10; ++i)
    {
        if (kfire && SDL_GetTicks() - kfire_held_start >= (i * 200))
        {
            draw_block(LAND_WIDTH / 2 - 75 + i * 10, LAND_HEIGHT - 17, 10, 7, COLOR_RED);
        }
        else
        {
            draw_block(LAND_WIDTH / 2 - 75 + i * 10, LAND_HEIGHT - 16, 10, 5, COLOR_BLUE);
        }
    }

    del_chat_line();
    for (int i = 0; i < CHAT_LINES; i++)
    {
        if (chatlines[i].str)
        {
            draw_string(4, 4 + 12 * i, COLOR_WHITE, chatlines[i].str);
            int w, h;
            get_string_size(chatlines[i].str, &w, &h);
        }
    }
    if (typing_str)
    {
        draw_block(6, 8 + 12 * (CHAT_LINES), 4, 4, COLOR_DARK_GRAY);
        draw_string(16, 4 + 12 * (CHAT_LINES), COLOR_DARK_GRAY, typing_str);
    }
}

void on_receive(struct moag *m, ENetEvent *ev)
{
    struct chunk_header *chunk;

    chunk = receive_chunk(ev->packet);

    switch (chunk->type)
    {
        case LAND_CHUNK:
        {
            struct land_chunk *land = (void *)chunk;
            int i = 0;
            for (int y = land->y; y < land->height + land->y; ++y)
            {
                for (int x = land->x; x < land->width + land->x; ++x)
                {
                    set_land_at(m, x, y, land->data[i]);
                    i++;
                }
            }
            break;
        }

        case TANK_CHUNK:
        {
            struct tank_chunk *tank = (void *)chunk;
            int id = tank->id;

            assert(id >= 0 && id <= MAX_PLAYERS);

            if (tank->action == SPAWN)
            {
                m->players[id].connected = true;

                m->players[id].tank.x = tank->x;
                m->players[id].tank.y = tank->y;
                char angle = tank->angle;

                m->players[id].tank.facingleft = false;
                if (angle < 0){
                    angle = -angle;
                    m->players[id].tank.facingleft = true;
                }
                m->players[id].tank.angle = angle;
            }
            else if (tank->action == MOVE)
            {
                m->players[id].tank.x = tank->x;
                m->players[id].tank.y = tank->y;
                char angle = tank->angle;

                m->players[id].tank.facingleft = false;
                if (angle < 0){
                    angle = -angle;
                    m->players[id].tank.facingleft = true;
                }
                m->players[id].tank.angle = angle;
            }
            else if (tank->action == KILL)
            {
                m->players[id].tank.x = -1;
                m->players[id].tank.y = -1;
                m->players[id].connected = false;
            }
            else
            {
                DIE("Invalid TANK_CHUNK type.\n");
            }
            break;
        }

        case BULLET_CHUNK:
        {
            struct bullet_chunk *bullet = (void *)chunk;
            int id = bullet->id;

            if (bullet->action == SPAWN)
            {
                m->bullets[id].active = true;
                m->bullets[id].x = bullet->x;
                m->bullets[id].y = bullet->y;
            }
            else if (bullet->action == MOVE)
            {
                m->bullets[id].x = bullet->x;
                m->bullets[id].y = bullet->y;
            }
            else if (bullet->action == KILL)
            {
                m->bullets[id].active = false;
            }
            else
            {
                DIE("Invalid BULLET_CHUNK type.\n");
            }
            break;
        }

        case SERVER_MSG_CHUNK:
        {
            struct server_msg_chunk *server_msg = (void *)chunk;
            int id = server_msg->id;
            unsigned char len = ev->packet->dataLength - sizeof(struct server_msg_chunk);

            switch (server_msg->action)
            {
                case CHAT:
                {
                    int namelen = strlen(m->players[id].name);
                    int linelen = namelen + len + 4;
                    char *line = safe_malloc(linelen);
                    line[0] = '<';
                    for(int i = 0; i < namelen; i++)
                        line[i + 1] = m->players[id].name[i];
                    line[namelen+1] = '>';
                    line[namelen+2] = ' ';
                    for (int i = 0; i < len; ++i)
                        line[namelen + 3 + i] = server_msg->data[i];
                    line[linelen - 1] = '\0';
                    add_chat_line(line);
                    break;
                }

                case NAME_CHANGE:
                {
                    if (len < 1 || len > 15)
                        break;
                    for (int i = 0; i < len; ++i)
                        m->players[id].name[i] = server_msg->data[i];
                    m->players[id].name[len]='\0';
                    break;
                }

                case SERVER_NOTICE:
                {
                    add_chat_line(string_duplicate((char *)server_msg->data));
                    break;
                }

                default:
                    DIE("Invalid SERVER_MSG_CHUNK action (%d).\n", server_msg->action);
            }
            break;
        }

        case CRATE_CHUNK:
        {
            struct crate_chunk *crate = (void *)chunk;

            if (crate->action == SPAWN)
            {
                m->crate.active = true;
                m->crate.x = crate->x;
                m->crate.y = crate->y;
            }
            else if (crate->action == MOVE)
            {
                m->crate.x = crate->x;
                m->crate.y = crate->y;
            }
            else if (crate->action == KILL)
            {
                m->crate.active = false;
            }
            else
            {
                DIE("Invalid CRATE_CHUNK type.\n");
            }
            break;
        }

        default:
            DIE("Invalid CHUNK type (%d).\n", chunk->type);
    }

    free(chunk);
}

int main(int argc, char *argv[])
{
    if (argc < 2) {
        printf("usage:  %s [address]\n", argv[0]);
        return EXIT_SUCCESS;
    }

    init_enet_client(argv[1], PORT);
    init_sdl(LAND_WIDTH, LAND_HEIGHT, "MOAG");

    if (!set_font("Nouveau_IBM.ttf", 14))
        DIE("Failed to open 'Nouveau_IBM.ttf'\n");

    struct moag moag;

    memset(&moag, 0, sizeof(moag));

    ENetEvent enet_ev;

    while (!is_closed()) {

        grab_events();

        if (typing_str && is_text_input())
        {
            if (is_key_down(SDLK_ESCAPE)
                || is_key_down(SDLK_LEFT)
                || is_key_down(SDLK_RIGHT)
                || is_key_down(SDLK_UP)
                || is_key_down(SDLK_DOWN))
            {
                typing_str = NULL;
                stop_text_input();
            }
            else if (is_key_down(SDLK_RETURN))
            {
                unsigned char buffer[256];
                size_t pos = 0;

                unsigned char len = strlen(typing_str);
                write8(buffer, &pos, CLIENT_MSG_CHUNK);
                write8(buffer, &pos, len);
                for (int i = 0; i < len; ++i)
                    write8(buffer, &pos, typing_str[i]);

                send_packet(buffer, pos, false, true);

                stop_text_input();
                typing_str = NULL;
                stop_text_input();
            }
        }
        else
        {
            if (is_key_down(SDLK_LEFT) && !kleft)
            {
                send_input_chunk(KLEFT_PRESSED, 0);
                kleft = true;
            }
            else if (!is_key_down(SDLK_LEFT) && kleft)
            {
                send_input_chunk(KLEFT_RELEASED, 0);
                kleft = false;
            }

            if (is_key_down(SDLK_RIGHT) && !kright)
            {
                send_input_chunk(KRIGHT_PRESSED, 0);
                kright = true;
            }
            else if (!is_key_down(SDLK_RIGHT) && kright)
            {
                send_input_chunk(KRIGHT_RELEASED, 0);
                kright = false;
            }

            if (is_key_down(SDLK_UP) && !kup)
            {
                send_input_chunk(KUP_PRESSED, 0);
                kup = true;
            }
            else if (!is_key_down(SDLK_UP) && kup)
            {
                send_input_chunk(KUP_RELEASED, 0);
                kup = false;
            }

            if (is_key_down(SDLK_DOWN) && !kdown)
            {
                send_input_chunk(KDOWN_PRESSED, 0);
                kdown = true;
            }
            else if (!is_key_down(SDLK_DOWN) && kdown)
            {
                send_input_chunk(KDOWN_RELEASED, 0);
                kdown = false;
            }

            if (is_key_down(' ') && !kfire)
            {
                send_input_chunk(KFIRE_PRESSED, 0);
                kfire = true;
                kfire_held_start = SDL_GetTicks();
            }
            else if (!is_key_down(' ') && kfire)
            {
                send_input_chunk(KFIRE_RELEASED, SDL_GetTicks() - kfire_held_start);
                kfire = false;
                kfire_held_start = 0;
            }

            if (is_key_down('t'))
            {
                typing_str = start_text_input();
            }
            if (is_key_down('/'))
            {
                typing_str = start_text_cmd_input();
            }
        }

        while (enet_host_service(get_client_host(), &enet_ev, 0))
        {
            switch (enet_ev.type)
            {
                case ENET_EVENT_TYPE_CONNECT:
                    break;

                case ENET_EVENT_TYPE_DISCONNECT:
                    LOG("Disconnected from server.\n");
                    close_window();
                    break;

                case ENET_EVENT_TYPE_RECEIVE:
                    on_receive(&moag, &enet_ev);
                    enet_packet_destroy(enet_ev.packet);
                    break;

                default:
                    break;
            }
        }

        SDL_FillRect(SDL_GetVideoSurface(), NULL, COLOR_BLACK);
        draw(&moag);
        char buf[256];
        sprintf(buf, "%u", get_peer()->roundTripTime);
        draw_string_right(LAND_WIDTH, 0, COLOR_GREEN, buf);
        SDL_Flip(SDL_GetVideoSurface());

#ifdef WIN32
        SDL_Delay(10);
#else
        struct timespec t;
        t.tv_sec = 0;
        t.tv_nsec = 10000000;
        while (nanosleep(&t, &t) == -1)
            continue;
#endif
    }

    uninit_sdl();
    uninit_enet();

    exit(EXIT_SUCCESS);
}
