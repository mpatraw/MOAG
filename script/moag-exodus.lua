moagScriptName = "LuaExodus"
moagScriptVersion = "0.0.1"

AdminPassphrase = "assuming she's a mammal"

require ("moag-standard-console" )

ladderTime = 60
ladderLength = 64

terrainWidth = 0
terrainHeight = 0
serverPointer = nil
totalPlayersConnected = 0
standardGravity = 0.1
respawnTime = 40

totalSpawns = 0

connectedUsers = {}
tankIds = {}
allBullets = {}
crateSingleton = nil

Weapons = {}

make_weapon = function( name, bullet_creator, frequency )
    local rv = {}
    rv.name = name
    rv.bullet_creator = bullet_creator
	rv.frequency = frequency
    Weapons[ name ] = rv
end

create_mirv_warhead = function(player, x, y, vx, vy)
	local rv = create_normal_bullet(player, x, y, vx, vy)
	rv.explode_radius = 30
	return rv
end

create_mirv = function(player, x, y, vx, vy)
	local rv = create_normal_bullet(player, x, y, vx, vy)
	rv.detonate_on_descent = true
	rv.detonate = function(bullet)
		destroy_around( bullet, 12, bullet.owner )
		for i=-3,3 do
			spawn_bullet( create_mirv_warhead( bullet.owner, bullet.x, bullet.y,
											   bullet.vx + i, bullet.vy ) )
		end
	end
	return rv
end

create_cluster_bomb = function(player, x, y, vx, vy)
	local rv = create_normal_bullet(player, x, y, vx, vy)
	-- TODO: check bouncing, does it really bounce?
	rv.detonate = function(bullet)
		local power = 1.5 -- changed from 1.5
		local spawns = 10
		local inc = 2 * math.pi / spawns
        local oldXPower = 0.5 -- changed from 0.25
        local oldYPower = 0.25 -- changed from 0.5
		destroy_around( bullet, 20, bullet.owner )
		for i=0,spawns do
			local dx = power * math.cos( inc * i )
			local dy = power * math.sin( inc * i )
			spawn_bullet( create_normal_bullet( bullet.owner, bullet.x, bullet.y,
                                                oldXPower * bullet.vx + dx,
                                                oldYPower * bullet.vy + dy ) )
		end
	end
	return rv
end

create_bouncer = function(player, x, y, vx, vy)
	local rv = create_normal_bullet(player, x, y, vx, vy)
	rv.special_impacts = 11
	rv.bounce_explosion_radius = 12
	rv.special_impact = bounce_bullet
	return rv
end

create_tunneler = function(player, x, y, vx, vy)
	local rv = create_normal_bullet(player, x, y, vx, vy)
	rv.special_impacts = 20
	rv.special_impact = tunnel_bullet
	return rv
end

create_super_dirtball = function(player, x, y, vx, vy)
	local rv = create_normal_bullet(player, x, y, vx, vy)
	rv.explode_radius = 300
	rv.fill_with = 1
	return rv
end

create_dirtball = function(player, x, y, vx, vy)
	local rv = create_normal_bullet(player, x, y, vx, vy)
	rv.explode_radius = 55
	rv.fill_with = 1
	return rv
end

create_nuke = function(player, x, y, vx, vy)
	local rv = create_normal_bullet(player, x, y, vx, vy)
	rv.explode_radius = 150
	return rv
end

create_baby_nuke = function(player, x, y, vx, vy)
	local rv = create_normal_bullet(player, x, y, vx, vy)
	rv.explode_radius = 55
	return rv
end

create_normal_bullet = function(player, x, y, vx, vy)
    local rv = {}
    rv.x = x
    rv.y = y
    rv.vx = vx
    rv.vy = vy
	rv.detonate_on_descent = false
    rv.explode_radius = 12
    rv.fill_with = 0
	rv.special_impacts = 0
    rv.radius = 0.5 -- bullets 0.5, tanks 8, crates 5 is close to original
    rv.update = generic_update_bullet
    rv.apply_physics = generic_apply_bullet_physics
    rv.detonate = generic_bullet_detonate
    rv.owner = player
    return rv
end

