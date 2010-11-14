#include <string>
#include <stdexcept>
#include <iostream>

extern "C" {
#include <lua5.1/lua.h>
#include <lua5.1/lauxlib.h>
#include <lua5.1/lualib.h>
};

class LuaInstance {
	private:
		lua_State *lua;
		
	public:
		LuaInstance(void);
		~LuaInstance(void);

		void runFile( std::string );

		void pushGlobal( std::string );

		void pushValue( double );
		void pushValue( int );
		void pushValue( std::string );

		void pop(void);
		double popNumber(void);
		std::string popString(void);

		void call(int, int);
};

std::string LuaInstance::popString(void) {
	size_t size;
	const char * rvdata = lua_tolstring( lua, -1, &size );
	if( !rvdata ) {
		throw std::runtime_error( "invalid coercion to string" );
	}

	std::string rv = std::string( rvdata, size );

	lua_pop( lua, 1 );

	return rv;
}

void LuaInstance::pop(void) {
	lua_pop( lua, 1 );
}

double LuaInstance::popNumber(void) {
	double rv = lua_tonumber( lua, -1 );
	lua_pop( lua, 1 );
	return rv;
}

void LuaInstance::call(int noArgs, int noResults) {
	int errorHandlerIndex = 0;
	int rv = lua_pcall( lua, noArgs, noResults, errorHandlerIndex );
	if( !rv ) return;
	switch( rv ) {
		case LUA_ERRRUN: throw std::runtime_error( "lua runtime error" );
		case LUA_ERRMEM: throw std::runtime_error( "lua out of memory" );
		case LUA_ERRERR: throw std::runtime_error( "lua internal error in error handler" );
		default: throw std::runtime_error( "lua internal unknown error" );
	}
}

void LuaInstance::pushValue( std::string s ) {
	lua_pushlstring( lua, s.data(), s.length() );
}

void LuaInstance::pushValue( double d ) {
	lua_pushnumber( lua, d );
}

void LuaInstance::pushValue( int v ) {
	lua_pushinteger( lua, v );
}

void LuaInstance::pushGlobal( std::string name ) {
	lua_getglobal( lua, name.c_str() );
}

LuaInstance::LuaInstance(void) :
	lua ( luaL_newstate() )
{
	if( !lua ) {
		throw std::runtime_error( "unable to initialize Lua" );
	}

	luaL_openlibs( lua );
}

LuaInstance::~LuaInstance(void) {
	lua_close( lua );
}

void LuaInstance::runFile( std::string filename ) {
	luaL_dofile( lua, filename.c_str() );
}

int main(int argc, char *argv[]) {
	using namespace std;

	LuaInstance lua;
	lua.runFile( "lua-test.lua" );

	lua.pushGlobal( "luafunc_helloworld" );
	lua.call( 0, 1 );
	lua.pop();

	lua.pushGlobal( "luafunc_add" );
	lua.pushValue( 3.14159 );
	lua.pushValue( 2.718 );
	lua.call( 2, 1 );
	cout << lua.popNumber() << endl;

	return 0;
}
