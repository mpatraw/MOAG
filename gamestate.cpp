#include "gamestate.h"

#include "server2.h"

#include "moag_chunk.h"
#include "moag_net.h"

#include <iostream>

#include <cmath>

namespace MoagServer {
	GameState::GameState(Server& server, const int width, const int height) :
		server( server ),
		width( width ),
		height( height ),
		terrain( new char [width * height] ),
		tanks (1),
		crate (0),
		bullets (0),
		dirty_terrain ()
	{
		initializeTerrain();

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
		if( x < 0 ) {
			w += x;
			x = 0;
		}
		if( y < 0 ) {
			h += y;
			y = 0;
		}
		w = MIN( w, width - x );
		h = MIN( h, height - y );
		if( w > 0 && h > 0 ) {
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
	}

	void GameState::enqueueDirtyTerrain(void) {
		while( !dirty_terrain.empty() ) {
			enqueueTerrainRectangle( dirty_terrain.front() );
			dirty_terrain.pop_front();
		}
	}

	void GameState::enqueueAllTerrain(void) {
		enqueueTerrainRectangle( 0, 0, width, height );
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
        lastX(x),
        lastY(y),
		facingLeft ( false ),
		angle ( 35 ),
        firepower ( 0 ),
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

	void GameState::addBullet(Bullet *bullet) {
        bullets.push_back( bullet );
	}

	void Tank::teleport(int x, int y) {
		lastX = this->x = x;
		lastY = this->y = y;
	}

	void GameState::respawn( Tank *tank ) {
		int x, y;
		x = ((spawns++) * 240) % (width-40) + 20;
		y = 60;
		tank->teleport( x, y );
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
        enqueueBullets();
	}

	void GameState::enqueueDirty(void) {
		enqueueDirtyTerrain();
		enqueueDirtyTanks();
        enqueueBullets();
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

        lastX = x;
        lastY = y;

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

        if( kFire ) {
            firepower += state.getFirepowerPerTick();
            firepower = MIN( state.getMaxFirepower(), firepower );
        } else if( firepower > 0.0 ) {
            fire( firepower );
            firepower = 0;
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

    bool GameState::hitsAnyTank( Bullet* bullet ) {
		for(tanklist_t::iterator i = tanks.begin(); i != tanks.end(); i++) {
			Tank *tank = *i;
			if( tank ) {
                if( bullet->hitsTank( tank ) ) {
                    return true;
                }
			}
		}
        return false;
    }

	void GameState::update(void) {
		for(tanklist_t::iterator i = tanks.begin(); i != tanks.end(); i++) {
			Tank *tank = *i;
			if( tank ) {
				tank->update();
			}
		}
		for(bulletlist_t::iterator i = bullets.begin(); i != bullets.end(); i++) {
			Bullet *bullet = *i;
            bullet->update();
		}
        deleteBullets();
	}

	void GameState::enqueueTerrainRectangle(Rectangle& rect) {
		enqueueTerrainRectangle( rect.x, rect.y, rect.w, rect.h );
	}

    void GameState::deleteBullets(void) {
        for(bulletlist_t::iterator i = bullets.begin(); i != bullets.end(); ) {
            bool shouldGo = (*i)->isDeletable();
            if( !shouldGo ) {
                i++;
                continue;
            }
            delete *i;
            i = bullets.erase( i );
        }
    }

    bool Bullet::enqueue(bool dry) {
        int x, y;
        getIntPosition( x, y );
		if( state.isOnMap( x, y ) ) {
			if( !dry ) {
				moag::ChunkEnqueue16( x );
				moag::ChunkEnqueue16( y );
			}
			return true;
		}
		return false;
    }

    void GameState::enqueueBullets(void) {
        using namespace std;
		int count = 0;
        moag::ChunkEnqueue8( BULLET_CHUNK );
        for(bulletlist_t::iterator i = bullets.begin(); i != bullets.end(); i++) {
            Bullet *bullet = *i;
			count += bullet->enqueue(true) ? 1 : 0;
        }
        moag::ChunkEnqueue16( count );
        for(bulletlist_t::iterator i = bullets.begin(); i != bullets.end(); i++) {
            Bullet *bullet = *i;
			bullet->enqueue(false);
        }
    }

    void Bullet::getIntPosition(int& x, int& y) {
        x = (int)(0.5 + fx);
        y = (int)(0.5 + fy);
    }

    void Bullet::detonate(void) {
		const int radius = 12;
		int x, y;
		
		getIntPosition( x, y );

		state.fillCircle( x, y, radius, TERRAIN_BLANK );
		state.killCircle( x, y, radius + 4, owner );

        deletable = true;
    }

    double Bullet::getSquaredDistanceTo(double x, double y) {
        return (x-fx)*(x-fx) + (y-fy)*(y-fy);
    }

    void GameState::fillCircle( int x, int y, int r, tile_t v ) {
		for(int i=-r;i<=r;i++) for(int j=-r;j<=r;j++) {
			if( (i*i)+(j*j) < r*r ) {
				setTerrain( x+i, y+j, v );
			}
		}
		dirty_terrain.push_back( Rectangle( x-r, y-r, 2*r+1, 2*r+1 ) );
    }

	void GameState::reportKill( int killerId, Tank *victim ) {
		// TODO
	}

	void GameState::killCircle( int x, int y, int r, int killerId ) {
		for(tanklist_t::iterator i = tanks.begin(); i != tanks.end();) {
			do {
				Tank *tank = *i;

				if( !tank ) break;

				int tx = tank->getX(), ty = tank->getY();
				if( (tx-x)*(tx-x) + (ty-y-3)*(ty-y-3) < (r*r) ) {
					reportKill( killerId, tank );
					respawn( tank ); // currently instant
					continue;
				}
			} while(0);
			i++;
		}
	}

    bool Bullet::hitsTank( Tank *tank ) {
        const double sumRadius = 8.4852;
        const double squaredSR = sumRadius * sumRadius;

        return getSquaredDistanceTo( tank->getX(), tank->getY() ) < squaredSR;
    }

    void Bullet::update(void) {
        int x, y;

        fx += vx;
        fy += vy;
        vy += state.getGravity();

        using namespace std;

        getIntPosition( x, y );

        if( !state.isBlank( x, y ) ||
            state.hitsAnyTank( this ) ) {
            detonate();
            return;
        }
    }

    void Tank::fire(double power) {
        using namespace std;
        const double theta = (facingLeft ? (180.0-angle) : angle) * M_PI / 180.0;
        const double cost = cos(theta), sint = - sin(theta);
        const double barrelLength = 5.0;
        const double bulletX = x + barrelLength * cost;
        const double bulletY = y - 7 + barrelLength * sint;
        const double velX = (x - lastX) + power * cost;
        const double velY = (y - lastY) + power * sint;
        Bullet *rv;
        rv = new Bullet( state, bulletX, bulletY, velX, velY );

        if( user ) {
            rv->setOwner( user );
        }
    }

    void Bullet::setOwner( MoagUser* user ) {
        owner = user->getId();
    }

	Bullet::Bullet(GameState& state, double fx, double fy, double vx, double vy) :
            state(state), fx(fx), fy(fy), vx(vx), vy(vy), deletable(false), owner(-1)
    {
        state.addBullet( this );
    }





};