make_weapon( "nuke", create_nuke, 20 )
make_weapon( "baby nuke", create_baby_nuke, 100 )
make_weapon( "dirtball", create_dirtball, 75 )
make_weapon( "super dirtball", create_super_dirtball, 15 )
make_weapon( "missile", create_normal_bullet, 0 )
make_weapon( "bouncer", create_bouncer, 100 )
make_weapon( "tunneler", create_tunneler, 75 )
make_weapon( "MIRV", create_mirv, 40 )
make_weapon( "cluster bomb", create_cluster_bomb, 120 )

is_opaque = function(x,y)
	local x = math.floor(0.5 + x)
	local y = math.floor(0.5 + y)
	return get_terrain_at( serverPointer, x, y) == 1
end

is_blank = function(x,y)
	return not is_opaque(x,y)
end

update_user_horizontal_movement = function(player, dx)
	local facingLeft = (dx < 0)
	local legal
	local xp = player.x + dx
	if facingLeft then legal = (player.x >= 10) else legal = (player.x < (terrainWidth -10)) end
	if is_blank(xp, player.y) and legal then
		player.x = xp
		return true
	elseif is_blank(xp, player.y - 1) and legal then
		player.x = xp
		player.y = player.y - 1
		return true
	elseif (is_blank(player.x, player.y-1) or 
			is_blank(player.x, player.y-2) or
			is_blank(player.x, player.y-3)) then
		player.y = player.y - 1
	end
	return false
end

update_user_movement = function(player)
	local lastX = player.x
	local lastY = player.y
	local a = player.angle
	local gravity = true
	if player.keys.left and player.keys.right then
		if player.laddersLeft > 0 then
			if is_opaque( player.x, player.y + 1 ) then
				if player.ladderTimeLeft > 0 then
					player.ladderTimeLeft = player.ladderTimeLeft - 1
				else
					player.launchLadder()
				end
			else
				player.ladderTimeLeft = ladderTime
			end
		end
	else
		player.ladderTimeLeft = ladderTime
		if player.keys.left then
			gravity = update_user_horizontal_movement(player, -1)
			if a > 0 then a = - a end
		end
		if player.keys.right then
			gravity = update_user_horizontal_movement(player, 1)
			if a < 0 then a = - a end
		end
	end
    player.angle = a
	player.vx = player.x - lastX
	player.vy = player.y - lastY
	return gravity
end

update_user_aim = function(player)
	local a = math.abs( player.angle )
	local facingLeft
	if player.angle < 0 then facingLeft = true else facingLeft = false end
	if player.keys.up then
		if a < 90 then a = a + 1 end
	end
	if player.keys.down then
		if a > 1 then a = a - 1 end
	end
	if facingLeft then player.angle = -a else player.angle = a end
end

update_user_physics = function(player, gravity)
	player.y = math.max( player.y, 20 )
	if gravity then
		if not is_opaque( player.x, player.y + 1 ) then player.y = player.y + 1 end
		if not is_opaque( player.x, player.y + 1 ) then player.y = player.y + 1 end
	end
end

update_user_fire = function(player)
	if player.keys.fire then
		player.firepower = math.min( 10, player.firepower + 0.1 )
	elseif player.firepower > 0 then
		player.fire()
		player.firepower = 0
	end
end

fire_bullet_from_tank = function(player, create_bullet)
    local firingAngle 
	local firingHeight = 7
	local barrelLength = 5.0
    if player.angle > 0 then
        firingAngle = math.pi * player.angle / 180
    else
        firingAngle = math.pi * (180 + player.angle) / 180
    end
	local bullet = create_bullet( player, 
                                  player.x + barrelLength * math.cos( firingAngle ),
                                  player.y - firingHeight - barrelLength * math.sin( firingAngle ),
                                  player.vx + player.firepower * math.cos( firingAngle ),
                                  player.vy - player.firepower * math.sin( firingAngle ) )
	table.insert( allBullets, bullet )
end

closer_than = function(alpha, beta, limit)
    local dx = alpha.x - beta.x
    local dy = alpha.y - beta.y
    local mr = alpha.radius + beta.radius + limit
    return (dx*dx + dy*dy) < (mr*mr)
end

