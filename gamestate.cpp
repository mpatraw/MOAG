#include "gamestate.h"

#include "server2.h"

#include "moag_chunk.h"
#include "moag_net.h"

#include <iostream>

namespace MoagServer {
	GameState::GameState(Server& server, const int width, const int height) :
		server( server ),
		width( width ),
		height( height ),
		terrain( new char [width * height] ),
#if 0
		zone_width( 10 ),
		zone_height( 10 ),
		dirty_cols( (width + zone_width - 1) / zone_width ),
		dirty_rows( (height + zone_height - 1) / zone_height ),
		dirty_size( dirty_rows * dirty_cols ),
		dirty( new int [dirty_rows * dirty_cols] ),
#endif
		tanks (1),
		crate (0),
		bullets (1),
		dirty_terrain ()
	{
		initializeTerrain();

#if 0
		for(int i=0;i<dirty_size;i++) {
			dirty[i] = 1;
		}
#endif


	}

	GameState::~GameState() {
		for( tanklist_t::iterator i = tanks.begin(); i != tanks.end(); i++) {
			delete *i;
		}
		delete [] terrain;
	}

	void GameState::initializeTerrain(void) {
		for(int x=0;x<width;x++) {
			for(int y=0;y<height;y++) {
				if( y < (height/3) ) {
					setTerrain( x, y, TERRAIN_BLANK );
				} else {
					setTerrain( x, y, TERRAIN_DIRT );
				}
			}
		}
	}

	void GameState::enqueueTerrainRectangle(int x, int y, int w, int h) {
		moag::ChunkEnqueue8( LAND_CHUNK );
		moag::ChunkEnqueue16( x );
		moag::ChunkEnqueue16( y );
		moag::ChunkEnqueue16( w );
		moag::ChunkEnqueue16( h );
		for(int j=0;j<h;j++) {
			for(int i=0;i<w;i++) {
				int v = getTerrain(x+i, y+j);
				moag::ChunkEnqueue8( v );
			}
		}
	}

	void GameState::enqueueDirtyTerrain(void) {
		while( !dirty_terrain.empty() ) {
			enqueueTerrainRectangle( dirty_terrain.front() );
			dirty_terrain.pop_front();
		}
#if 0
		for(int i=0;i<dirty_size;i++) if( dirty[i] ) {
			int col = i % zone_width;
			int row = i / zone_width;
			int x = col * zone_width;
			int y = row * zone_height;
			int w = MIN( zone_width, width - x );
			int h = MIN( zone_height, height - y );
			enqueueTerrainRectangle( x, y, w, h );
		}
#endif
	}

	void GameState::enqueueAllTerrain(void) {
		enqueueTerrainRectangle( 0, 0, width, height );
	}

	int GameState::getLeastFreeBulletId(void) {
		int id = 0;
		do {
			if( id >= bullets.size() ) {
				bullets.resize( 2 * bullets.size() );
			}
			if( !bullets[id] ) break;
			++id;
		} while(true);
		return id;
	}

	int GameState::getLeastFreeTankId(void) {
		int id = 0;
		do {
			if( id >= tanks.size() ) {
				tanks.resize( 2 * tanks.size() );
			}
			if( !tanks[id] ) break;
			++id;
		} while(true);
		return id;
	}

	Tank::Tank(GameState& state,int id, int x, int y) :
		id ( id ),
		state( state ),
		x ( x ),
		y ( y ),
		facingLeft ( false ),
		angle ( 35 ),
		dirty ( true )
	{
	}

	void Tank::enqueue( bool broadcast ) {
		moag::ChunkEnqueue8( TANK_CHUNK );
		moag::ChunkEnqueue8( id );
		moag::ChunkEnqueue16( x );
		moag::ChunkEnqueue16( y );
		if( facingLeft ) {
			moag::ChunkEnqueue8( -angle  );
		} else {
			moag::ChunkEnqueue8( angle  );
		}
		if( broadcast ) {
			dirty = false;
		}
	}
	
	Tank * GameState::spawnTank(void) {
		int x = ((spawns++) * 240) % (width-40) + 20;
		int y = 60;
		int id = getLeastFreeTankId();
		return tanks[id] = new Tank(*this,id,x,y);
	}
	
	void GameState::enqueueDirtyTanks(void) {
		for(tanklist_t::iterator i = tanks.begin(); i != tanks.end(); i++) {
			Tank *tank = *i;
			if( tank && tank->isDirty() ) {
				tank->enqueue(true); // note assumption
			}
		}
	}

	void GameState::enqueueAll(void) {
		enqueueAllTerrain();
		enqueueAllTanks();
	}

	void GameState::enqueueDirty(void) {
		enqueueDirtyTerrain();
		enqueueDirtyTanks();
	}

	void GameState::enqueueAllTanks(void) {
		for(tanklist_t::iterator i = tanks.begin(); i != tanks.end(); i++) {
			Tank *tank = *i;
			if( tank ) {
				tank->enqueue(false); // note assumption
			}
		}
	}

	void Tank::updatePhysics(bool grav) {
		if( y < 20 ) {
			y = 20;
		} else if(grav) {
			for(int i=0;i<2;i++) {
				if( state.getTerrain(x, y+1) == TERRAIN_BLANK ) {
					using namespace std;
					dirty = true;
					y++;
				}
			}
		}
	}

    bool Tank::updateHorizontalMovement( int dx ) {
        const int xp = x + dx;

        facingLeft = (dx < 0);

        bool legal = facingLeft ? (x >= 10) : (x < (state.getWidth()-10));

        if( state.isBlank(xp,y) && legal ) {
            x = xp;
            return true;
        } else if( state.isBlank(xp,y-1) && legal ) {
            x = xp;
            y--;
            return true;
        } else if( state.isBlank(x,y-1) ||
                   state.isBlank(x,y-2) ||
                   state.isBlank(x,y-3) ) {
            y--;
            return false;
        } else {
            return false;
        }
    }

	bool Tank::updateMovement(void) {
		bool grav = true;

		if( !user ) return grav;

		const bool kLeft = user->getKey( INPUT_LEFT ),
                   kRight = user->getKey( INPUT_RIGHT ),
                   kUp = user->getKey( INPUT_UP ),
                   kDown = user->getKey( INPUT_DOWN ),
                   kFire = user->getKey( INPUT_FIRE ),
                   kLadder = user->getKey( INPUT_LEFT ) && user->getKey( INPUT_RIGHT );
        if( kLadder ) {
            // TODO
            dirty = true;
        } else if( kLeft ) {
            grav = updateHorizontalMovement( -1 );
            dirty = true;
        } else if( kRight ) {
            grav = updateHorizontalMovement( 1 );
            dirty = true;
        }

        if( kUp && angle < 90) {
            angle++;
            dirty = true;
        } else if( kDown && angle > 1) {
            angle--;
            dirty = true;
        }

		return grav;
	}

	void Tank::setUser(MoagUser *user) {
		this->user = user;
	}

	void Tank::update(void) {
		bool grav = updateMovement();
		updatePhysics(grav);
	}

	void GameState::update(void) {
		for(tanklist_t::iterator i = tanks.begin(); i != tanks.end(); i++) {
			Tank *tank = *i;
			if( tank ) {
				tank->update();
			}
		}
	}

	void GameState::enqueueTerrainRectangle(Rectangle& rect) {
		enqueueTerrainRectangle( rect.x, rect.y, rect.w, rect.h );
	}


};
