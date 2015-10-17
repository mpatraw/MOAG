
LDFLAGS=-lm -lenet -lz
CFLAGS=-Wall -pedantic -g -std=c99

SRC=$(wildcard src/*.c)
OBJ=$(SRC:.c=.o)

CLIENT_OBJ=$(filter-out src/server.o,$(OBJ))
CLIENT_LD=$(LDFLAGS) -lSDL_ttf `sdl-config --libs`
SERVER_OBJ=$(filter-out src/client.o src/sdl_aux.o,$(OBJ))
SERVER_LD=$(LDFLAGS) `sdl-config --libs`

.PHONY: all clean

all: bin/client bin/server

.c.o:
	$(CC) -c $< $(CFLAGS) -o $@

bin/client: $(OBJ)
	$(CC) $(CLIENT_OBJ) $(CLIENT_LD) -o $@

bin/server: $(OBJ)
	$(CC) $(SERVER_OBJ) $(SERVER_LD) -o $@

clean:
	rm -rf $(OBJ) bin/client bin/server
