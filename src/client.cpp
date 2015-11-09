
#include <deque>
#include <iostream>
#include <string>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

#include "moag.hpp"

struct input {
    uint8_t key;
    uint16_t ms;
    friend m::packet &operator <<(m::packet &p, const input &i) {
        p << static_cast<uint8_t>(INPUT_CHUNK) << i.key << i.ms;
        return p;
    }
};

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

class land_texture final : public m::land_delegate {
public:
    land_texture(SDL_Renderer *renderer) : renderer{renderer} {
        texture = SDL_CreateTexture(
                renderer,
                SDL_PIXELFORMAT_RGBA8888,
                SDL_TEXTUREACCESS_TARGET,
                g_land_width, g_land_height);
    }
    land_texture(const land_texture &) = delete;
    ~land_texture() {
        if (texture) {
            SDL_DestroyTexture(texture);
        }
    }

    void set_dirt(int x, int y) {
        SDL_SetRenderTarget(renderer, texture);
        SDL_SetRenderDrawColor(renderer, 128, 128, 128, 255);
        SDL_RenderDrawPoint(renderer, x, y);
        SDL_SetRenderTarget(renderer, nullptr);
    }

    void set_air(int x, int y) {
        SDL_SetRenderTarget(renderer, texture);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderDrawPoint(renderer, x, y);
        SDL_SetRenderTarget(renderer, nullptr);
    }

    const SDL_Texture *sdl_texture() const { return texture; }

private:
    SDL_Renderer *renderer;
    SDL_Texture *texture;
};

static std::unique_ptr<m::client> client;
static m::land main_land;

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
    auto texture = create_string_texture(text, c);
    if (!texture) {
        return;
    }
    int w, h;
    SDL_QueryTexture(texture, NULL, NULL, &w, &h);
    SDL_Rect src = {x, y, w, h};
    SDL_RenderCopy(main_renderer, texture, NULL, &src);
    SDL_DestroyTexture(texture);
}

class message_scroller
{
public:
    enum { lines = 7, expires_after = 18000 };
    message_scroller() {}
    ~message_scroller()
    {
        for (auto text : message_texts) {
            SDL_DestroyTexture(text);
        }
    }

    void add_message(const char *str)
    {
        message_expirations.push_back(SDL_GetTicks() + expires_after);
        message_texts.push_back(create_string_texture(str));
        if (message_texts.size() > lines) {
            SDL_DestroyTexture(message_texts.front());
            message_expirations.pop_front();
            message_texts.pop_front();
        }
    }

    void expire_messages()
    {
        while (!message_expirations.empty() && message_expirations.front() < SDL_GetTicks()) {
            SDL_DestroyTexture(message_texts.front());
            message_expirations.pop_front();
            message_texts.pop_front();
        }
    }

    typedef std::deque<SDL_Texture *>::const_iterator const_iterator;

    const_iterator begin() const { return message_texts.begin(); }
    const_iterator end() const { return message_texts.end(); }

private:
    std::deque<uint32_t> message_expirations;
    std::deque<SDL_Texture *> message_texts;
};

static message_scroller chat_line;
static std::string typing_str;

static inline void send_input_chunk(uint8_t key, uint16_t t)
{
    m::packet p;
    p << input{key, t};
    client->send(p);
}

static void draw_tank(int x, int y, int turret_angle, bool facingleft)
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

static void draw_crate(int x, int y)
{
    x = x - (crate_width / 2);
    SDL_Rect src = {x, y, crate_width, crate_height};
    SDL_RenderCopy(main_renderer, crate_texture, NULL, &src);
}

static void draw_bullets(m::moag *m) {
    for (int i = 0; i < g_max_bullets; i++) {
        m::bullet *b = &m->bullets[i];
        if (b->active) {
            SDL_Rect src = {b->x / 10, b->y / 10, bullet_width, bullet_height};
            SDL_RenderCopy(main_renderer, bullet_texture, NULL, &src);
        }
    }
}
      
static void draw(m::moag *m)
{
    SDL_SetRenderDrawColor(main_renderer, 128, 128, 128, 255);
    auto del = dynamic_cast<land_texture const *>(main_land.get_delegate());
    SDL_RenderCopy(main_renderer, const_cast<SDL_Texture *>(del->sdl_texture()), nullptr, nullptr);

    if (m->crate.active) {
        draw_crate((m->crate.x - 4) / 10, (m->crate.y - 8) / 10);
    }

    draw_bullets(m);

    for (int i = 0; i < g_max_players; i++)
    {
        if (m->players[i].connected)
        {
            draw_tank(m->players[i].tank.x / 10,
                      m->players[i].tank.y / 10,
                      m->players[i].tank.angle,
                      m->players[i].tank.facingleft);
        }
    }

    chat_line.expire_messages();
    int i = 0;
    for (auto text : chat_line) {
        int w, h;
        SDL_QueryTexture(text, NULL, NULL, &w, &h);
        SDL_Rect src = {4, 4 + 12 * i, w, h};
        SDL_RenderCopy(main_renderer, text, NULL, &src);
        i++;
    }

    if (SDL_IsTextInputActive())
    {
        SDL_SetRenderDrawColor(main_renderer, 128, 128, 128, 255);
        SDL_Rect rect = {6, 8 + 12 * message_scroller::lines, 4, 4};
        SDL_RenderDrawRect(main_renderer, &rect);

        quick_render_string(16, 4 + 12 * message_scroller::lines, typing_str.c_str());
    }
}

