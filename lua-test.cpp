#include <string>
#include <stdexcept>
#include <iostream>

#include "moag_lua.h"

#include "server_lua.h"

#include <memory>

int main(int argc, char *argv[]) {
	using namespace MoagScript;
	using namespace std;

	LuaInstance lua;

	exportAllServer( lua );

	lua.runFile( "lua-test.lua" );

	lua.pushGlobal( "luafunc_helloworld" );
	lua.call( 0, 1 );
	lua.pop();

	lua.pushGlobal( "luafunc_add" );
	lua.pushValue( 3.14159 );
	lua.pushValue( 2.718 );
	lua.call( 2, 1 );
	cout << lua.popNumber() << endl;

	cout << LuaCall ( lua, "luafunc_div" )
				( LuaCall( lua, "luafunc_add" )(4)(6).getNumber() )
				( LuaCall( lua, "luafunc_add" )(3)(2).getNumber() )
				.getNumber() << endl;

	LuaAutoReference lua_div ( lua.globalReference( "luafunc_div" ) );
	LuaAutoReference lua_add ( lua.globalReference( "luafunc_add" ) );
	cout << LuaCall ( lua, *lua_div)
				( LuaCall( lua, *lua_add )(4)(6).getNumber() )
				( LuaCall( lua, *lua_add )(3)(2).getNumber() )
				.getNumber() << endl;

	cout << LuaCall( lua, "test_c_function" )( "hello" )("world").getString() << endl;


	return 0;
}