apply_physics = function(obj, ax, ay, transition)
	local x0 = math.floor(0.5+obj.x)
	local y0 = math.floor(0.5+obj.y)
	obj.x = obj.x + obj.vx
	obj.y = obj.y + obj.vy
	obj.vx = obj.vx + ax
	obj.vy = obj.vy + ay
	local rv = {}
	local dx = x0 - math.floor(0.5+obj.x)
	local dy = y0 - math.floor(0.5+obj.y)
	local dxs
	if dx < 0 then dxs = -1 else dxs = 1 end
	local dys
	if dy < 0 then dys = -1 else dys = 1 end
	for i=x0,x0+dx,dxs do for j=y0,y0+dy,dys do
		transition( i, j )
	end end
end

generic_apply_bullet_physics = function(bullet)
	apply_physics( bullet, 0, 0.1, function(i,j) end )
end

create_ladder = function(player, x, y, vx, vy)
	local rv = {}
	rv.x = x
	rv.y = y
	rv.vx = vx
	rv.vy = vy
	rv.active = ladderLength
	rv.update = function(rv)
		rv.active = rv.active - 1
		rv.x = rv.vx + rv.x
		rv.y = rv.vy + rv.y
		if is_opaque( rv.x, rv.y ) then
			local x = rv.x
			local y = rv.y
			-- The original ladder also clears a TINY bit of land; not sure of reason
			local ly = y
			while ly < terrainHeight and is_opaque(x,ly) do ly = ly + 1 end
			while ly < terrainHeight and is_blank(x,ly) do ly = ly + 1 end
			local maxy = ly + 1
			ly = y
			while ly > 0 and is_opaque(x,ly) do ly = ly - 1 end
			local miny = ly
			for ly=miny,maxy,2 do
				set_terrain_at( serverPointer, x-1, ly, 0 )
				set_terrain_at( serverPointer, x, ly, 1 )
				set_terrain_at( serverPointer, x+1, ly, 0 )
				set_terrain_at( serverPointer, x-1, ly+1, 1 )
				set_terrain_at( serverPointer, x, ly+1, 1 )
				set_terrain_at( serverPointer, x+1, ly+1, 1 )
			end
			mark_terrain_dirty( serverPointer, x-1, miny, 3, maxy-miny+1 )
			return false
		end
		return rv.active > 0
	end
	return rv
end

scan_around = function(center, radius)
    for i,v in ipairs(connectedUsers) do
        if closer_than( center, v, radius ) then
            return true
        end
    end
    if closer_than( center, crateSingleton, radius ) then
        return true
    end
    return false
end

tunnel_bullet = function(bullet)
	local ool = 1 / math.sqrt( bullet.vx*bullet.vx + bullet.vy*bullet.vy )
	local dx = bullet.vx * ool
	local dy = bullet.vy * ool
	local vbull = { x = bullet.x + 8 * dx, y = bullet.y + 8 * dy, radius = bullet.radius }
	destroy_around( bullet, 9, bullet.owner )
	destroy_around( vbull, 9, bullet.owner )
end

bounce_bullet = function(bullet)
	local decay = 0.9
	local ool = 1 / math.sqrt( bullet.vx*bullet.vx + bullet.vy*bullet.vy )
	local dx = bullet.vx * ool * 0.25
	local dy = bullet.vy * ool * 0.25
	local bounce_x = false
	local bounce_y = false

	for i=0,40 do
		if is_blank( bullet.x, bullet.y ) then break end
		bullet.x = bullet.x - dx
		bullet.y = bullet.y - dy
	end

	for i=1,100 do
		if is_opaque( bullet.x + bullet.vx * i, bullet.y ) then bounce_x = true end
		if is_opaque( bullet.y, bullet.y + bullet.vy * i ) then bounce_y = true end
		if bounce_x or bounce_y then break end
	end

	if bounce_x then bullet.vx = - bullet.vx end
	if bounce_y then bullet.vy = - bullet.vy end
	bullet.vx = bullet.vx * decay
	bullet.vy = bullet.vy * decay

	if bullet.bounce_explosion_radius > 0 then
		destroy_around( bullet, bullet.bounce_explosion_radius, bullet.owner )
	end
end

generic_update_bullet = function(bullet)
    local detonate = false
	if bullet.detonate_on_descent and bullet.vy > 0 then
		detonate = true
	end
    if scan_around( bullet, 0 ) then
        detonate = true
    end
    if is_opaque( bullet.x, bullet.y ) then
		if bullet.special_impacts > 0 then
			bullet.special_impact( bullet )
			bullet.special_impacts = bullet.special_impacts - 1
		else
			detonate = true
		end
    end
    if detonate then
        bullet.detonate( bullet )
    else
        bullet.apply_physics( bullet )
    end
    return not detonate
