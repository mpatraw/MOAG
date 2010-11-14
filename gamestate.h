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
			bool facingLeft;
			int angle;

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

			void setUser(MoagUser*);
			void update(void);

			bool isDirty(void) const { return dirty; }

			static Tank* spawn(int);
	};

	struct Crate {
	};

	struct Bullet {
	};

	typedef char tile_t;
		
    class GameState {
		private:
			Server& server;

			int width, height;
			tile_t *terrain;

			typedef std::list<Rectangle> rectlist_t;
			rectlist_t dirty_terrain;

#if 0
			int zone_width, zone_height;
			int dirty_cols, dirty_rows, dirty_size;
			int *dirty;
#endif

			int spawns;
			
			typedef std::vector<Tank*> tanklist_t;
			typedef std::vector<Crate*> cratelist_t;
			typedef std::vector<Bullet*> bulletlist_t;

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
#if 0
				if( terrain[ x + y * width ] != v ) {
					const int dc = x / zone_width;
					const int dr = y / zone_height;
					dirty[ dc + dr * zone_width ] = 1;
				}
#endif
				terrain[ x + y * width ] = v;
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
			int getLeastFreeBulletId(void);

			// placeholders to be replaced by Lua updating

			void enqueueDirtyTerrain(void);
			void enqueueAllTerrain(void);
			void enqueueDirtyTanks(void);
			void enqueueAllTanks(void);

			void enqueueAll(void);
			void enqueueDirty(void);

			Tank *spawnTank(void);

			void update(void);

    };

};

#endif
