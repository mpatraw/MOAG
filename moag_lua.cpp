#include "moag_lua.h"

#include <stdexcept>

namespace MoagScript {
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

	LuaReference* LuaInstance::popReference(void) {
		return new LuaReference( *this );
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

	LuaReference::LuaReference( LuaInstance& lua ) :
		lua ( lua ),
		index ( luaL_ref( lua.getLua(), LUA_REGISTRYINDEX ) )
	{
	}

	LuaReference::~LuaReference(void) {
		luaL_unref( lua.getLua(), LUA_REGISTRYINDEX, index );
	}

	void LuaReference::push(void) {
		lua_rawgeti( lua.getLua(), LUA_REGISTRYINDEX, index ); 
	}

	lua_State* LuaInstance::getLua(void) {
		return lua;
	}

	LuaCall::LuaCall( LuaInstance& lua, std::string fname ) :
		lua ( lua ),
		argcount( 0 ),
		results( 1 )
	{
		lua.pushGlobal( fname );
	}

	LuaCall::LuaCall( LuaInstance& lua, LuaReference& ref) :
		lua ( lua ),
		argcount( 0 ),
		results( 1 )
	{
		ref.push();
	}

	void LuaCall::discard(void) {
		lua.call( argcount, results );
	}

	double LuaCall::getNumber(void) {
		lua.call( argcount, results );
		return lua.popNumber();
	}

	std::string LuaCall::getString(void) {
		lua.call( argcount, results );
		return lua.popString();
	}

	LuaReference* LuaCall::getReference(void) {
		lua.call( argcount, results );
		return lua.popReference();
	}

}

