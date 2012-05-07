
#include "client.h"

int main(int argc, char *argv[])
{
    if (argc < 2) {
        printf("usage:  %s [address]\n", argv[0]);
        return EXIT_SUCCESS;
    }

    if (enet_initialize() != 0) {
        fprintf(stderr, "An error occurred while initializing ENet.\n");
        return EXIT_FAILURE;
    }
    atexit(enet_deinitialize);

    ENetHost *client = enet_host_create(NULL, MAX_CLIENTS, NUM_CHANNELS, 0, 0);
    if (!client) {
        fprintf (stderr, "An error occurred while trying to create an ENet client host.\n");
        return EXIT_FAILURE;
    }

    ENetAddress address;
    enet_address_set_host(&address, argv[1]);
    address.port = PORT;

    ENetPeer *peer = enet_host_connect(client, &address, NUM_CHANNELS, 0);
    if (!peer) {
        fprintf(stderr, "No available peers for initiating an ENet connection.\n");
        return EXIT_FAILURE;
    }

    ENetEvent event;

    if (enet_host_service(client, &event, CONNECT_TIMEOUT) == 0 ||
        event.type != ENET_EVENT_TYPE_CONNECT) {
        enet_peer_reset(peer);
        fprintf(stderr, "Connection to %s timed out.\n", argv[1]);
        return EXIT_FAILURE;
    }

    bool running = true;
    while (running) {
        while (enet_host_service(client, &event, 10)) {
            switch (event.type) {
            case ENET_EVENT_TYPE_CONNECT:
                printf("Connected %x:%u.\n",
                       event.peer->address.host,
                       event.peer->address.port);

                /* Store any relevant client information here. */
                event.peer->data = "Client information";
                break;

            case ENET_EVENT_TYPE_DISCONNECT:
                printf("Disconected.\n");

                /* Reset the peer's client information. */
                event.peer->data = NULL;

                running = false;
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

    enet_host_destroy(client);


    /*if (moag::OpenWindow(WIDTH, HEIGHT, "MOAG") == -1) {
        printf("Failed to start window\n");
        return 1;
    }

    if (moag::SetFont("Nouveau_IBM.ttf", 12) == -1) {
        printf("Failed to open font\n");
        return 1;
    }

    moag::MainLoop();

    moag::CloseWindow();*/

    return EXIT_SUCCESS;
}
