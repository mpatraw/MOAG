#ifndef H_MOAG_LUA
#define H_MOAG_LUA

#include <string>

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
			
		public:
			LuaInstance(void);
			~LuaInstance(void);

			lua_State * getLua(void);

			void runFile( std::string );

			void pushGlobal( std::string );

			void pushValue( double );
			void pushValue( int );
			void pushValue( std::string );

			void pop(void);
			double popNumber(void);
			std::string popString(void);
			LuaReference* popReference(void);

			void call(int, int);
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

	class LuaCall {
		private:
			LuaInstance& lua;
			int argcount;
			int results;

		public:
			LuaCall( LuaInstance&, std::string );
			LuaCall( LuaInstance&, LuaReference& );

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
	};
};

#endif
