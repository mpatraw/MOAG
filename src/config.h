
#ifndef CONFIG_H
#define CONFIG_H

// Defined in main.cpp.
extern bool g_is_server;
extern const char *g_host;
extern int g_port;

const int g_max_players = 8;
const int g_connect_timeout = 10000;
const int g_number_of_channels = 2;

#endif