end

destroy_around = function(center, radius, owner)
    for i,v in ipairs(connectedUsers) do
        if closer_than( center, v, radius ) then
            v.kill( owner )
        end
    end
    if closer_than( center, crateSingleton, radius ) then
        destroy_crate( owner, crateSingleton )
    end
	terrain_fill_circle( serverPointer, center.x, center.y, radius, 0)
end

generic_bullet_detonate = function(bullet)
    if bullet.fill_with == 0 then
        destroy_around( bullet, bullet.explode_radius, bullet.owner )
    else
        terrain_fill_circle( serverPointer, bullet.x, bullet.y, bullet.explode_radius, bullet.fill_with)
    end
end


update_user = function(i)
    local player = connectedUsers[i]
	local gravity = update_user_movement( player )
    if player.spawn_wait > 0 then
        player.spawn_wait = player.spawn_wait - 1
        if player.spawn_wait <= 0 then
            player.spawn()
        end
    end
	update_user_physics( player, gravity )
	update_user_aim( player )
	update_user_fire( player )
	set_tank_pos( player.tank, player.x, player.y )
	set_tank_angle( player.tank, player.angle )
    if closer_than( player, crateSingleton, 0 ) then
        collect_crate( player, crateSingleton )
    end
end

update_bullet = function(i)
    local bullet = allBullets[i]
    if not bullet.update( bullet ) then
        table.remove( allBullets, i )
    else
        put_bullet( serverPointer, math.floor(0.5 + bullet.x),  math.floor(0.5 + bullet.y))
    end
end

update = function()
	for i,v in ipairs( connectedUsers ) do update_user( i ) end
	for i,v in ipairs( allBullets ) do update_bullet( i ) end
    update_crate()
end

next_tank_id = function()
    local i = 0
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

get_random_crate_weapon = function()
	local total = 0
	for name, weapon in pairs( Weapons ) do
		total = total + weapon.frequency
	end
	total = math.random( total - 1 )
	for name, weapon in pairs( Weapons ) do
		total = total - weapon.frequency
		if total < 0 then
			return weapon
		end
	end
end

create_crate = function()
	crateSingleton = {}
	crateSingleton.hidden = true
    crateSingleton.radius = 5 -- assume a frictionless spherical crate
	return rv
end

respawn_crate = function()
	local c = crateSingleton
	c.hidden = false
	c.x = math.random( terrainWidth - 40 ) + 20
	c.y = 30
	c.contents = get_random_crate_weapon()
	terrain_fill_circle( serverPointer, c.x, c.y-12, 12, 0 )
	move_crate_to( serverPointer, c.x, c.y )
end

destroy_crate = function(player, crate)
	local vx = 0 -- oh come on sf :P (TODO special casing the bouncer)
    local vy = -0.2
    local weapon = crate.contents
	local bullet = weapon.bullet_creator( player, 
                                          crate.x,
                                          crate.y - 4,
                                          vx,
                                          vy )
    count_crate_kill( player, crate )
	despawn_crate()
    table.insert( allBullets, bullet )
end

collect_crate = function(player, crate)
    local weapon = crate.contents
    player.current_weapon = weapon
    broadcast_notice( serverPointer, string.format( ": %s picked up %s!", player.name, weapon.name ) )
    despawn_crate()
end

update_crate = function()
    local crate = crateSingleton
	if is_blank( crate.x, crate.y + 1 ) then
		crate.y = crate.y + 1
		move_crate_to( serverPointer, crate.x, crate.y )
	end
end

despawn_crate = function()
	respawn_crate()
end

initialize_server = function( srvptr, width, height )
	serverPointer = srvptr

	math.randomseed( os.time() )
	for i=0,100 do math.random() end -- for better randomness per lua docs

	terrainWidth = width
	terrainHeight = height

	for x=0,width do for y=0,height do
		if y < (height / 3) then
			set_terrain_at( serverPointer, x, y, 0 )
		else
			set_terrain_at( serverPointer, x, y, 1 )
		end
	end end

	terrain_fill_circle( serverPointer, width/2, height/2, 200, 1 )
	terrain_fill_circle( serverPointer, width/2, height/2, 180, 0 )

	create_crate()
	respawn_crate()