static void process_packet(m::moag *m, m::packet &p)
{
    uint8_t type;
    p >> type;
    switch (type) {
        case LAND_CHUNK: {
            uint16_t lx, ly, lw, lh;
            p >> lx >> ly >> lw >> lh;
            for (int y = ly; y < lh + ly; ++y) {
                for (int x = lx; x < lw + lx; ++x) {
                    uint8_t d;
                    p >> d;
                    if (d) {
                        main_land.set_dirt(x, y);
                    } else {
                        main_land.set_air(x, y);
                    }
                }
            }
            break;
        }

        case PACKED_LAND_CHUNK: {
            uint16_t lx, ly, lw, lh;
            p >> lx >> ly >> lw >> lh;
            size_t datalen = 0;
            uint8_t *data = rldecode(p.remaining_data(), p.remaining_size(), &datalen);

            size_t i = 0;
            for (int y = ly; y < lh + ly; ++y) {
                if (i >= datalen) {
                    break;
                }
                for (int x = lx; x < lw + lx; ++x) {
                    if (data[i]) {
                        main_land.set_dirt(x, y);
                    } else {
                        main_land.set_air(x, y);
                    }
                    i++;
                    if (i >= datalen) {
                        break;
                    }
                }
            }
            free(data);
            break;
        }

        case TANK_CHUNK: {
            uint8_t action, id, cangle;
            uint16_t tx, ty;
            p >> action >> id >> tx >> ty >> cangle;
            auto angle = static_cast<char>(cangle); 
            
            assert(id >= 0 && id <= g_max_players);

            if (action == SPAWN) {
                m->players[id].connected = true;

                m->players[id].tank.x = tx;
                m->players[id].tank.y = ty;

                m->players[id].tank.facingleft = false;
                if (angle < 0){
                    angle = -angle;
                    m->players[id].tank.facingleft = true;
                }
                m->players[id].tank.angle = angle;
            } else if (action == MOVE) {
                m->players[id].tank.x = tx;
                m->players[id].tank.y = ty;

                m->players[id].tank.facingleft = false;
                if (angle < 0){
                    angle = -angle;
                    m->players[id].tank.facingleft = true;
                }
                m->players[id].tank.angle = angle;
            } else if (action == KILL) {
                m->players[id].tank.x = -1;
                m->players[id].tank.y = -1;
                m->players[id].connected = false;
            } else {
                fprintf(stderr, "Invalid TANK_CHUNK type.\n");
                exit(-1);
            }
            break;
        }

        case BULLET_CHUNK: {
            uint8_t action, id;
            uint16_t bx, by;
            p >> action >> id >> bx >> by;

            if (action == SPAWN) {
                m->bullets[id].active = true;
                m->bullets[id].x = bx;
                m->bullets[id].y = by;
            } else if (action == MOVE) {
                m->bullets[id].x = bx;
                m->bullets[id].y = by;
            } else if (action == KILL) {
                m->bullets[id].active = false;
            } else {
                fprintf(stderr, "Invalid BULLET_CHUNK type.\n");
                exit(-1);
            }
            break;
        }

        case CRATE_CHUNK: {
            uint8_t action;
            uint16_t cx, cy;
            p >> action >> cx >> cy;

            if (action == SPAWN) {
                m->crate.active = true;
                m->crate.x = cx;
                m->crate.y = cy;
            } else if (action == MOVE) {
                m->crate.x = cx;
                m->crate.y = cy;
            } else if (action == KILL) {
                m->crate.active = false;
            } else {
                fprintf(stderr, "Invalid CRATE_CHUNK type.\n");
                exit(-1);
            }
            break;
        }

        case SERVER_MSG_CHUNK: {
            uint8_t id, action;
            p >> id >> action;

            auto data = p.remaining_data();
            auto len = p.remaining_size();

            switch (action) {
                case CHAT: {
                    int namelen = strlen(m->players[id].name);
                    int linelen = namelen + len + 4;
                    char *line = (char *)malloc(linelen);
                    line[0] = '<';
					for (int i = 0; i < namelen; i++) {
						line[i + 1] = m->players[id].name[i];
					}
                    line[namelen+1] = '>';
                    line[namelen+2] = ' ';
					for (size_t i = 0; i < len; ++i) {
						line[namelen + 3 + i] = data[i];
					}
                    line[linelen - 1] = '\0';
                    chat_line.add_message(line);
                    break;
                }

                case NAME_CHANGE: {
					if (len < 1 || len > 15) {
						break;
					}
					for (size_t i = 0; i < len; ++i) {
						m->players[id].name[i] = data[i];
					}
                    m->players[id].name[len]='\0';
                    break;
                }

                case SERVER_NOTICE: {
                    chat_line.add_message(reinterpret_cast<const char *>(data));
                    break;
                }

                default:
                    fprintf(stderr, "Invalid SERVER_MSG_CHUNK action (%d).\n", action);
                    exit(-1);
            }
            break;
        }

        default:
            fprintf(stderr, "Invalid CHUNK type (%d).\n", type);
            exit(-1);
    }
}

