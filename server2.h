#ifndef H_SERVER2
#define H_SERVER2

#include "moag_server.h"

#include <vector>
#include <string>

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
            std::string name;

			bool marked;

			bool keypressLeft,
                 keypressRight,
                 keypressDown,
                 keypressUp,
                 keypressFire;

		public:
			MoagUser( Server&, moag::Connection );

			moag::Connection getConnection(void) { return conn; }

            void markForDisconnectionError(const std::string&);
			void markForDisconnection(void) { marked = true; }
			bool markedForDisconnection(void) { return marked; }

            int getId(void) const { return id; }
            const std::string& getName(void) const { return name; }

            void handleCommand(char *);
            void changeNickname( const std::string& );


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

		public:
			Server(const int, const int);
			~Server(void);

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
	};

};

#endif
