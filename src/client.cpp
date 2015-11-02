
#include <iostream>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

extern "C" {
#include "common.h"
#include "moag.h"
}

static SDL_Window *main_window = nullptr;
static SDL_Renderer *main_renderer = nullptr;
static SDL_Texture *tank_texture = nullptr;
static SDL_Texture *turret_texture = nullptr;
static SDL_Texture *bullet_texture = nullptr;
static SDL_Texture *crate_texture = nullptr;
static TTF_Font *main_font = nullptr;

static int tank_width;
static int tank_height;
static int turret_width;
static int turret_height;
static int bullet_width;
static int bullet_height;
static int crate_width;
static int crate_height;

static SDL_Texture *load_texture_from_file(const char *filename)
{
    auto surface = IMG_Load(filename);
    if (!surface) {
        return nullptr;
    }
    auto texture = SDL_CreateTextureFromSurface(main_renderer, surface);
    SDL_FreeSurface(surface);
    return texture;
}

static SDL_Texture *create_string_texture(const char *text, SDL_Color c={255, 255, 255})
{
    auto surface = TTF_RenderText_Solid(main_font, text, c);
    if (!surface) {
        return nullptr;
    }
    auto texture = SDL_CreateTextureFromSurface(main_renderer, surface);
    SDL_FreeSurface(surface);
    return texture;
}

static void quick_render_string(int x, int y, const char *text, SDL_Color c={255, 255, 255})
{
    auto texture = create_string_texture(text);
    if (!texture) {
        return;
    }
    int w, h;
    SDL_QueryTexture(texture, NULL, NULL, &w, &h, c);
    SDL_Rect src = {x, y, w, h};
    SDL_RenderCopy(main_renderer, texture, NULL, &src);
    SDL_DestroyTexture(texture);
}

#define BUFLEN          256
#define CHAT_LINES      7
#define CHAT_EXPIRETIME 18000

struct chatline
{
    int expire;
    SDL_Texture *text;
};

static inline void send_input_chunk(int key, uint16_t t)
{
    struct input_chunk chunk;
    chunk._.type = INPUT_CHUNK;
    chunk.key = key;
    chunk.ms = t;

    send_chunk((struct chunk_header *)&chunk, sizeof chunk, false, true);

    LOG("%u: %s: %zu\n", (unsigned)time(NULL), __PRETTY_FUNCTION__, sizeof chunk);
}

struct chatline chatlines[CHAT_LINES] = {{0}};

std::string typing_str;
bool kleft = false;
bool kright = false;
bool kup = false;
bool kdown = false;
bool kfire = false;
uint32_t kfire_held_start = 0;

void draw_tank(int x, int y, int turret_angle, bool facingleft)
{
    SDL_Rect tank_src = {x - tank_width / 2, y - tank_height, tank_width, tank_height};
    SDL_RenderCopy(main_renderer, tank_texture, NULL, &tank_src);
    if (facingleft) {
        turret_angle = 180 + turret_angle;
    } else {
        turret_angle = 360 - turret_angle;
    }
    SDL_Rect turret_src = {x, y - tank_height, turret_width, turret_height};
    SDL_Point cen = {0, turret_height / 2};
    SDL_RenderCopyEx(main_renderer, turret_texture, NULL, &turret_src, turret_angle, &cen, SDL_FLIP_NONE);
}

void draw_crate(int x, int y)
{
    x = x - (crate_width / 2);
    SDL_Rect src = {x, y, crate_width, crate_height};
    SDL_RenderCopy(main_renderer, crate_texture, NULL, &src);
}

void draw_bullets(struct moag *m)
{
    for (int i = 0; i < MAX_BULLETS; i++)
    {
        struct bullet *b = &m->bullets[i];
        if (b->active) {
            SDL_Rect src = {b->x, b->y, bullet_width, bullet_height};
            SDL_RenderCopy(main_renderer, bullet_texture, NULL, &src);
        }
    }
}

void del_chat_line(void)
{
    if (chatlines[0].expire < SDL_GetTicks())
    {
        SDL_DestroyTexture(chatlines[0].text);
        for (int i = 0; i < CHAT_LINES - 1; i++){
            chatlines[i].expire = chatlines[i + 1].expire;
            chatlines[i].text = chatlines[i + 1].text;
        }
        chatlines[CHAT_LINES - 1].text = NULL;
    }
}

void add_chat_line(char* str)
{
    int i = 0;
    while (chatlines[i].text)
    {
        if (++i >= CHAT_LINES)
        {
            chatlines[0].expire = 0;
            del_chat_line();
            i = CHAT_LINES - 1;
            break;
        }
    }
    chatlines[i].text = create_string_texture(str);
    chatlines[i].expire = SDL_GetTicks() + CHAT_EXPIRETIME;
}

