CPP=g++
CPPFLAGS=-g
LIBS=-lSDLmain -lSDL -lSDL_net -lSDL_ttf

ENGINE_OBJS=moag_event.o moag_chunk.o moag_client.o moag_server.o moag_window.o moag_loop.o moag_net.o

all: moag_client moag_server

clean:
	rm -f *.o
	rm -f moag_client moag_server

moag_client: $(ENGINE_OBJS) client.o
	$(CPP) $(CPPFLAGS) -o $@ $^ $(LIBS)

moag_server: $(ENGINE_OBJS) server.o
	$(CPP) $(CPPFLAGS) -o $@ $^ $(LIBS)
