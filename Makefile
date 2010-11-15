CPP=g++
CPPFLAGS=-g -Wall
LIBS=-lSDLmain -lSDL -lSDL_net -lSDL_ttf
LUA_LIBS=-llua5.1

ENGINE_OBJS=moag_event.o moag_chunk.o moag_client.o moag_server.o moag_window.o moag_loop.o moag_net.o

all: moag_client moag_server2

clean:
	rm -f *.o
	rm -f moag_client moag_server

lua-test: moag_lua.o lua-test.o server_lua.o
	$(CPP) $(CPPFLAGS) -o $@ $^ $(LUA_LIBS)

moag_client: $(ENGINE_OBJS) client.o
	$(CPP) $(CPPFLAGS) -o $@ $^ $(LIBS)

moag_server: $(ENGINE_OBJS) server.o
	$(CPP) $(CPPFLAGS) -o $@ $^ $(LIBS)

moag_server2: $(ENGINE_OBJS) server2.o gamestate.o moag_lua.o server_lua.o
	$(CPP) $(CPPFLAGS) -o $@ $^ $(LIBS) $(LUA_LIBS)
