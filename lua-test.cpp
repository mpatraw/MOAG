#include <string>
#include <stdexcept>
#include <iostream>

#include "moag_lua.h"

int main(int argc, char *argv[]) {
	using namespace MoagScript;
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

	cout << LuaCall ( lua, "luafunc_div" )
				( LuaCall( lua, "luafunc_add" )(4)(6).getNumber() )
				( LuaCall( lua, "luafunc_add" )(3)(2).getNumber() )
				.getNumber() << endl;

	return 0;
}
