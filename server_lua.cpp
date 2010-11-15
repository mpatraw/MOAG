#include "server_lua.h"

#include <sstream>

namespace MoagScript {
	namespace Exports {
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

		lua.exportFunction( "shutdown_server", shutdownServer );
	}

}
