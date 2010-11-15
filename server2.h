#ifndef H_SERVER2
#define H_SERVER2

#define MIN(a,b) (((a)>(b))?(b):(a))
#define MAX(a,b) (((a)<(b))?(b):(a))

#include <pthread.h>

#include "moag_server.h"

#include <vector>
#include <string>

#include "gamestate.h"

#include "moag_lua.h"

enum {
    C2SM_PRESS_LEFT = 1,
    C2SM_RELEASE_LEFT,
    C2SM_PRESS_RIGHT,
    C2SM_RELEASE_RIGHT,
    C2SM_PRESS_UP,
    C2SM_RELEASE_UP,
    C2SM_PRESS_DOWN,
    C2SM_RELEASE_DOWN,
    C2SM_PRESS_FIRE,
    C2SM_RELEASE_FIRE,
    C2SM_CHAT_MESSAGE
};

typedef enum {
	INPUT_LEFT = 1,
	INPUT_RIGHT,
	INPUT_DOWN,
	INPUT_UP,
	INPUT_FIRE
} input_key_t;

enum {
	LAND_CHUNK = 1,
	TANK_CHUNK,
	BULLET_CHUNK,
	MSG_CHUNK,
	CRATE_CHUNK
};

namespace MoagServer {
	class Server;

	class MoagTicker : public moag::CallbackObject {
		private:
			Server& server;
		public:
			MoagTicker( Server& );

			int operator()(moag::Connection);
	};

	class MoagGreeter : public moag::CallbackObject {
		private:
			Server& server;
		public:
			MoagGreeter( Server& );

			int operator()(moag::Connection);
	};

	class MoagUser {
		private:
            static int nextUserId;

            Server& server;

			moag::Connection conn;
            std::string name;
            int id;

			bool marked;

			bool keypressLeft,
                 keypressRight,
                 keypressDown,
                 keypressUp,
                 keypressFire;

			Tank *tank;

			MoagScript::LuaAutoReference sob;

		public:
			MoagUser( Server&, moag::Connection, Tank* );
			~MoagUser(void);

            void markForDisconnectionError(const std::string&);

			moag::Connection getConnection(void);
			void markForDisconnection(void);
			bool markedForDisconnection(void);
            int getTankId(void) const;
            int getId(void) const;
            const std::string& getName(void) const;

            void handleCommand(char *);
            void changeNickname( const std::string& );
            void setNickname( const std::string& );

            void handleMessage(void);
            void handleActivity(void);

			bool getKey( input_key_t ) const;
	};

	class Server {
		private:
			MoagTicker ticker;
			MoagGreeter greeter;

			GameState state;

            int tickCount;

			bool doQuit;

			typedef std::vector< MoagUser* > userlist_t;
			userlist_t users;

			MoagScript::LuaInstance& lua;

			pthread_mutex_t mutex;

		public:
			Server(MoagScript::LuaInstance&, const int, const int, const int, const int);
			~Server(void);

			void acquireMutex(void);
			void releaseMutex(void);

			MoagScript::LuaInstance& getLuaInstance(void) { return lua; }

			void run(const int);

            void broadcastName( MoagUser* );

			int userConnected( moag::Connection );

			void sendChunksTo( MoagUser* );
			void broadcastChunks(void);
			void activitySweep(void);
			void disconnectSweep(void);

			void broadcastNotice( const std::string& );
			void sendNoticeTo( const std::string&, MoagUser* );

			void broadcastChatMessage( MoagUser*, const std::string& );

            void didTick(void);
			void stepGame(void);

			void shutdown(void);
	};

};

#endif
