moagScriptName = "LuaSuperficial"
moagScriptVersion = "0.0.1"

AdminPassphrase = "things fall apart, the center cannot hold"

require ("moag-standard-console" )

serverPointer = nil
totalPlayersConnected = 0
connectedUsers = {}

tankIds = {}

t = 0.0


update = function()
	t = t + 0.01
	put_bullet( serverPointer, 320 + 50 * math.cos( t ), 320 + 50 * math.sin( t ) )
end

next_tank_id = function()
	i = 0
	while true do
		if not tankIds[i] then
			tankIds[i] = true
			return i
		end
		i = i + 1
	end
end

free_tank_id = function(id)
	tankIds[id] = false
end

initialize_server = function( srvptr )
	print( "initialized with server pointer:" )
	print( srvptr )
	serverPointer = srvptr

	w = get_terrain_width( serverPointer )
	h = get_terrain_width( serverPointer )
	terrain_fill_circle( serverPointer, w/2, h/2, 200, 1 )
	terrain_fill_circle( serverPointer, w/2, h/2, 180, 0 )

end

create_moag_user = function( userptr, id )
    rv = {}

	totalPlayersConnected = totalPlayersConnected + 1
	table.insert( connectedUsers, rv )

	rv.userPointer = userptr
	rv.name = string.format( "player%d", totalPlayersConnected )
	print( serverPointer )
	rv.tankId = next_tank_id()
	rv.tank = make_tank( serverPointer, rv.tankId, string.format( "%s's little tank", rv.name ) )
	print( "tank:" )
	print( rv.tank )
	rv.change_nick = function(s)
		rv.name = s
		set_tank_name( rv, string.format( "%s's little tank", s ) )
		broadcast_notice( serverPointer, string.format( ": %s is now known as %s", oldname, newname ) )
	end

	set_tank_pos( rv.tank, 320, 200 )

	send_notice_to( serverPointer, userptr, string.format( "Welcome to MOAG, %s! %d users online.", rv.name, # connectedUsers ) )
	send_notice_to( serverPointer, userptr, string.format( "This is MOAG/%s version %s.", moagScriptName, moagScriptVersion ) )
	broadcast_notice( serverPointer, string.format( "A new challenger appears: %s has connected. %d users online.", rv.name, # connectedUsers ) );

    return rv
end

destroy_moag_user = function( player )
	free_tank_id( player.tankId )
	print( "deletin'")
	print ( player.tank )
	delete_tank( serverPointer, player.tank )
	for k,v in ipairs( connectedUsers ) do
		if v == player then
			table.remove( connectedUsers, k )
			break
		end
	end
	broadcast_notice( serverPointer, string.format( "%s has left the game. %d players connected.", player.name, # connectedUsers ) )
	return nil
end
