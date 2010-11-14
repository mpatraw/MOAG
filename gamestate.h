#ifndef H_GAMESTATE
#define H_GAMESTATE

/* Lots of the functionality in the gamestate class can and should
   be outsourced to Lua when scripting gets in. Not because it
   can't be written legibly in C, but because it's much easier to
   change flexibly in Lua to quickly write fun new weapons, game
   modes and such -- so as much as possible should be done in Lua.

   Basically all the updating functionality should go, and
   most of the contents of the Tank, Crate, Bullet, etc. classes
   should be replaced with a pointer to the equivalent Lua structure.
*/

#include <list>
#include <vector>

#include <cassert>

#define TERRAIN_BLANK 0
#define TERRAIN_DIRT 1

namespace MoagServer {
	class MoagUser;
	class Server;
	class GameState;

	struct Rectangle {
		int x, y, w, h;

		Rectangle( int x, int y, int w, int h ) :
			x(x), y(y), w(w), h(h)
		{
		}

	};

	class Tank {
		private:
			int id;
			GameState& state;

			int x, y;
			double lastX, lastY;
			bool facingLeft;
			int angle;
			double firepower;

			MoagUser *user;

			bool dirty;

			// to be replaced by Lua
			bool updateHorizontalMovement(int);
			bool updateMovement(void);
			void updatePhysics(bool);

		public:
			Tank(GameState&, int, int, int);
			void enqueue(bool);

			int getId(void) const { return id; }

			void teleport(int,int);

			void setUser(MoagUser*);
			void update(void);

			bool isDirty(void) const { return dirty; }

			static Tank* spawn(int);

			int getX() const { return x; }
			int getY() const { return y; }

			void fire(double);
	};

	struct Crate {
	};



	class Bullet {
		protected:
			GameState& state;

			double fx, fy;
			double vx, vy;

			bool deletable;

			void getIntPosition(int&, int&);

			double getSquaredDistanceTo(double, double);

			int owner;

		public:
			Bullet(GameState&, double, double, double, double);
			virtual ~Bullet(void) {};

			void setOwner( MoagUser* );

			virtual void update(void);
			virtual void detonate(void);
			virtual void enqueue(void);
			virtual bool hitsTank(Tank*);

			bool isDeletable(void) const { return deletable; }
	};

	typedef char tile_t;
		
    class GameState {
		private:
			Server& server;

			int width, height;
			tile_t *terrain;

			typedef std::list<Rectangle> rectlist_t;
			rectlist_t dirty_terrain;

			int spawns;
			
			typedef std::vector<Tank*> tanklist_t;
			typedef std::vector<Crate*> cratelist_t;
			typedef std::list<Bullet*> bulletlist_t;

			tanklist_t tanks;
			bulletlist_t bullets;

			Crate *crate;

			void initializeTerrain(void);

			void enqueueTerrainRectangle(int,int,int,int);
			void enqueueTerrainRectangle(Rectangle&);

		public:
			int getWidth(void) const { return width; }
			int getHeight(void) const { return height; }


			GameState(Server&, const int, const int);
			~GameState(void);

			// I'd make these inline, but then I suppose Lua would
			// have trouble accessing them. Ah well, would be
			// premature optimization anyway.
			void setTerrain(int x, int y, tile_t v) {
				assert( x >= 0 && x < width && y >= 0 && y < height );
				terrain[ x + y * width ] = v;
			}

			bool isOnMap(int x, int y) const {
				return ( x >= 0 && x < width && y >= 0 && y < height );
			}

			bool isBlank(int x, int y) const {
				return getTerrain(x,y) == TERRAIN_BLANK;
			}

			tile_t getTerrain(int x, int y) const {
				assert( x >= 0 && x < width && y >= 0 && y < height );
				return terrain[ x + y * width ];
			}

			// these are for compatibility with a finite client
			// for as long as possible
			int getLeastFreeTankId(void);

			// placeholders to be replaced by Lua updating

			void enqueueDirtyTerrain(void);
			void enqueueAllTerrain(void);
			void enqueueDirtyTanks(void);
			void enqueueAllTanks(void);

			void enqueueAll(void);
			void enqueueDirty(void);

			Tank *spawnTank(void);
			void respawn(Tank*);

			void update(void);

			double getGravity(void) { return 0.1; }
			double getFirepowerPerTick(void) { return 0.1; }
			double getMaxFirepower(void) { return 10.0; }

			void enqueueBullets(void);
			void deleteBullets(void);

			bool hitsAnyTank( Bullet* );

			void addBullet(Bullet* );

			void fillCircle( int, int, int, tile_t );
			void killCircle( int, int, int, int );

			void reportKill( int, Tank* );
    };

};

#endif
