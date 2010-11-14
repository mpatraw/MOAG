#include "server2.h"

#include "moag_chunk.h"
#include "moag.h"

#include <stdexcept>

#include <iostream>

#include <sstream>

#define MOAG_FLUSH_CHUNKS() moag::SendChunk(0, -1, 1)

#define MIN(a,b) (((a)>(b))?(b):(a))
#define MAX(a,b) (((a)<(b))?(b):(a))

#ifndef UINT32_MAX
#define UINT32_MAX ((Uint32)-1)
#endif

#define MSGORIGIN_NONE -1

#define MSGTYPE_CHAT 1
#define MSGTYPE_NICKCHANGE 2
#define MSGTYPE_NOTICE 3

#define MAX_CLIENTS 8


enum {
	LAND_CHUNK = 1,
	TANK_CHUNK,
	BULLET_CHUNK,
	MSG_CHUNK,
	CRATE_CHUNK
};

namespace MoagServer {
	int MoagUser::nextUserId = 1;

	MoagTicker::MoagTicker( Server& server ) :
		server ( server )
	{
	}

	MoagGreeter::MoagGreeter( Server& server ) :
		server ( server )
	{
	}

	MoagUser::MoagUser( Server& server, moag::Connection conn ) :
		server( server ),
		conn( conn ),
		name( "AfghanCap" ),
		id( nextUserId++ ),
		marked( false )
	{
	}

	int MoagTicker::operator()(moag::Connection con) {
		server.activitySweep();
		server.disconnectSweep();
		server.didTick();
		return 0;
	}

	int MoagGreeter::operator()(moag::Connection con) {
		using namespace std;
		return server.userConnected( con );
	}

	Uint32 SdlServerTickCallback( Uint32 delay, void *param ) {
		static bool first_tick = true;
		static Uint32 last_tick;
		static int compensation = 0;
		intptr_t target = reinterpret_cast<intptr_t>(param);
		int rv = target;
		using namespace std;

		Uint32 now = SDL_GetTicks();
		if( !first_tick ) {
			int real = now - last_tick;
			int diff = real - (target - compensation);
			rv -= diff;
			rv = MAX( 1, rv );
			compensation = diff;
		} else {
			first_tick = false;
		}
		last_tick = now;

		moag::ServerTick();

		return rv;
	}

	void Server::run(const int freq) {
		double ms = 1000.0 / freq;
		int ims = (int)(0.5 + ms);

		double efrm = 1000.0 / (double) ims;

		using namespace std;

		cout << "Running server with delay: " << ims << " milliseconds (expected frame rate: " << efrm << ")" << endl;

		intptr_t target = ims;

		SDL_TimerID id = SDL_AddTimer( ims, SdlServerTickCallback, reinterpret_cast<void*>(target) );
		while( !doQuit ) {
			SDL_Delay( 1000 );
			int ticks = tickCount;
			tickCount = 0;
			cerr << "debug: ticked " << ticks << " times this second" << endl;
		}
		SDL_RemoveTimer( id );
	}

	Server::Server(const int port, const int maxClients) :
		ticker( MoagTicker( *this ) ),
		greeter( MoagGreeter( *this ) ),
		tickCount( 0 ),
		doQuit( false ),
		users ()
	{
		if( moag::OpenServer( port, maxClients ) == -1 ) {
			throw std::runtime_error( "failed to open server -- port bound?" );
		}

		moag::SetServerCallback( &ticker, moag::CB_SERVER_UPDATE );
		moag::SetServerCallback( &greeter, moag::CB_CLIENT_CONNECT );
	}

	Server::~Server(void) {
		for( userlist_t::iterator i = users.begin(); i != users.end(); ) {
			MoagUser *user = *i;
			delete user;
			i = users.erase( i );
		}
		moag::CloseServer();
	}

	int Server::userConnected(moag::Connection conn) {
		MoagUser *user = new MoagUser( *this, conn );

		broadcastNotice( "A challenger appears!" );
		sendNoticeTo( "Welcome to MOAG!", user );

		users.push_back( user );

		broadcastName( user );

		return 0;
	}

	void Server::sendChunksTo(MoagUser *user) {
		/* This is not in MoagUser since the chunk system is global --
		   I don't want to give the false impression that we can do
		   multiple sends in parallel. */
		int rv = moag::SendChunk( user->getConnection(), moag::SEND_ALL, 1 );
		if( rv == -1 ) {
			user->markForDisconnection();
		}
	}

	void MoagUser::markForDisconnectionError( const std::string& reason ) {
		using namespace std;
		cerr << "disconnecting user " << conn << " for reason: " << reason << endl;
		markForDisconnection();
	}

	void Server::broadcastChunks(void) {
		for(userlist_t::iterator i = users.begin(); i != users.end(); i++) {
			int rv = moag::SendChunk( (*i)->getConnection(), moag::SEND_ALL, 0 );
			if( rv == -1 ) {
				(*i)->markForDisconnection();
			}
		}
		MOAG_FLUSH_CHUNKS();
	}

	void MoagUser::changeNickname( const std::string& nickname ) {
		using namespace std;
		std::string oldNickname = name;
		name = nickname;

		std::ostringstream oss;
		oss << ": " << oldNickname << " is now known as " << nickname << ".";
		server.broadcastNotice( oss.str() );
		server.broadcastName( this );
	}

