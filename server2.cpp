#include "server2.h"

#include "moag_chunk.h"

#include <stdexcept>

#include <iostream>

#include <sstream>

#include "SDL/SDL.h"
#include "SDL/SDL_net.h"

#include "server_lua.h"

/* Note that we do NOT include moag.h, as we'll be replacing
   the structures contained therein with similarly-named ones.
   They're not completely equivalent; notably there's now
   a more significant split between a User and a Tank.
   The client won't generally need all these structures, just
   enough to put the thing on the screen -- which is often
   just id, sprite, position (but for tank also e.g. facing).
   */

#define DEBUG_PUBLIC_SHUTDOWN

#define MOAG_FLUSH_CHUNKS() moag::SendChunk(0, -1, 1)

#define TERRAIN_WIDTH 800
#define TERRAIN_HEIGHT 600

#ifndef UINT32_MAX
#define UINT32_MAX ((Uint32)-1)
#endif

#define MSGORIGIN_NONE -1

#define MSGTYPE_CHAT 1
#define MSGTYPE_NICKCHANGE 2
#define MSGTYPE_NOTICE 3

#define MAX_CLIENTS 8

namespace MoagServer {
	int MoagUser::nextUserId = 1;

	bool MoagUser::getKey( input_key_t key ) const {
		switch( key ) {
			case INPUT_LEFT: return keypressLeft;
			case INPUT_RIGHT: return keypressRight;
			case INPUT_DOWN: return keypressDown;
			case INPUT_UP: return keypressUp;
			case INPUT_FIRE: return keypressFire;
			default: return false;
		}
	}

	MoagTicker::MoagTicker( Server& server ) :
		server ( server )
	{
	}

	MoagGreeter::MoagGreeter( Server& server ) :
		server ( server )
	{
	}

	MoagUser::MoagUser( Server& server, moag::Connection conn, Tank *tank ) :
		server( server ),
		conn( conn ),
		name( "AfghanCap" ),
		id( nextUserId++ ),
		marked( false ),
		keypressLeft ( false ),
		keypressRight ( false ),
		keypressDown ( false ),
		keypressUp ( false ),
		keypressFire ( false ),
		tank ( tank ),
		sob ( MoagScript::LuaCall( server.getLuaInstance(), "create_moag_user" )
                ( this )( id ).getReference() )
	{
	}

	int MoagTicker::operator()(moag::Connection con) {
		server.acquireMutex();
		server.stepGame();
		server.activitySweep();
		server.disconnectSweep();
		server.didTick();
		server.releaseMutex();
		return 0;
	}

