#ifndef H_MOAG_LUA
#define H_MOAG_LUA

#include <string>

#include <memory>

extern "C" {
#include <lua5.1/lua.h>
#include <lua5.1/lauxlib.h>
#include <lua5.1/lualib.h>
};

namespace MoagScript {
	class LuaReference;

	class LuaInstance {
		private:
			lua_State *lua;
			bool borrowed;
			
		public:
			LuaInstance(void);
			LuaInstance(lua_State*);
			~LuaInstance(void);

			int getTop(void);

			lua_State * getLua(void);

			void addPackagePath( std::string );

			LuaInstance& runFile( std::string );

			LuaInstance& pushGlobal( std::string );

			LuaInstance& pushValue( double );
			LuaInstance& pushValue( int );
			LuaInstance& pushValue( std::string );
			LuaInstance& pushValue( void* );
			LuaInstance& pushValue( LuaReference& );

			LuaInstance& call(int, int);

			void exportFunction( std::string, lua_CFunction );

			LuaReference* globalReference( std::string );

			void pop(void);
			double popNumber(void);
			std::string popString(void);
			LuaReference* popReference(void);
			void * popUserData(void);
	};

	class LuaReference {
		private:
			LuaInstance& lua;
			int index;
		public:
			LuaReference(LuaInstance&);
			~LuaReference(void);

			void push(void);
	};

	typedef std::auto_ptr<LuaReference> LuaAutoReference;

	class LuaCall {
		private:
			LuaInstance& lua;
			int argcount;
			int results;

		public:
			LuaCall( LuaInstance&, std::string );
			LuaCall( LuaInstance&, LuaReference& );

			LuaCall& refarg( LuaReference& ref ) {
				ref.push();
				++argcount;
				return *this;
			}

			template<typename T>
			LuaCall& operator()( T x ) {
				lua.pushValue( x );
				++argcount;
				return *this;
			};

			void discard(void);
			double getNumber(void);
			std::string getString(void);
			LuaReference* getReference(void);
			void * getUserData(void);
	};
};

#endif
