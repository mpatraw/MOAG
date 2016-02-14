
#ifndef CONFIG_HPP
#define CONFIG_HPP

// Defined in main.cpp.
extern bool g_is_client;
extern bool g_is_server;
extern const char *g_host;
extern int g_port;

const int g_land_width = 800;
const int g_land_height = 600;

const int g_max_players = 8;
const int g_connect_timeout = 10000;
const int g_number_of_channels = 2;

// Server constants.
const int g_gravity = 1;
const int g_bouncer_bounces = 11;
const int g_tunnel_tunnelings = 20;
const int g_shotgun_pellets = 6;
const int g_respawn_time = 40;

const int g_max_bullets = 64;
const int g_max_name_len = 16;

#endif

