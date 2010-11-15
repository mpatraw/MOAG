#include "moag_lua.h"

#include <stdexcept>
#include <iostream>

namespace MoagScript {
	void* LuaInstance::popUserData(void) {
		void *rv = lua_touserdata( lua, -1 );
		return rv;
	}

	void LuaInstance::exportFunction( std::string name, lua_CFunction f ) {
		lua_register( lua, name.c_str(), f );
	}

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

	LuaReference * LuaInstance::globalReference( std::string name ) {
		pushGlobal( name );
		return popReference();
	}

	LuaInstance& LuaInstance::call(int noArgs, int noResults) {
		int errorHandlerIndex = 0;
		int rv = lua_pcall( lua, noArgs, noResults, errorHandlerIndex );
		if( rv ) {
            std::string errmsg = popString();
            switch( rv ) {
                case LUA_ERRRUN: throw std::runtime_error( "lua runtime error: " + errmsg );
                case LUA_ERRMEM: throw std::runtime_error( "lua out of memory: " + errmsg );
                case LUA_ERRERR: throw std::runtime_error( "lua internal error in error handler: " + errmsg );
                default: throw std::runtime_error( "lua internal unknown error: " + errmsg );
            }
		}
		return *this;
	}

	LuaInstance& LuaInstance::pushValue( LuaReference& ref ) {
		ref.push();
		return *this;
	}


	LuaInstance& LuaInstance::pushValue( void *p ) {
		lua_pushlightuserdata( lua, p );
		return *this;
	}

	LuaInstance& LuaInstance::pushValue( std::string s ) {
		lua_pushlstring( lua, s.data(), s.length() );
		return *this;
	}

	LuaInstance& LuaInstance::pushValue( double d ) {
		lua_pushnumber( lua, d );
		return *this;
	}

	LuaInstance& LuaInstance::pushValue( int v ) {
		lua_pushinteger( lua, v );
		return *this;
	}

	LuaInstance& LuaInstance::pushGlobal( std::string name ) {
		lua_getglobal( lua, name.c_str() );
		return *this;
	}

	int LuaInstance::getTop(void) {
		return lua_gettop( lua );
	}

	LuaInstance::LuaInstance( lua_State *lua ) :
		lua ( lua ),
		borrowed ( true )
	{
	}

	LuaInstance::LuaInstance(void) :
		lua ( luaL_newstate() ),
		borrowed ( false )
	{
		if( !lua ) {
			throw std::runtime_error( "unable to initialize Lua" );
		}

		luaL_openlibs( lua );
	}

	LuaInstance::~LuaInstance(void) {
		if( !borrowed ) {
			lua_close( lua );
		}
	}

	LuaInstance& LuaInstance::runFile( std::string filename ) {
		luaL_dofile( lua, filename.c_str() );
		return *this;
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

	void* LuaCall::getUserData(void) {
		lua.call( argcount, results );
		return lua.popUserData();
	}


	LuaReference* LuaCall::getReference(void) {
		lua.call( argcount, results );
		return lua.popReference();
	}

}