	void MoagUser::handleCommand(char *buff) {
		// note that the argument is NOT a const char, must be
		// modifiable
		const char *cmd = strtok( buff, " " );
		if( !strcmp( cmd, "n" ) || !strcmp( cmd, "nick" ) ) {
			const char *nick = strtok( 0, "" );
			changeNickname( nick );
		} else {
			server.sendNoticeTo( ": unknown command.", this );
		}
	}

	void MoagUser::handleMessage(void) {

		if( moag::ReceiveChunk( conn, 1 ) == -1 ) {
			markForDisconnectionError( "error on receive waiting for message length" );
			return;
		}

		int length = moag::ChunkDequeue8();
		char buff[256]; // safe since length field is 8-bit

		if( length < 0 || length >= sizeof buff ) {
			markForDisconnectionError( "message buffer overflow or invalid length field" );
			return;
		}

		memset( buff, 0, sizeof buff );

		if( moag::ReceiveChunk( conn, length ) == -1 ) {
			markForDisconnectionError( "error on receive waiting for message" );
			return;
		}

		// is there some weird efficiency reason why we're
		// doing a call per byte instead of normal recv()-style?
		for(int i=0;i<length;i++) {
			buff[i] = moag::ChunkDequeue8();
		}

		if( buff[0] == '/' ) {
			handleCommand( &buff[1] );
		} else {
			server.broadcastChatMessage( this, std::string( buff ) );
		}
	}

	void MoagUser::handleActivity(void) {

		if( moag::ReceiveChunk( conn, 1 ) == -1 ) {
			markForDisconnection();
			return;
		}
		int c2sm_type = moag::ChunkDequeue8();
		switch( c2sm_type ) {
			case C2SM_RELEASE_LEFT:
				keypressLeft = false;
				break;
			case C2SM_RELEASE_RIGHT:
				keypressRight = false;
				break;
			case C2SM_RELEASE_UP:
				keypressUp = false;
				break;
			case C2SM_RELEASE_DOWN:
				keypressDown = false;
				break;
			case C2SM_RELEASE_FIRE:
				keypressFire = false;
				break;
			case C2SM_PRESS_LEFT:
				keypressLeft = true;
				break;
			case C2SM_PRESS_RIGHT:
				keypressRight = true;
				break;
			case C2SM_PRESS_UP:
				keypressUp = true;
				break;
			case C2SM_PRESS_DOWN:
				keypressDown = true;
				break;
			case C2SM_PRESS_FIRE:
				keypressFire = true;
				break;
			case C2SM_CHAT_MESSAGE:
				handleMessage();
				break;
			default:
				markForDisconnectionError( "invalid chunktype" );
				break;
		}
	}
	
	void Server::activitySweep(void) {
		for(userlist_t::iterator i = users.begin(); i != users.end(); i++) {
			MoagUser *user = *i;
			if( user->markedForDisconnection() ) {
				continue;
			}

			moag::Connection conn = user->getConnection();

			if( moag::HasActivity( conn, 0 ) ) {
				user->handleActivity();
			}
		}
	}

	void Server::didTick(void) {
		++tickCount;
	}

	void Server::disconnectSweep(void) {
		for(userlist_t::iterator i = users.begin(); i != users.end(); ) {
			if( (*i)->markedForDisconnection() ) {
				MoagUser *user = *i;
				i = users.erase( i );

				delete user;
			} else {
				i++;
			}
		}
	}

	void Server::broadcastNotice( const std::string& msg ) {
		sendNoticeTo( msg, 0 );
	}

	void Server::broadcastChatMessage( MoagUser *user, const std::string& msg ) {
		int length = MIN( 255, msg.length() );
		const char *str = msg.c_str();

		moag::ChunkEnqueue8( MSG_CHUNK );
		moag::ChunkEnqueue8( user->getId() );
		moag::ChunkEnqueue8( MSGTYPE_CHAT );
		moag::ChunkEnqueue8( length );
		for(int i=0;i<length;i++) {
			moag::ChunkEnqueue8( str[i] );
		}

		broadcastChunks();
	}

	void Server::broadcastName( MoagUser *user ) {
		const std::string& name = user->getName();
		int length = name.length();
		const char *data = name.c_str();

		moag::ChunkEnqueue8( MSG_CHUNK );
		moag::ChunkEnqueue8( user->getId() );
		moag::ChunkEnqueue8( MSGTYPE_NICKCHANGE );

		moag::ChunkEnqueue8( length );
		for(int i=0;i<length;i++) {
			moag::ChunkEnqueue8( data[i] );
		}

		broadcastChunks();
	}

	void Server::sendNoticeTo( const std::string& msg, MoagUser *user ) {
		int length = MIN( 255, msg.length() );
		const char *str = msg.c_str();

		moag::ChunkEnqueue8( MSG_CHUNK );
		moag::ChunkEnqueue8( MSGORIGIN_NONE );
		moag::ChunkEnqueue8( MSGTYPE_NOTICE );
		moag::ChunkEnqueue8( length );
		for(int i=0;i<length;i++) {
			moag::ChunkEnqueue8( str[i] );
		}

		if( !user ) {
			broadcastChunks();
		} else {
			sendChunksTo( user );
		}
	}
};

int main(int argc, char*argv[]) {
	using namespace MoagServer;
	using namespace std;

	try {
		Server server (8080, MAX_CLIENTS);

		cout << "Server running.." << endl;

		server.run(65);

		cout << "Goodbye!" << endl;
	}
	catch( std::exception& e ) {
		cerr << "caught fatal exception: " << e.what() << endl;
	}

	return 0;
}