	int MoagGreeter::operator()(moag::Connection con) {
		using namespace std;
		server.acquireMutex();
		int rv = server.userConnected( con );
		server.releaseMutex();
		return rv;
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
#ifdef DEBUG_TIME
			int ticks = tickCount;
#endif
			tickCount = 0;
#ifdef DEBUG_TIME
			cerr << "debug: ticked " << ticks << " times this second" << endl;
#endif
		}
		SDL_RemoveTimer( id );
	}

	void Server::shutdown(void) {
		doQuit = true;
	}

	Server::Server( MoagScript::LuaInstance& lua, const int port, const int maxClients, const int width, const int height) :
		ticker( MoagTicker( *this ) ),
		greeter( MoagGreeter( *this ) ),
		state ( GameState( *this, width, height ) ),
		tickCount( 0 ),
		doQuit( false ),
		users (),
        lua ( lua )
	{

		MoagScript::LuaCall( lua, "initialize_server" )( this ).discard();

		if( moag::OpenServer( port, maxClients ) == -1 ) {
			throw std::runtime_error( "failed to open server -- port bound?" );
		}

		pthread_mutex_init( &mutex, 0 );

		moag::SetServerCallback( &ticker, moag::CB_SERVER_UPDATE );
		moag::SetServerCallback( &greeter, moag::CB_CLIENT_CONNECT );
	}

	void Server::acquireMutex(void) {
		pthread_mutex_lock( &mutex );
	}

	void Server::releaseMutex(void) {
		pthread_mutex_unlock( &mutex );
	}

	Server::~Server(void) {
		for( userlist_t::iterator i = users.begin(); i != users.end(); ) {
			MoagUser *user = *i;
			moag::Disconnect( user->getConnection() );
			delete user;
			i = users.erase( i );
		}
		moag::CloseServer();

		pthread_mutex_destroy( &mutex );
	}

	int Server::userConnected(moag::Connection conn) {
		Tank *tank = state.spawnTank();
		MoagUser *user = new MoagUser( *this, conn, tank );
		tank->setUser( user );

		users.push_back( user );

		broadcastName( user );

		for( userlist_t::iterator i = users.begin(); i != users.end(); i++) {
			broadcastName( *i ); // broadcasting is just laziness
		}

		state.enqueueAll();
		sendChunksTo( user );

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
			using namespace std;
			int rv = moag::SendChunk( (*i)->getConnection(), moag::SEND_ALL, 0 );
			
			if( rv == -1 ) {
				(*i)->markForDisconnection();
			}
		}
		MOAG_FLUSH_CHUNKS();
	}

	void MoagUser::setNickname( const std::string& nickname ) {
		name = nickname;
		server.broadcastName( this );
	}

	void MoagUser::changeNickname( const std::string& nickname ) {
		using namespace std;
		std::string oldNickname = name;

		setNickname( name );

		std::ostringstream oss;
		oss << ": " << oldNickname << " is now known as " << nickname << ".";
		server.broadcastNotice( oss.str() );
	}

	void MoagUser::handleCommand(char *buff) {
		// note that the argument is NOT a const char, must be
		// modifiable
		const char *cmd = strtok( buff, " " );
		if( !strcmp( cmd, "n" ) || !strcmp( cmd, "nick" ) ) {
			const char *nick = strtok( 0, "" );
			changeNickname( nick );
#ifdef DEBUG_PUBLIC_SHUTDOWN
		} else if( !strcmp( cmd, "shutdown" ) ) {
			server.shutdown();
#endif
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

		if( length < 0 || length >= (int) sizeof buff ) {
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

		MoagScript::LuaCall( server.getLuaInstance(), "handle_console" )
			.refarg( *sob )( std::string( buff ) ).discard();
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
		moag::ChunkEnqueue8( user->getTankId() );
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
		moag::ChunkEnqueue8( user->getTankId() );
		moag::ChunkEnqueue8( MSGTYPE_NICKCHANGE );

		moag::ChunkEnqueue8( length );
		for(int i=0;i<length;i++) {
			moag::ChunkEnqueue8( data[i] );
		}

		broadcastChunks();
	}

	void Server::stepGame(void) {
		/* Gospel code updates in this order:
				crateUpdate
				(send crate)
				for each tank:
					tankUpdate
					(send tank)
				for each bullet:
					bulletUpdate
				(send bullets)
				for each client: // activity sweep
					update user input

			Terrain updates are done along the way, which probably
			means only during bullet updates.
		*/

		state.update();

		state.enqueueDirty();
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

	moag::Connection MoagUser::getConnection(void) {
		return conn;
	}
	void MoagUser::markForDisconnection(void) {
		marked = true;
	}
	bool MoagUser::markedForDisconnection(void) {
		return marked;
	}
	int MoagUser::getTankId(void) const {
		return tank->getId();
	}
	int MoagUser::getId(void) const {
		return id;
	}
	const std::string& MoagUser::getName(void) const {
		return name;
	}

	MoagUser::~MoagUser(void) {
		using namespace std;
		MoagScript::LuaCall( server.getLuaInstance(), "destroy_moag_user" )
			.refarg( *sob ).discard();
		delete tank;
	}
};

int main(int argc, char*argv[]) {
	using namespace MoagServer;
	using namespace std;
    using namespace MoagScript;

	int scriptsLoaded = 0;

	try {
        LuaInstance lua;
		
		lua.addPackagePath( "./script/?.lua" );

        exportAllServer( lua );

		for(int i=1;i<argc;i++) {
			cout << "Loading: " << argv[i] << "..";
			lua.runFile( argv[i] );
			cout << " done." << endl;
			++scriptsLoaded;
		}
		if( scriptsLoaded < 1 ) {
			throw std::runtime_error( "no scripts loaded" );
		}

		Server server (lua, 8080, MAX_CLIENTS, TERRAIN_WIDTH, TERRAIN_HEIGHT);

		cout << "Server running.." << endl;

		server.run(65);

#if 0
		SDLNet_Quit();
		SDL_Quit();
#endif

		cout << "Goodbye!" << endl;
	}
	catch( std::exception& e ) {
		cerr << "caught fatal exception: " << e.what() << endl;
	}

	return 0;
}
