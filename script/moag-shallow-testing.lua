moagScriptName = "LuaSuperficial"
moagScriptVersion = "0.0.1"

AdminPassphrase = "things fall apart, the center cannot hold"

require ("moag-standard-console" )

serverPointer = nil
totalPlayersConnected = 0
connectedUsers = {}

tankIds = {}

t = 0.0

update_user = function(player)
	x = player.x
	y = player.y
	a = player.angle
	-- angles are encoded somewhat weirdly; you can face straight right, but
	-- never straight left. TODO convert to a somewhat more standard
	-- interpretation when we touch the client.
	if player.keys.left then
		x = x - 1
		if a > 0 then a = - a end
	end
	if player.keys.right then
		x = x + 1
		if a < 0 then a = - a end
	end
	if player.keys.up then y = y - 1 end
	if player.keys.down then y = y + 1 end
	player.x = x
	player.y = y
	player.angle = a
	set_tank_pos( player.tank, player.x, player.y )
	set_tank_angle( player.tank, player.angle )
end

update = function()
	t = t + 0.01
	for i,v in ipairs( connectedUsers ) do update_user( v ) end
	sunx = 320 + 50 * math.cos( t )
	suny = 320 + 50 * math.sin( t )
	moonx = sunx + 10 * math.cos( 40 * t )
	moony = suny + 10 * math.sin( 40 * t )
	put_bullet( serverPointer, sunx, suny )
	put_bullet( serverPointer, moonx, moony )
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
	serverPointer = srvptr

	w = get_terrain_width( serverPointer )
	h = get_terrain_width( serverPointer )
	terrain_fill_circle( serverPointer, w/2, h/2, 200, 1 )
	terrain_fill_circle( serverPointer, w/2, h/2, 180, 0 )

end

create_moag_user = function( userptr, id, keys )
    rv = {}

	totalPlayersConnected = totalPlayersConnected + 1
	table.insert( connectedUsers, rv )

	rv.keys = keys
	rv.userPointer = userptr
	rv.name = string.format( "player%d", totalPlayersConnected )
	rv.tankId = next_tank_id()
	rv.tank = make_tank( serverPointer, rv.tankId, rv.name )
	rv.change_nick = function(s)
		oldname = rv.name
		rv.name = s
		set_tank_name( rv.tank, s )
		broadcast_notice( serverPointer, string.format( ": %s is now known as %s", oldname, s ) )
	end

	rv.x = 320
	rv.y = 200
	rv.angle = 1

	set_tank_pos( rv.tank, rv.x, rv.y )

	send_notice_to( serverPointer, userptr, string.format( "Welcome to MOAG, %s! %d users online.", rv.name, # connectedUsers ) )
	send_notice_to( serverPointer, userptr, string.format( "This is MOAG/%s version %s.", moagScriptName, moagScriptVersion ) )
	broadcast_notice( serverPointer, string.format( "A new challenger appears: %s has connected. %d users online.", rv.name, # connectedUsers ) );

    return rv
end

destroy_moag_user = function( player )
	free_tank_id( player.tankId )
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
