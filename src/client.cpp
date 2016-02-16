
#include <deque>
#include <iostream>
#include <string>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

#include "moag.hpp"

static m::player players[g_max_players];
static m::bullet bullets[g_max_bullets];
static m::crate crate;

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

static std::unique_ptr<m::network_manager> client;
static m::land main_land;

static SDL_Texture *load_texture_from_file(const char *filename) {
    auto surface = IMG_Load(filename);
    if (!surface) {
        return nullptr;
    }
    auto texture = SDL_CreateTextureFromSurface(main_renderer, surface);
    SDL_FreeSurface(surface);
    return texture;
}

static SDL_Texture *create_string_texture(const char *text, SDL_Color c={255, 255, 255,255}) {
    auto surface = TTF_RenderText_Solid(main_font, text, c);
    if (!surface) {
        return nullptr;
    }
    auto texture = SDL_CreateTextureFromSurface(main_renderer, surface);
    SDL_FreeSurface(surface);
    return texture;
}

static void quick_render_string(int x, int y, const char *text, SDL_Color c={255, 255, 255, 255}) {
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

class message_scroller {
public:
    enum { lines = 7, expires_after = 18000 };
    message_scroller() {}
    ~message_scroller() {
        for (auto text : message_texts) {
            SDL_DestroyTexture(text);
        }
    }

    void add_message(const char *str) {
        message_expirations.push_back(SDL_GetTicks() + expires_after);
        message_texts.push_back(create_string_texture(str));
        if (message_texts.size() > lines) {
            SDL_DestroyTexture(message_texts.front());
            message_expirations.pop_front();
            message_texts.pop_front();
        }
    }

    void expire_messages() {
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

static inline void send_input_chunk(uint8_t key, uint16_t t) {
    m::message msg{new m::input_message_def{key, t}};
    client->send(msg);
}

static void draw_tank(int x, int y, int turret_angle, bool facingleft) {
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

static void draw_crate(int x, int y) {
    x = x - (crate_width / 2);
    SDL_Rect src = {x, y, crate_width, crate_height};
    SDL_RenderCopy(main_renderer, crate_texture, NULL, &src);
}

static void draw_bullets() {
    for (int i = 0; i < g_max_bullets; i++) {
        m::bullet *b = &bullets[i];
        if (b->active) {
            auto x = static_cast<int>(b->x);
            auto y = static_cast<int>(b->y);
            SDL_Rect src = {x, y, bullet_width, bullet_height};
            SDL_RenderCopy(main_renderer, bullet_texture, NULL, &src);
        }
    }
}
      
static void draw() {
    SDL_SetRenderDrawColor(main_renderer, 128, 128, 128, 255);
    auto del = dynamic_cast<land_texture const *>(main_land.get_delegate());
    SDL_RenderCopy(main_renderer, const_cast<SDL_Texture *>(del->sdl_texture()), nullptr, nullptr);

    if (crate.active) {
        draw_crate(crate.x - 4, crate.y - 8);
    }

    draw_bullets();

    for (int i = 0; i < g_max_players; i++) {
        if (players[i].connected) {
            draw_tank(players[i].tank.x,
                      players[i].tank.y,
                      players[i].tank.angle,
                      players[i].tank.facingleft);
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

    if (SDL_IsTextInputActive()) {
        SDL_SetRenderDrawColor(main_renderer, 128, 128, 128, 255);
        SDL_Rect rect = {6, 8 + 12 * message_scroller::lines, 4, 4};
        SDL_RenderDrawRect(main_renderer, &rect);

        quick_render_string(16, 4 + 12 * message_scroller::lines, typing_str.c_str());
    }
}

static void process_packet(m::packet &p) {
    auto &msg = p.get_message();
    auto type = msg.get_type();
    switch (type) {
        case m::message_type::land: {
            auto land = dynamic_cast<m::land_message_def &>(msg.get_def());
            int i = 0;
            for (int y = land.y; y < land.h + land.y; ++y) {
                for (int x = land.x; x < land.w + land.x; ++x) {
                    if (land.data[i]) {
                        main_land.set_dirt(x, y);
                    } else {
                        main_land.set_air(x, y);
                    }
                    i++;
                }
            }
            break;
        }

        case m::message_type::tank: {
            auto tank = dynamic_cast<m::tank_message_def &>(msg.get_def());
            auto id = tank.id;
            if (tank.op == m::entity_op_type::spawn) {
                players[id].connected = true;

                players[id].tank.x = tank.x;
                players[id].tank.y = tank.y;

                players[id].tank.facingleft = tank.facing == m::tank_facing_type::left;
                players[id].tank.angle = tank.angle;
            } else if (tank.op == m::entity_op_type::move) {
                players[id].tank.x = tank.x;
                players[id].tank.y = tank.y;

                players[id].tank.facingleft = tank.facing == m::tank_facing_type::left;
                players[id].tank.angle = tank.angle;
            } else if (tank.op == m::entity_op_type::kill) {
                players[id].tank.x = -1;
                players[id].tank.y = -1;
                players[id].connected = false;
            } else {
                fprintf(stderr, "Invalid TANK_CHUNK type.\n");
                exit(-1);
            }
            break;
        }

        case m::message_type::bullet: {
            auto bullet = dynamic_cast<m::bullet_message_def &>(msg.get_def());
            auto id = bullet.id;

            if (bullet.op == m::entity_op_type::spawn) {
                bullets[id].active = true;
                bullets[id].x = bullet.x;
                bullets[id].y = bullet.y;
            } else if (bullet.op == m::entity_op_type::move) {
                bullets[id].x = bullet.x;
                bullets[id].y = bullet.y;
            } else if (bullet.op == m::entity_op_type::kill) {
                bullets[id].active = false;
            } else {
                fprintf(stderr, "Invalid BULLET_CHUNK type.\n");
                exit(-1);
            }
            break;
        }

        case m::message_type::crate: {
            auto c = dynamic_cast<m::crate_message_def &>(msg.get_def());

            if (c.op == m::entity_op_type::spawn) {
                crate.active = true;
                crate.x = c.x;
                crate.y = c.y;
            } else if (c.op == m::entity_op_type::move) {
                crate.x = c.x;
                crate.y = c.y;
            } else if (c.op == m::entity_op_type::kill) {
                crate.active = false;
            } else {
                fprintf(stderr, "Invalid CRATE_CHUNK type.\n");
                exit(-1);
            }
            break;
        }

        case m::message_type::message: {
            auto m = dynamic_cast<m::message_message_def &>(msg.get_def());
            auto id = m.id;
            auto str = m.get_string();

            switch (m.op) {
                case m::message_op_type::chat: {
                    int namelen = strlen(players[id].name);
                    int linelen = namelen + str.length() + 4;
                    char *line = (char *)malloc(linelen);
                    line[0] = '<';
                    for (int i = 0; i < namelen; i++) {
                        line[i + 1] = players[id].name[i];
                    }
                    line[namelen+1] = '>';
                    line[namelen+2] = ' ';
                    for (size_t i = 0; i < str.length(); ++i) {
                        line[namelen + 3 + i] = str[i];
                    }
                    line[linelen - 1] = '\0';
                    chat_line.add_message(line);
                    break;
                }

                case m::message_op_type::name_change: {
                    if (str.length() < 1 || str.length() > 15) {
                        break;
                    }
                    for (size_t i = 0; i < str.length(); ++i) {
                        players[id].name[i] = str[i];
                    }
                    players[id].name[str.length()]='\0';
                    break;
                }

                case m::message_op_type::server_notice: {
                    chat_line.add_message(str.c_str());
                    break;
                }

                default:
                    fprintf(stderr, "Invalid SERVER_MSG_CHUNK action (%d).\n", (int)m.op);
                    exit(-1);
            }
            break;
        }

        default:
            fprintf(stderr, "Invalid CHUNK type (%d).\n", (int)msg.get_type());
            exit(-1);
    }
}

void client_main() {
    client.reset(new m::network_manager{g_host, g_port});

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::cerr << SDL_GetError() << std::endl;
        goto sdl_init_fail;
    }
    SDL_StopTextInput();

    if (TTF_Init() != 0) {
        std::cerr << SDL_GetError() << std::endl;
        goto ttf_init_fail;
    }

    main_window = SDL_CreateWindow("MOAG",
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        g_land_width, g_land_height, 0);
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
                m::message msg{new m::message_message_def{
                    m::message_op_type::client, 0, typing_str.c_str()}};
                client->send(msg);

                SDL_StopTextInput();
            }
        }

        while (true) {
            auto packet = client->recv();
            if (packet.get_type() == m::packet_type::none) {
                break;
            }
            process_packet(packet);
        }

        SDL_SetRenderDrawColor(main_renderer, 0, 0, 0, 255);
        SDL_RenderClear(main_renderer);
        draw();
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