end


ConsoleCommands.select_weapon = function(player, arg)
--	if not require_admin( player ) then return end
	local weapons = {}
    for name,weapon in pairs( Weapons ) do
        weapons[ name ] = weapon.bullet_creator
    end
	weapons["ladder"] = create_ladder
	if weapons[ arg ] then
		player.current_weapon = weapons[ arg ]
	else
		player.current_weapon = create_normal_bullet
	end
end

spawn_bullet = function(bullet)
    table.insert( allBullets, bullet )
end

ConsoleCommands.weapon = ConsoleCommands.select_weapon

create_moag_user = function( userptr, id, keys )
    local rv = {}

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

	rv.deaths = 0
	rv.kills = 0
    rv.radius = 8
    rv.times_fired = 0
	rv.current_weapon = Weapons.missile
    rv.spawn_wait = 0

	rv.fire = function()
        count_player_fire( rv )
		fire_bullet_from_tank( rv, rv.current_weapon.bullet_creator )
        rv.current_weapon = Weapons.missile
	end

	rv.launchLadder = function()
		rv.laddersLeft = rv.laddersLeft - 1
		local ladder = create_ladder( rv, rv.x, rv.y, 0, -1 )
		table.insert( allBullets, ladder )
	end

	rv.spawn = function()
		totalSpawns = totalSpawns + 1
		rv.x = math.mod( (totalSpawns*240), (terrainWidth-40) ) + 20
		rv.y = 60
		rv.angle = 30
		rv.firepower = 0
		rv.vx = 0
		rv.vy = 0
		set_tank_pos( rv.tank, rv.x, rv.y )
		set_tank_angle( rv.tank, rv.angle )
		rv.ladderTimeLeft = ladderTime
		rv.laddersLeft = 1
	end

	rv.kill = function(killer)
        rv.x = -9000
        rv.y = -9000
		set_tank_pos( rv.tank, -9000, -9000 )
        rv.spawn_wait = respawnTime
        count_tank_kill( killer, rv )
	end

	rv.spawn()

	send_notice_to( serverPointer, userptr, string.format( "Welcome to MOAG, %s! %d users online.", rv.name, # connectedUsers ) )
	send_notice_to( serverPointer, userptr, string.format( "This is MOAG/%s version %s.", moagScriptName, moagScriptVersion ) )
	broadcast_notice( serverPointer, string.format( "A new challenger appears: %s has connected. %d users online.", rv.name, # connectedUsers ) );

    return rv
end

count_player_fire = function( player )
    player.times_fired = player.times_fired + 1
end

count_tank_kill = function( killer, victim )
    victim.deaths = victim.deaths + 1
    if not (killer == victim) then
        killer.kills = killer.kills + 1
        broadcast_notice( serverPointer, string.format( ": %s killed %s!",
            get_player_scored_name( killer ),
            get_player_scored_name( victim ) ) )
    else
        broadcast_notice( serverPointer, string.format( ": %s commmitted suicide.",
            get_player_scored_name( killer ) ) )
    end
end

count_crate_kill = function( killer, crate )
    local weapon = crate.contents
    broadcast_notice( serverPointer,
                      string.format( ": %s destroyed a crate containing %s!",
                      get_player_scored_name( killer ),
                      weapon.name ) )
end

get_player_scored_name = function( player )
    return string.format( "%s [%s]", player.name, get_player_score( player ) )
end

get_player_score = function( player )
    local kdr = nil
    local acc = nil
    if player.deaths > 0 then
        kdr = string.format( "K/D %.3f", player.kills / player.deaths )
    end
    if player.times_fired > 0 then
        local hits_per_k = player.kills * 1000 / player.times_fired
        acc = string.format( "accuracy %d", hits_per_k )
    end
    local rv = string.format( "kills %d, deaths %d", player.kills, player.deaths)
    if kdr then rv = rv .. ", " .. kdr end
    if acc then rv = rv .. ", " .. acc end
    return rv
end

destroy_moag_user = function( player )
	-- note: even after this is called, Lua will keep the player object around
	-- until it's ready to be GCed. This means e.g. that a bullet fired by the
	-- player won't contain a dangling pointer.
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
