#ifndef H_SERVER2
#define H_SERVER2

#define MIN(a,b) (((a)>(b))?(b):(a))
#define MAX(a,b) (((a)<(b))?(b):(a))

#ifndef UINT32_MAX
#define UINT32_MAX ((Uint32)-1)
#endif

#define MSGORIGIN_NONE -1

#define MSGTYPE_CHAT 1
#define MSGTYPE_NICKCHANGE 2
#define MSGTYPE_NOTICE 3


#include <pthread.h>

#include "moag_server.h"

#include <vector>
#include <string>

#include "moag_lua.h"

#include "moag_shallow.h"

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
            int id;

			bool marked;

			MoagScript::LuaAutoReference keytable;
			MoagScript::LuaAutoReference sob;


		public:
			MoagUser( Server&, moag::Connection );
			~MoagUser(void);

            void markForDisconnectionError(const std::string&);

			moag::Connection getConnection(void);
			void markForDisconnection(void);
			bool markedForDisconnection(void);

            void handleMessage(void);
            void handleActivity(void);
	};

	class Server {
		private:
			MoagTicker ticker;
			MoagGreeter greeter;

            int tickCount;

			bool doQuit;

			typedef std::vector< MoagUser* > userlist_t;
			userlist_t users;

			MoagScript::LuaInstance& lua;

			typedef std::vector< MoagShallow::TankState* > tanklist_t;
			MoagShallow::TerrainState terrain;
			MoagShallow::CrateState crate;
			tanklist_t tanks;
			MoagShallow::BulletQueueManager bulletQ;

			pthread_mutex_t mutex;

		public:
			Server(MoagScript::LuaInstance&, const int, const int, const int, const int);
			~Server(void);

			MoagShallow::TerrainState& getTerrainState(void);
			MoagShallow::CrateState& getCrateState(void);

			void enqueueAll(void);
			void enqueueDirtyForBroadcast(void);

			void acquireMutex(void);
			void releaseMutex(void);

			MoagScript::LuaInstance& getLuaInstance(void) { return lua; }

			void run(const int);

			int userConnected( moag::Connection );

			void sendChunksTo( MoagUser* );
			void broadcastChunks(void);
			void activitySweep(void);
			void disconnectSweep(void);

			void prepareBullet(int,int);

			MoagShallow::TankState* makeTank(int,std::string);
			void deleteTank(MoagShallow::TankState*);

			void broadcastNotice( const std::string& );
			void sendNoticeTo( const std::string&, MoagUser* );

            void didTick(void);
			void stepGame(void);

			void shutdown(void);
	};

};

#endif
