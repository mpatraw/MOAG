#include "server_lua.h"

#include <sstream>

namespace MoagScript {
	namespace Exports {
		int getTerrainHeight( lua_State *l ) {
			using namespace MoagServer;

			LuaInstance lua ( l );
			Server *server = static_cast<Server*>( lua.popUserData() );

			lua.pushValue( server->getTerrainState().getHeight() );

			return 1;
		}

		int getTerrainWidth( lua_State *l ) {
			using namespace MoagServer;

			LuaInstance lua ( l );
			Server *server = static_cast<Server*>( lua.popUserData() );

			lua.pushValue( server->getTerrainState().getWidth() );

			return 1;
		}

		int getTerrainAt( lua_State *l ) {
			using namespace MoagServer;
			using namespace MoagShallow;

			LuaInstance lua ( l );

			int y = lua.popInteger();
			int x = lua.popInteger();
			Server *server = static_cast<Server*>( lua.popUserData() );

			TerrainState& ts = server->getTerrainState();

			lua.pushValue( ts.getTerrain( x, y ) );

			return 1;
		}

		int terrainFillCircle( lua_State *l ) {
			using namespace MoagServer;
			using namespace MoagShallow;

			LuaInstance lua ( l );

			int value = lua.popInteger();
			int r = lua.popInteger();
			int y = lua.popInteger();
			int x = lua.popInteger();
			Server *server = static_cast<Server*>( lua.popUserData() );

			TerrainState& ts = server->getTerrainState();

			for(int i=-r;i<=r;i++) for(int j=-r;j<=r;j++) {
				if( (i*i)+(j*j) < r*r ) {
					ts.setTerrain( x+i, y+j, value );
				}
			}
			ts.markDirty( x-r, y-r, 2*r+1, 2*r+1 );

			return 0;
		}


		int setTerrainAt( lua_State *l ) {
			using namespace MoagServer;
			using namespace MoagShallow;

			LuaInstance lua ( l );

			int value = lua.popInteger();
			int y = lua.popInteger();
			int x = lua.popInteger();
			Server *server = static_cast<Server*>( lua.popUserData() );

			TerrainState& ts = server->getTerrainState();

			ts.setTerrain( x, y, value );

			return 0;
		}

		int markTerrainDirty( lua_State *l ) {
			using namespace MoagServer;
			using namespace MoagShallow;

			LuaInstance lua ( l );

			int h = lua.popInteger();
			int w = lua.popInteger();
			int y = lua.popInteger();
			int x = lua.popInteger();
			Server *server = static_cast<Server*>( lua.popUserData() );

			TerrainState& ts = server->getTerrainState();

			ts.markDirty( x, y, w, h );

			return 0;
		}

		int shutdownServer( lua_State* l ) {
			using namespace MoagServer;
			
			LuaInstance lua ( l );

			Server *server = static_cast<Server*>( lua.popUserData() );

			server->shutdown();

			return 0;
		}

		int broadcastNotice( lua_State* l ) {
			using namespace MoagServer;
			
			LuaInstance lua ( l );

			std::string message = lua.popString();
			Server *server = static_cast<Server*>( lua.popUserData() );

			server->broadcastNotice( message );

			return 0;
		}

		int sendNoticeTo( lua_State* l ) {
			using namespace MoagServer;
			
			LuaInstance lua ( l );

			std::string message = lua.popString();
			MoagUser *user = static_cast<MoagUser*>( lua.popUserData() );
			Server *server = static_cast<Server*>( lua.popUserData() );

			server->sendNoticeTo( message, user );
			
			return 0;
		}

		int changeUserNickname( lua_State* l ) {
			using namespace MoagServer;
			
			LuaInstance lua ( l );

			std::string nick = lua.popString();
			MoagUser *user = static_cast<MoagUser*>( lua.popUserData() );

			user->changeNickname( nick );

			return 0;
		}

		int setUserNickname( lua_State* l ) {
			using namespace MoagServer;
			
			LuaInstance lua ( l );

			std::string nick = lua.popString();
			MoagUser *user = static_cast<MoagUser*>( lua.popUserData() );

			user->setNickname( nick );

			return 0;
		}
	}

	void exportAllServer( LuaInstance& lua ) {
		using namespace Exports;

		lua.exportFunction( "send_notice_to", sendNoticeTo );
		lua.exportFunction( "broadcast_notice", broadcastNotice );

		lua.exportFunction( "set_user_nickname", setUserNickname );
		lua.exportFunction( "change_user_nickname", setUserNickname );

		lua.exportFunction( "set_terrain_at", setTerrainAt );
		lua.exportFunction( "mark_terrain_dirty", markTerrainDirty );
		lua.exportFunction( "get_terrain_at", getTerrainAt );
		lua.exportFunction( "get_terrain_width", getTerrainWidth );
		lua.exportFunction( "get_terrain_height", getTerrainHeight );
		lua.exportFunction( "terrain_fill_circle", terrainFillCircle );

		lua.exportFunction( "shutdown_server", shutdownServer );
	}

}