void draw(struct moag *m)
{
    SDL_SetRenderDrawColor(main_renderer, 128, 128, 128, 255);
    for (int x = 0; x < LAND_WIDTH; ++x)
    {
        for (int y = 0; y < LAND_HEIGHT; ++y)
        {
            if (get_land_at(m, x, y)) {
                SDL_RenderDrawPoint(main_renderer, x, y);
            }
        }
    }

    if (m->crate.active)
        draw_crate(m->crate.x-4,m->crate.y-8);

    draw_bullets(m);

    for (int i = 0; i < MAX_PLAYERS; i++)
    {
        if (m->players[i].connected)
        {
            draw_tank(m->players[i].tank.x,
                      m->players[i].tank.y,
                      m->players[i].tank.angle,
                      m->players[i].tank.facingleft);
        }
    }

    del_chat_line();
    for (int i = 0; i < CHAT_LINES; i++)
    {
        if (chatlines[i].text)
        {
            int w, h;
            SDL_QueryTexture(chatlines[i].text, NULL, NULL, &w, &h);
            SDL_Rect src = {4, 4 + 12 * i, w, h};
            SDL_RenderCopy(main_renderer, chatlines[i].text, NULL, &src);
        }
    }
    if (SDL_IsTextInputActive())
    {
        SDL_SetRenderDrawColor(main_renderer, 128, 128, 128, 255);
        SDL_Rect rect = {6, 8 + 12 * (CHAT_LINES), 4, 4};
        SDL_RenderDrawRect(main_renderer, &rect);

        //draw_string(16, 4 + 12 * (CHAT_LINES), COLOR_MOAG_LIGHT_GRAY, typing_str);
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
            struct land_chunk *land = (struct land_chunk *)chunk;
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

        case PACKED_LAND_CHUNK:
        {
            struct packed_land_chunk *land = (struct packed_land_chunk *)chunk;
            const size_t packed_len = ev->packet->dataLength - sizeof(struct packed_land_chunk);
            size_t datalen = 0;
            uint8_t *data = rldecode(land->data, packed_len, &datalen);

            int i = 0;
            for (int y = land->y; y < land->height + land->y; ++y)
            {
                if (i >= datalen) break;
                for (int x = land->x; x < land->width + land->x; ++x)
                {
                    set_land_at(m, x, y, data[i]);
                    i++;
                    if (i >= datalen) break;
                }
            }

            free(data);
            break;
        }

        case TANK_CHUNK:
        {
            struct tank_chunk *tank = (struct tank_chunk *)chunk;
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
            struct bullet_chunk *bullet = (struct bullet_chunk *)chunk;
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
            struct server_msg_chunk *server_msg = (struct server_msg_chunk *)chunk;
            int id = server_msg->id;
            unsigned char len = ev->packet->dataLength - sizeof(struct server_msg_chunk);

            switch (server_msg->action)
            {
                case CHAT:
                {
                    int namelen = strlen(m->players[id].name);
                    int linelen = namelen + len + 4;
                    char *line = (char *)safe_malloc(linelen);
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
            struct crate_chunk *crate = (struct crate_chunk *)chunk;

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

void client_main(void)
{
    init_enet_client(HOST, PORT);

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::cerr << SDL_GetError() << std::endl;
        goto sdl_init_fail;
    }

    if (TTF_Init() != 0) {
        std::cerr << SDL_GetError() << std::endl;
        goto ttf_init_fail;
    }

    main_window = SDL_CreateWindow("MOAG", -1, -1, LAND_WIDTH, LAND_HEIGHT, 0);
    if (!main_window) {
        std::cerr << SDL_GetError() << std::endl;
        goto main_window_fail;
    }

    main_renderer = SDL_CreateRenderer(main_window, -1, SDL_RENDERER_ACCELERATED);
    if (!main_renderer) {
        std::cerr << SDL_GetError() << std::endl;
        goto main_renderer_fail;
    }

    tank_texture = load_texture_from_file("tank.png");
    turret_texture = load_texture_from_file("turret.png");
    bullet_texture = load_texture_from_file("bullet.png");
    crate_texture = load_texture_from_file("crate.png");
    if (!tank_texture || !turret_texture || !bullet_texture || !crate_texture) {
        std::cerr << SDL_GetError() << std::endl;
        goto texture_load_fail;
    }
    SDL_QueryTexture(tank_texture, NULL, NULL, &tank_width, &tank_height);
    SDL_QueryTexture(turret_texture, NULL, NULL, &turret_width, &turret_height);
    SDL_QueryTexture(bullet_texture, NULL, NULL, &bullet_width, &bullet_height);
    SDL_QueryTexture(crate_texture, NULL, NULL, &crate_width, &crate_height);

    main_font = TTF_OpenFont("Nouveau_IBM.ttf", 14);
    if (!main_font) {
        std::cerr << SDL_GetError() << std::endl;
        goto font_load_fail;
    }

    struct moag moag;

    memset(&moag, 0, sizeof(moag));

    ENetEvent enet_ev;

    while (true) {
        SDL_Event ev;
        while (SDL_PollEvent(&ev)) {
            switch (ev.type) {
            case SDL_QUIT:
                goto end_loop;
                break;
            } 
        }

        auto kb = SDL_GetKeyboardState(NULL);

        if (SDL_IsTextInputActive())
        {
            if (kb[SDL_SCANCODE_ESCAPE]
                || kb[SDL_SCANCODE_LEFT]
                || kb[SDL_SCANCODE_RIGHT]
                || kb[SDL_SCANCODE_UP]
                || kb[SDL_SCANCODE_DOWN])
            {
                SDL_StopTextInput();
            }
            else if (kb[SDL_SCANCODE_RETURN])
            {
                // length of typing_str including null + sizeof(client_msg_chunk)
                unsigned char buffer[257];
                size_t pos = 0;

                unsigned char len = typing_str.length() + 1;
                write8(buffer, &pos, CLIENT_MSG_CHUNK);
                //write8(buffer, &pos, len);
                for (int i = 0; i < len; ++i)
                    write8(buffer, &pos, typing_str[i]);

                send_packet(buffer, pos, false, true);

                SDL_StopTextInput();
            }
        }
        else
        {
            if (kb[SDL_SCANCODE_LEFT] && !kleft)
            {
                send_input_chunk(KLEFT_PRESSED, 0);
                kleft = true;
            }
            else if (!kb[SDL_SCANCODE_LEFT] && kleft)
            {
                send_input_chunk(KLEFT_RELEASED, 0);
                kleft = false;
            }

            if (kb[SDL_SCANCODE_RIGHT] && !kright)
            {
                send_input_chunk(KRIGHT_PRESSED, 0);
                kright = true;
            }
            else if (!kb[SDL_SCANCODE_RIGHT] && kright)
            {
                send_input_chunk(KRIGHT_RELEASED, 0);
                kright = false;
            }

            if (kb[SDL_SCANCODE_UP] && !kup)
            {
                send_input_chunk(KUP_PRESSED, 0);
                kup = true;
            }
            else if (!kb[SDL_SCANCODE_UP] && kup)
            {
                send_input_chunk(KUP_RELEASED, 0);
                kup = false;
            }

            if (kb[SDL_SCANCODE_DOWN] && !kdown)
            {
                send_input_chunk(KDOWN_PRESSED, 0);
                kdown = true;
            }
            else if (!kb[SDL_SCANCODE_DOWN] && kdown)
            {
                send_input_chunk(KDOWN_RELEASED, 0);
                kdown = false;
            }

            if (kb[SDL_SCANCODE_SPACE] && !kfire)
            {
                send_input_chunk(KFIRE_PRESSED, 0);
                kfire = true;
                kfire_held_start = SDL_GetTicks();
            }
            else if (!kb[SDL_SCANCODE_SPACE] && kfire)
            {
                send_input_chunk(KFIRE_RELEASED, SDL_GetTicks() - kfire_held_start);
                kfire = false;
                kfire_held_start = 0;
            }

            if (kb[SDL_SCANCODE_T])
            {
                SDL_StartTextInput();
                typing_str = "";
            }
            if (kb[SDL_SCANCODE_SLASH])
            {
                SDL_StartTextInput();
                typing_str = "/";
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
                    goto end_loop;
                    break;

                case ENET_EVENT_TYPE_RECEIVE:
                    on_receive(&moag, &enet_ev);
                    enet_packet_destroy(enet_ev.packet);
                    break;

                default:
                    break;
            }
        }

        SDL_SetRenderDrawColor(main_renderer, 0, 0, 0, 255);
        SDL_RenderClear(main_renderer);
        draw(&moag);
        char buf[256];
        sprintf(buf, "%u", get_peer()->roundTripTime);
        quick_render_string(LAND_WIDTH - 30, 0, buf);
        SDL_RenderPresent(main_renderer);
    }
end_loop:

    TTF_CloseFont(main_font);
font_load_fail:
texture_load_fail:
    SDL_DestroyTexture(tank_texture);
    SDL_DestroyTexture(turret_texture);
    SDL_DestroyTexture(bullet_texture);
    SDL_DestroyTexture(crate_texture);
    SDL_DestroyRenderer(main_renderer);
main_renderer_fail:
    SDL_DestroyWindow(main_window);
main_window_fail:
    TTF_Quit();
ttf_init_fail:
    SDL_Quit();
sdl_init_fail:
    uninit_enet();
}
