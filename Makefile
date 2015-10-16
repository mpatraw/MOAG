
LDFLAGS=-lm -lenet -lz
CFLAGS=-Wall -pedantic -g -std=c99

CLIENT_SRC=client.c common.c sdl_aux.c
CLIENT_OBJ=$(CLIENT_SRC:.c=.o)
CLIENT_LD=$(LDFLAGS) -lSDL_ttf `sdl-config --libs`

SERVER_SRC=server.c common.c
SERVER_OBJ=$(SERVER_SRC:.c=.o)
SERVER_LD=$(LDFLAGS)

.PHONY: all clean

all: client server

.c.o:
	$(CC) -c $< $(CFLAGS) -o $@

client: $(CLIENT_OBJ)
	$(CC) $(CLIENT_OBJ) $(CLIENT_LD) -o $@

server: $(SERVER_OBJ)
	$(CC) $(SERVER_OBJ) $(SERVER_LD) -o $@

clean:
	rm -rf $(CLIENT_OBJ) $(SERVER_OBJ) client server
