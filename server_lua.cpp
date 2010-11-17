#include "server_lua.h"

#include <sstream>

#include <cassert>

namespace MoagScript {
	namespace Exports {
		int putBullet( lua_State *l ) {
			using namespace MoagServer;
			using namespace MoagShallow;

			LuaInstance lua ( l );

			int y = lua.popInteger();
			int x = lua.popInteger();
			Server *server = static_cast<Server*>( lua.popUserData() );
			assert( server );

			server->prepareBullet( x, y );

			return 0;
		}

		int setTankName( lua_State *l ) {
			using namespace MoagServer;
			using namespace MoagShallow;

			LuaInstance lua ( l );
			std::string name = lua.popString();
			TankState *tank = static_cast<TankState*>( lua.popUserData() );
			assert( tank );

			tank->name = name;
			tank->dirty = tank->nameDirty = true;

			return 0;
		}

		int setTankAngle( lua_State *l ) {
			using namespace MoagServer;
			using namespace MoagShallow;

			LuaInstance lua ( l );
			int angle = lua.popInteger();
			TankState *tank = static_cast<TankState*>( lua.popUserData() );
			assert( tank );

			if( tank->angle != angle ) {
				tank->angle = angle;
				tank->dirty = true;
			}

			return 0;
		}

		int setTankPos( lua_State *l ) {
			using namespace MoagServer;
			using namespace MoagShallow;

			LuaInstance lua ( l );
			int y = lua.popInteger();
			int x = lua.popInteger();
			TankState *tank = static_cast<TankState*>( lua.popUserData() );

			if( tank->x != x || tank->y != y ) {
				tank->x = x;
				tank->y = y;
				tank->dirty = true;
			}

			return 0;
		}

		int deleteTank( lua_State *l ) {
			using namespace MoagServer;
			using namespace MoagShallow;

			LuaInstance lua ( l );
			TankState *tank = static_cast<TankState*>( lua.popUserData() );
			Server *server = static_cast<Server*>( lua.popUserData() );

			server->deleteTank( tank );

			return 0;
		}

		int makeTank( lua_State *l ) {
			using namespace MoagServer;
			using namespace MoagShallow;

			LuaInstance lua ( l );
			std::string name = lua.popString();
			int id = lua.popInteger();
			Server *server = static_cast<Server*>( lua.popUserData() );

			TankState *rv = server->makeTank( id, name );

			lua.pushValue( rv );

			return 1;
		}

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

#if 0
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
#endif
	}

	void exportAllServer( LuaInstance& lua ) {
		using namespace Exports;

		lua.exportFunction( "send_notice_to", sendNoticeTo );
		lua.exportFunction( "broadcast_notice", broadcastNotice );

#if 0
		lua.exportFunction( "set_user_nickname", setUserNickname );
		lua.exportFunction( "change_user_nickname", setUserNickname );
#endif

		lua.exportFunction( "set_terrain_at", setTerrainAt );
		lua.exportFunction( "mark_terrain_dirty", markTerrainDirty );
		lua.exportFunction( "get_terrain_at", getTerrainAt );
		lua.exportFunction( "get_terrain_width", getTerrainWidth );
		lua.exportFunction( "get_terrain_height", getTerrainHeight );
		lua.exportFunction( "terrain_fill_circle", terrainFillCircle );

		lua.exportFunction( "make_tank", makeTank );
		lua.exportFunction( "delete_tank", deleteTank );
		lua.exportFunction( "set_tank_angle", setTankAngle );
		lua.exportFunction( "set_tank_pos", setTankPos );
		lua.exportFunction( "set_tank_name", setTankName );

		lua.exportFunction( "put_bullet", putBullet );

		lua.exportFunction( "shutdown_server", shutdownServer );
	}

}
