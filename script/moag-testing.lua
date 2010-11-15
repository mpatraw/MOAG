moagScriptName = "LuaTesting"
moagScriptVersion = "0.0.1"

AdminPassphrase = "so much tea"

serverPointer = nil
totalPlayersConnected = 0
connectedUsers = {}

initialize_server = function( srvptr )
	serverPointer = srvptr
end

create_moag_user = function( userptr, id )
    rv = {}

	totalPlayersConnected = totalPlayersConnected + 1
	table.insert( connectedUsers, rv )

	rv.userPointer = userptr
	rv.name = string.format( "player%d", totalPlayersConnected )

	set_user_nickname( userptr, rv.name )

	send_notice_to( serverPointer, userptr, string.format( "Welcome to MOAG, %s! %d users online.", rv.name, # connectedUsers ) )
	send_notice_to( serverPointer, userptr, string.format( "This is MOAG/%s version %s.", moagScriptName, moagScriptVersion ) )
	broadcast_notice( serverPointer, string.format( "A new challenger appears: %s has connected. %d users online.", rv.name, # connectedUsers ) );

    return rv
end

destroy_moag_user = function( player )
	for k,v in ipairs( connectedUsers ) do
		if v == player then
			table.remove( connectedUsers, k )
			break
		end
	end
	broadcast_notice( serverPointer, string.format( "%s has left the game. %d players connected.", player.name, # connectedUsers ) )
	return nil
end
