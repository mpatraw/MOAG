
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
                printf("A new client connected from %x:%u.\n",
                       event.peer->address.host,
                       event.peer->address.port);

                /* Store any relevant client information here. */
                event.peer->data = "Client information";
                break;

            case ENET_EVENT_TYPE_DISCONNECT:
                printf("%s disconected.\n", (char *)event.peer->data);

                /* Reset the peer's client information. */
                event.peer->data = NULL;
                break;

            case ENET_EVENT_TYPE_RECEIVE:
                printf("A packet of length %u containing %s was received from %s on channel %u.\n",
                       event.packet->dataLength,
                       (char *)event.packet->data,
                       (char *)event.peer->data,
                       event.channelID);

                /* Clean up the packet now that we're done using it. */
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
