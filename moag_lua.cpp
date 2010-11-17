#include "moag_lua.h"

#include <stdexcept>
#include <iostream>

namespace MoagScript {
	void LuaInstance::dumpStack(std::string reason) {
		using namespace std;

		int i;
		int top = lua_gettop(lua);

		cerr << "Dumping stack: " << reason << endl;
		cerr << "Stack size: " << top << endl;

		for (i = 1; i <= top; i++) {
			int t = lua_type(lua, i);
			cerr << "\t";
			switch (t) {
				case LUA_TSTRING:
					cerr << "string: " << lua_tostring( lua, i ) << endl;
					break;
				case LUA_TBOOLEAN:
					cerr << "bool: " << lua_toboolean( lua, i ) << endl;
					break;
				case LUA_TNUMBER:
					cerr << "number: " << lua_tonumber( lua, i ) << endl;
					break;
				default:
					cerr << lua_typename( lua, t ) << endl;
					break;
			}
		}
		cerr << "[end of stack]" << endl;
	}

	void* LuaInstance::popUserData(void) {
		void *rv = lua_touserdata( lua, -1 );
		lua_pop( lua, 1 );
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

	int LuaInstance::popInteger(void) {
		int rv = lua_tointeger( lua, -1 );
		lua_pop( lua, 1 );
		return rv;
	}

	bool LuaInstance::popBoolean(void) {
		bool rv = lua_toboolean( lua, -1 );
		lua_pop( lua, 1 );
		return rv;
	}

	double LuaInstance::popNumber(void) {
		double rv = lua_tonumber( lua, -1 );
		lua_pop( lua, 1 );
		return rv;
	}

	LuaReference * LuaInstance::makeTable() {
		lua_newtable( lua );
		return popReference();
	}

	LuaReference * LuaInstance::globalReference( std::string name ) {
		pushGlobal( name );
		return popReference();
	}

	LuaInstance& LuaInstance::call(int noArgs, int noResults, std::string fname) {
		int errorHandlerIndex = 0;
#ifdef DEBUG_LUA_LL
		dumpStack( "calling " + fname);
#endif
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
#ifdef DEBUG_LUA_LL
		dumpStack( "called"  + fname );
#endif
		return *this;
	}

	LuaInstance& LuaInstance::pushValue( bool value ) {
		lua_pushboolean( lua, value );
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
		using namespace std;
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
		lua.pop();
	}

	int LuaCall::getInteger(void) {
		lua.call( argcount, results );
		return lua.popInteger();
	}

	bool LuaCall::getBoolean(void) {
		lua.call( argcount, results );
		return lua.popBoolean();
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

	void LuaInstance::addPackagePath( std::string path ) {
		pushGlobal( "package" );
		lua_getfield( lua, -1, "path" );
		std::string newpath = std::string(lua_tostring( lua, -1 )) + ";" + path;
		lua_pop( lua, 1 );
		lua_pushstring( lua, newpath.c_str() );
		lua_setfield( lua, -2, "path" );
		lua_pop( lua, 1 );
	}

	void LuaReference::setBoolean( std::string name, bool value) {
		push();
		lua.pushValue( name );
		lua.pushValue( value );
		lua_settable( lua.getLua(), -3 );
		lua_pop( lua.getLua(), 1 );
	}
}

