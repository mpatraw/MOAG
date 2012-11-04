
PLATFORM = linux

ifeq ($(PLATFORM),mingw32-linux)
CC = i486-mingw32-gcc
CCFLAGS = -c -Wall -pedantic -g -std=c99 -D_POSIX_C_SOURCE=199309L -DWIN32
LDFLAGS = -lmingw32 -lenet -lz -lm -lws2_32 -lwinmm
else
CC = gcc
CCFLAGS = -c -Wall -pedantic -g -std=c99 -D_POSIX_C_SOURCE=199309L
LDFLAGS = -lenet -lz -lm
endif


all: client server

ifeq ($(PLATFORM),mingw32-linux)
client: client.o sdl_aux.o enet_aux.o common.o
	$(CC) $^ -o $@ $(LDFLAGS) -lSDLmain -lSDL -lSDL_ttf

server: server.o enet_aux.o common.o
	$(CC) $^ -o $@ $(LDFLAGS) -lSDL
else
client: client.o sdl_aux.o enet_aux.o common.o
	$(CC) $^ -o $@ $(LDFLAGS) -lSDLmain -lSDL -lSDL_ttf

server: server.o enet_aux.o common.o
	$(CC) $^ -o $@ $(LDFLAGS) -lSDL
endif

test: test.o common.o
	$(CC) $^ -o $@ -lm -lz

.c.o:
	$(CC) $(CCFLAGS) $< -o $@

clean:
	rm -f *.o client server test
