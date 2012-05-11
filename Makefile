
CC = gcc
CCFLAGS = -c -Wall -pedantic -g -std=c99 -D_POSIX_C_SOURCE=199309L
LDFLAGS = -lenet -lz -lm

all: client server

client: client.o sdl_aux.o enet_aux.o common.o
	$(CC) $(LDFLAGS) -lSDL -lSDL_ttf client.o sdl_aux.o enet_aux.o common.o -o $@

server: server.o enet_aux.o common.o
	$(CC) $(LDFLAGS) server.o enet_aux.o common.o -o $@

.c.o:
	$(CC) $(CCFLAGS) $< -o $@

clean:
	rm -f *.o client server
