#include "server_lua.h"

#include <sstream>

namespace MoagScript {
	namespace Exports {
		int shutdownServer( lua_State* l ) {
			using namespace MoagServer;
			
			LuaInstance lua ( l );

			Server *server = static_cast<Server*>( lua.popUserData() );

			server->shutdown();

			lua.pushValue( 42 );
			return 1;
		}

		int broadcastNotice( lua_State* l ) {
			using namespace MoagServer;
			
			LuaInstance lua ( l );

			std::string message = lua.popString();
			Server *server = static_cast<Server*>( lua.popUserData() );

			server->broadcastNotice( message );

			lua.pushValue( 42 );
			return 1;
		}

		int sendNoticeTo( lua_State* l ) {
			using namespace MoagServer;
			
			LuaInstance lua ( l );

			std::string message = lua.popString();
			MoagUser *user = static_cast<MoagUser*>( lua.popUserData() );
			Server *server = static_cast<Server*>( lua.popUserData() );

			server->sendNoticeTo( message, user );

			lua.pushValue( 42 );
			return 1;
		}

		int changeUserNickname( lua_State* l ) {
			using namespace MoagServer;
			
			LuaInstance lua ( l );

			std::string nick = lua.popString();
			MoagUser *user = static_cast<MoagUser*>( lua.popUserData() );

			user->changeNickname( nick );

			lua.pushValue( 42 );

			return 1;
		}

		int setUserNickname( lua_State* l ) {
			using namespace MoagServer;
			
			LuaInstance lua ( l );

			std::string nick = lua.popString();
			MoagUser *user = static_cast<MoagUser*>( lua.popUserData() );

			user->setNickname( nick );

			lua.pushValue( 42 );

			return 1;
		}

		int testFunction( lua_State* l ) {
			LuaInstance lua ( l );
			
				// Popping must be done in reverse order.
			std::string beta = lua.popString();
			std::string alpha = lua.popString();

			std::ostringstream oss;
			oss << alpha << "-" << beta;
			lua.pushValue( oss.str() );

			return 1;
		}
	}

	void exportAllServer( LuaInstance& lua ) {
		using namespace Exports;

		lua.exportFunction( "send_notice_to", sendNoticeTo );
		lua.exportFunction( "broadcast_notice", broadcastNotice );

		lua.exportFunction( "set_user_nickname", setUserNickname );
		lua.exportFunction( "change_user_nickname", setUserNickname );

		lua.exportFunction( "shutdown_server", shutdownServer );

		lua.exportFunction( "test_c_function", testFunction );
	}

}
