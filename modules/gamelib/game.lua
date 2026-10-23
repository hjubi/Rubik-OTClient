-- chunkname: @/gamelib/game.lua

function g_game.getRsa()
	return G.currentRsa
end

function g_game.findPlayerItem(itemId, subType, tier)
	local localPlayer = g_game.getLocalPlayer()

	if localPlayer then
		for slot = InventorySlotFirst, InventorySlotLast do
			local item = localPlayer:getInventoryItem(slot)

			if item and item:getId() == itemId and (subType == -1 or item:getSubType() == subType) then
				return item
			end
		end
	end

	return g_game.findItemInContainers(itemId, subType, tier or 0)
end

-- The server profile supplies the build revision. HTTPS uses its first six
-- characters; the game login uses the full revision.
function g_game.getLoginClientVersion(shortRevision)
    local version = g_game.getClientVersion()
    local server = Servers_init and Servers_init[G.host]
    local revision = server and server.clientRevision
    if type(revision) == 'string' then
        revision = revision:match('^%s*(.-)%s*$')
    end
    if not revision or revision == '' then
        return tostring(version)
    end
    if shortRevision then revision = revision:sub(1, 6) end
    return string.format('%d.%02d.%s', math.floor(version / 100), version % 100, revision)
end

-- Both HTTPS and TCP use the same configurable asset identifier.
function g_game.getLoginAssetVersion()
    local server = Servers_init and Servers_init[G.host]
    local identifier = server and server.assetVersion
    if type(identifier) == 'string' then
        identifier = identifier:match('^%s*(.-)%s*$')
        if identifier ~= '' then return identifier end
    end
    return g_things.getAssetIdentifier()
end

-- Resolve the transport from the URL before consulting the port in the UI.
-- The login service and the game world returned by it are separate endpoints.
function g_game.getLoginEndpoint(address, port, useHttp)
    local scheme, authority, path = address:match('^(%a+)://([^/?#]+)(.*)$')
    if scheme then
        scheme = scheme:lower()
        if scheme ~= 'http' and scheme ~= 'https' then
            return nil, 'Unsupported login URL scheme.'
        end
    end
    if not scheme then
        if address:find('://', 1, true) then
            return nil, 'Unsupported login URL scheme.'
        end
        authority, path = address:match('^([^/?#]+)(.*)$')
    end
    if not authority or authority:find('[@%s]') or path:find('#', 1, true) then
        return nil, 'Invalid login server address.'
    end

    local host, explicitPort
    if authority:sub(1, 1) == '[' then
        host, explicitPort = authority:match('^(%[[^%]]+%]):(%d+)$')
        if not host then host = authority:match('^(%[[^%]]+%])$') end
    else
        host, explicitPort = authority:match('^([^:]+):(%d+)$')
        if not host then host = authority:match('^([^:]+)$') end
    end
    port = tonumber(explicitPort) or (scheme and (scheme == 'https' and 443 or 80)) or tonumber(port)
    if not host or not port or port < 1 or port > 65535 or port ~= math.floor(port) then
        return nil, 'Invalid login server address or port.'
    end
    if path == '' then path = '/' end
    if path:sub(1, 1) == '?' then path = '/' .. path end
    return {
        host = host:lower(), path = path, port = port,
        useHttp = scheme == 'http' or (not scheme and port ~= 443 and useHttp == true)
    }
end

local function getQtPlatformOs()
    local os = g_app.getOs()
    return os == 'windows' and OsTypes.QtWindows or os == 'mac' and OsTypes.QtMac or OsTypes.QtLinux
end

local profileRsa

function g_game.chooseRsa(host)
    local server = Servers_init and Servers_init[host]
    if server and server.rsa then
        local rsa = server.rsa == 'cipsoft' and CIPSOFT_RSA or
                    server.rsa == 'otserv' and OTSERV_RSA or server.rsa
        g_game.setRsa(rsa, server.rsaExponent)
        g_game.setCustomOs(server.os or (rsa == CIPSOFT_RSA and g_game.getClientVersion() >= 1530 and getQtPlatformOs() or -1))
        profileRsa = rsa
        return
    end
    if profileRsa and G.currentRsa == profileRsa then
        g_game.setRsa(OTSERV_RSA)
        g_game.setCustomOs(-1)
    end
    profileRsa = nil
    if G.currentRsa ~= CIPSOFT_RSA and G.currentRsa ~= OTSERV_RSA then
        return
    end
    local endpoint = g_game.getLoginEndpoint(host, 7171, false)
    host = endpoint and endpoint.host or host:lower()
    if host == 'tibia.com' or host:ends('.tibia.com') or
       host == 'cipsoft.com' or host:ends('.cipsoft.com') then
        g_game.setRsa(CIPSOFT_RSA)

        if g_game.getClientVersion() >= 1530 then
            local appOs = g_app.getOs()
            g_game.setCustomOs(appOs == 'windows' and OsTypes.QtWindows or
                               appOs == 'mac' and OsTypes.QtMac or OsTypes.QtLinux)
        elseif g_app.getOs() == 'windows' then
            g_game.setCustomOs(OsTypes.Windows)
        else
            g_game.setCustomOs(OsTypes.Linux)
        end
    else
        if G.currentRsa == CIPSOFT_RSA then
            g_game.setCustomOs(-1)
        end
        g_game.setRsa(OTSERV_RSA)
    end

    -- Hack fix to resolve some 760 login issues
    if g_game.getClientVersion() <= 760 then
        g_game.setCustomOs(2)
    end
