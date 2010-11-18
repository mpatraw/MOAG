ConsoleCommands = {}

invalid_nick = function( nick )
	local l = string.len( nick )
	local lowLimit = 2
	local highLimit = 15
	local validChars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789"

	if l < lowLimit then
		return "too short"
	elseif l > highLimit then
		return "too long"
	else
		for ch in nick:gmatch"." do
			if string.find( validChars, ch, 1, true ) == nil then
				return string.format( "invalid character: '%s'", ch )
			end
		end
	end

	return nil
end

change_name = function( player, newname )
	local oldname = player.name
	if invalid_nick( newname ) then
		send_notice_to( serverPointer, player.userPointer, ": invalid nick -- " .. invalid_nick( newname ) )
	else
		player.change_nick( newname )
	end
end

identify_as_admin = function( player, passphrase )
	if player.admin then
		send_notice_to( serverPointer, player.userPointer, ": you already have admin privileges!" )
		return
	end
	if passphrase == AdminPassphrase then
		player.admin = true
		broadcast_notice( serverPointer, string.format( ": admin privileges granted to %s", player.name ) )
	else
		broadcast_notice( serverPointer, string.format( ": %s failed to identify for admin privileges", player.name ) )
	end
end

require_admin = function( player )
	if player.admin then
		return true
	else
		send_notice_to( serverPointer, player.userPointer, string.format( ": permission denied" ) )
	end
end

ConsoleCommands.n = change_name
ConsoleCommands.nick = change_name

ConsoleCommands.admin = identify_as_admin

ConsoleCommands.shutdown = function(player, args)
	if require_admin( player ) then
		shutdown_server( serverPointer )
	end
end

handle_command = function( player, cmd, args )
	cmd = string.lower( cmd )
	if ConsoleCommands[ cmd ] then
		ConsoleCommands[ cmd ]( player, args )
	else
		send_notice_to( serverPointer, player.userPointer, string.format( ": unknown command '%s'", cmd ) )
	end
end

handle_console = function( player, message )
	if string.find( message, "/" ) == 1 then
		local cmdline = string.sub( message, 2 )
		local spacepos = string.find( cmdline, " " )
		if spacepos == nil then
			handle_command( player, cmdline )
		else
			handle_command( player, string.sub( cmdline, 1, spacepos - 1), string.sub( cmdline, spacepos + 1 ) )
		end
	else
		local fullMessage = string.format( "<%s> %s", player.name, message )
		broadcast_notice( serverPointer, fullMessage )
	end
end