void client_main(void)
{
    client.reset(new m::client);

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::cerr << SDL_GetError() << std::endl;
        goto sdl_init_fail;
    }
    SDL_StopTextInput();

    if (TTF_Init() != 0) {
        std::cerr << SDL_GetError() << std::endl;
        goto ttf_init_fail;
    }

    main_window = SDL_CreateWindow("MOAG", -1, -1, g_land_width, g_land_height, 0);
    if (!main_window) {
        std::cerr << SDL_GetError() << std::endl;
        goto main_window_fail;
    }

    main_renderer = SDL_CreateRenderer(main_window, -1,
            SDL_RENDERER_SOFTWARE | SDL_RENDERER_TARGETTEXTURE);
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

    m::moag moag;
    memset(&moag, 0, sizeof(moag));

    main_land.set_delegate(new land_texture(main_renderer));

    uint32_t kfire_held_start;
    kfire_held_start = 0;
    while (true) {
        SDL_Event ev;
        while (SDL_PollEvent(&ev)) {
            switch (ev.type) {
            case SDL_KEYDOWN:
                if (ev.key.keysym.sym == SDLK_LEFT) {
                    send_input_chunk(KLEFT_PRESSED, 0);
                }

                if (ev.key.keysym.sym == SDLK_RIGHT) {
                    send_input_chunk(KRIGHT_PRESSED, 0);
                }

                if (ev.key.keysym.sym == SDLK_UP) {
                    send_input_chunk(KUP_PRESSED, 0);
                }

                if (ev.key.keysym.sym == SDLK_DOWN) {
                    send_input_chunk(KDOWN_PRESSED, 0);
                }

                if (ev.key.keysym.sym == SDLK_SPACE && kfire_held_start == 0) {
                    send_input_chunk(KFIRE_PRESSED, 0);
                    kfire_held_start = SDL_GetTicks();
                }

                if (ev.key.keysym.sym == SDLK_t) {
                    SDL_StartTextInput();
                    typing_str = "";
                }

                if (ev.key.keysym.sym == SDLK_SLASH) {
                    SDL_StartTextInput();
                    typing_str = "/";
                }

                if (ev.key.keysym.sym == SDLK_ESCAPE) {
                    goto end_loop;
                }
                break;

            case SDL_KEYUP:
                if (ev.key.keysym.sym == SDLK_LEFT) {
                    send_input_chunk(KLEFT_RELEASED, 0);
                }

                if (ev.key.keysym.sym == SDLK_RIGHT) {
                    send_input_chunk(KRIGHT_RELEASED, 0);
                }

                if (ev.key.keysym.sym == SDLK_UP) {
                    send_input_chunk(KUP_RELEASED, 0);
                }

                if (ev.key.keysym.sym == SDLK_DOWN) {
                    send_input_chunk(KDOWN_RELEASED, 0);
                }

                if (ev.key.keysym.sym == SDLK_SPACE) {
                    send_input_chunk(KFIRE_RELEASED, SDL_GetTicks() - kfire_held_start);
                    kfire_held_start = 0;
                }
                break;

            case SDL_TEXTINPUT:
                typing_str += ev.text.text;
                break;

            case SDL_QUIT:
                goto end_loop;
                break;
            } 
        }

        auto kb = SDL_GetKeyboardState(NULL);

        if (SDL_IsTextInputActive()) {
            if (kb[SDL_SCANCODE_ESCAPE]
                || kb[SDL_SCANCODE_LEFT]
                || kb[SDL_SCANCODE_RIGHT]
                || kb[SDL_SCANCODE_UP]
                || kb[SDL_SCANCODE_DOWN]) {
                SDL_StopTextInput();
            } else if (kb[SDL_SCANCODE_RETURN]) {
                m::packet p;
                p << static_cast<uint8_t>(CLIENT_MSG_CHUNK);
                p << typing_str.c_str();
                client->send(p);

                SDL_StopTextInput();
            }
        }

        while (true) {
            auto packet = client->recv();
            if (packet.empty()) {
                break;
            }
            process_packet(&moag, packet);
        }

        SDL_SetRenderDrawColor(main_renderer, 0, 0, 0, 255);
        SDL_RenderClear(main_renderer);
        draw(&moag);
        quick_render_string(g_land_width - 30, 0, std::to_string(client->rtt()).c_str());
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
    return;
}