end

function g_game.setRsa(rsa, e)
	e = e or "65537"

	g_crypt.rsaSetPublicKey(rsa, e)

	G.currentRsa = rsa
end

function g_game.isOfficialTibia()
	return G.currentRsa == CIPSOFT_RSA
end

function g_game.getSupportedClients()
	return {
		740,
		741,
		750,
		755,
		760,
		770,
		772,
		780,
		781,
		782,
		790,
		792,
		800,
		810,
		811,
		820,
		821,
		822,
		830,
		831,
		840,
		842,
		850,
		853,
		854,
		855,
		857,
		860,
		861,
		862,
		870,
		871,
		900,
		910,
		920,
		931,
		940,
		943,
		944,
		951,
		952,
		953,
		954,
		960,
		961,
		963,
		970,
		971,
		972,
		973,
		980,
		981,
		982,
		983,
		984,
		985,
		986,
		1000,
		1001,
		1002,
		1010,
		1011,
		1012,
		1013,
		1020,
		1021,
		1022,
		1030,
		1031,
		1032,
		1033,
		1034,
		1035,
		1036,
		1037,
		1038,
		1039,
		1040,
		1041,
		1050,
		1051,
		1052,
		1053,
		1054,
		1055,
		1056,
		1057,
		1058,
		1059,
		1060,
		1061,
		1062,
		1063,
		1064,
		1070,
		1071,
		1072,
		1073,
		1074,
		1075,
		1076,
		1080,
		1081,
		1082,
		1090,
		1091,
		1092,
		1093,
		1094,
		1095,
		1096,
		1097,
		1098,
		1099,
		1100,
		1281,
		1285,
		1286,
		1287,
		1291,
		1300,
		1310,
		1311,
		1314,
		1316,
		1320,
		1321,
		1322,
		1332,
		1334,
		1336,
		1337,
		1340,
		1400,
		1405,
		1410,
		1412,
		1500,
		1501,
		1503,
		1510,
		1511,
		1520,
		1521,
		1530,
		1533
	}
end

function g_game.getClientProtocolVersion(client)
	local clients = {
		[982] = 974,
		[985] = 977,
		[981] = 973,
		[983] = 975,
		[1001] = 979,
		[1002] = 980,
		[984] = 976,
		[986] = 978,
		[980] = 971
	}

	return clients[client] or client
end

if not G.currentRsa then
	g_game.setRsa(OTSERV_RSA)
end

function g_game.getItemFrame(value)
	local frame = 0

	if value > 0 and value <= 999 then
		frame = 1
	elseif value > 999 and value <= 9999 then
		frame = 2
	elseif value > 9999 and value <= 99999 then
		frame = 3
	elseif value > 99999 and value <= 999999 then
		frame = 4
	elseif value > 1000000 then
		frame = 5
	end

	return frame
end

function g_game.getRectFrame(frame)
	if frame == 1 then
		return "0 0 32 32"
	elseif frame == 2 then
		return "32 0 32 32"
	elseif frame == 3 then
		return "64 0 32 32"
	elseif frame == 4 then
		return "96 0 32 32"
	elseif frame == 5 then
		return "128 0 32 32"
	elseif frame == 6 then
		return "160 0 32 32"
	end
end

function g_game.getVocationName(vocationId)
	if vocationId == 1 then
		return "Knight"
	elseif vocationId == 2 then
		return "Paladin"
	elseif vocationId == 3 then
		return "Sorcerer"
	elseif vocationId == 4 then
		return "Druid"
	elseif vocationId == 5 then
		return "Monk"
	elseif vocationId == 11 then
		return "Elite Knight"
	elseif vocationId == 12 then
		return "Royal Paladin"
	elseif vocationId == 13 then
		return "Master Sorcerer"
	elseif vocationId == 14 then
		return "Elder Druid"
	elseif vocationId == 15 then
		return "Exalted Monk"
	else
		return "None"
	end
end

function g_game.requestSelectCharacterTitle(titleId)
	titleId = tonumber(titleId) or 0

	if titleId < 0 then
		return
	end

	if g_game.getClientVersion and g_game.getClientVersion() < 1412 then
		return
	end

	if g_game.requestSetCharacterTitle then
		g_game.requestSetCharacterTitle(titleId)
	end
end

function g_game.onOtcToggle(opCode, enabled)
	return
end

-- Crosshair / aimed spell cast. The 4 crosshair spells (Divine Grenade/Barrage,
-- Ethereal Barrage, Death Echo) are server-side needPosition spells: they must carry the
-- cursor/target tile position in the say packet, otherwise the server casts them on the
-- caster's own tile. This is the missing glue: stage the aim, then talk normally - sendTalk
-- appends the staged position (15.30 say tail) and clears it.
--   mode 1 = release-on-tile, mode 2 = cast at cursor tile, mode 3 = target (no tile aim).
function g_game.talkSpell(words, mode, pos)
	if pos and pos.x and pos.x ~= 0 and pos.x ~= 65535 and g_game.setNextTalkAim then
		g_game.setNextTalkAim(pos)
	end

	g_game.talk(words)
end
