
#include "server.h"

int main(int argc, char *argv[])
{
    if (enet_initialize() != 0) {
        fprintf(stderr, "An error occurred while initializing ENet.\n");
        return EXIT_FAILURE;
    }
    atexit(enet_deinitialize);

    ENetAddress address;
    ENetHost *server;

    address.host = ENET_HOST_ANY;
    address.port = PORT;

    server = enet_host_create(&address, MAX_CLIENTS, NUM_CHANNELS, 0, 0);
    if (!server) {
        fprintf (stderr, "An error occurred while trying to create an ENet server host.\n");
        return EXIT_FAILURE;
    }

    fprintf(stdout, "Started server\n");

    ENetEvent event;

    for (;;) {
        while (enet_host_service(server, &event, 10)) {
            switch (event.type) {
            case ENET_EVENT_TYPE_CONNECT:
                break;

            case ENET_EVENT_TYPE_DISCONNECT:
                break;

            case ENET_EVENT_TYPE_RECEIVE:
                enet_packet_destroy(event.packet);
                break;

            default:
                break;
            }
        }
    }

    enet_host_destroy(server);

    return EXIT_SUCCESS;
}
