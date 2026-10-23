-- Exercise Rubik's actual Lua login flow with mocked UI and HTTP boundaries.
local checks = 0
local function eq(actual, expected)
    checks = checks + 1
    assert(actual == expected, 'check '..checks..': expected '..tostring(expected)..', got '..tostring(actual))
end
function string.ends(value, suffix) return value:sub(-#suffix) == suffix end
local appOs, version = 'windows', 1533
G = {account='test-account', password='test-password'}
OsTypes = {Windows=2, Linux=1, QtWindows=5, QtMac=6, QtLinux=7}
CIPSOFT_RSA, OTSERV_RSA = 'cipsoft-key', 'otserv-key'
g_app = {getOs=function() return appOs end}
g_crypt = {rsaSetPublicKey=function(key, exponent) G.key, G.exponent = key, exponent end}
g_game = {
    getClientVersion=function() return version end,
    setClientVersion=function(value) version = value end,
    setProtocolVersion=function(value) G.protocol = value end,
    setCustomOs=function(value) G.os=value end
}
dofile('modules/gamelib/game.lua')
ProtocolGame = {}
dofile('modules/gamelib/protocolgame.lua')
local url = 'https://www.tibia.com/clientservices/loginservice.php'
Servers_init = {[url]={rsa='cipsoft', protocol=1533, clientRevision='9d148fe0199b24ebf68217e93948da003f5063b6'}}
G.host, G.port = url, 7171
local endpoint = assert(g_game.getLoginEndpoint(url, 7171, true))
eq(endpoint.host, 'www.tibia.com'); eq(endpoint.port,443); eq(endpoint.useHttp,false)
eq(endpoint.path, '/clientservices/loginservice.php')
eq(g_game.getLoginEndpoint('https://bad.test:0/login', 80, true), nil)
eq(g_game.getLoginEndpoint('https://bad.test:abc/login', 80, true), nil)
eq(g_game.getLoginEndpoint('https://user@bad.test/login', 80, true), nil)
eq(g_game.getLoginEndpoint('ftp://bad.test/login', 80, true), nil)
eq(g_game.getLoginEndpoint('https://tibia.com', 7171, true).path, '/')
g_game.chooseRsa(url)
eq(G.key,CIPSOFT_RSA); eq(G.os,5); eq(G.exponent,'65537')
eq(g_game.getLoginClientVersion(true),'15.33.9d148f')
eq(ProtocolGame:getLoginClientVersion(),'15.33.9d148fe0199b24ebf68217e93948da003f5063b6')
for _, platform in ipairs({{'linux',7},{'mac',6},{'windows',5}}) do
    appOs=platform[1]; g_game.chooseRsa(url); eq(G.os,platform[2])
end
g_game.chooseRsa('https://tibia.com.evil.test/login')
eq(G.key,OTSERV_RSA); eq(G.os,-1)
g_game.chooseRsa(url)
local requests, cancelled, logs, boxes = {}, {}, {}, {}
HTTP = {
    postJSON=function(target, body, callback)
        requests[#requests+1] = {url=target, body=body, callback=callback}
        return #requests
    end,
    cancel=function(id) cancelled[id]=true end
}
g_things={getAssetIdentifier=function() return string.rep('d',64) end}
g_platform={getOSName=function() return 'Windows' end}
g_logger={info=function(message) logs[#logs+1]=message end}
modules={game_things={isLoaded=function() return true end}}
json={encode=function(value) return value end, decode=function(value) return value end}
SubscriptionStatus={Premium=1,Free=0}
local errorMessage, characters, account
function tr(message) return message end
function connect(box, handlers) box.handlers=handlers end
function displayCancelBox() local box={destroy=function() end}; boxes[#boxes+1]=box; return box end
function displayErrorBox(_, message) errorMessage=message; return {} end
dofile('modules/client_entergame/entergame.lua')
EnterGame.clearAccountFields=function() end
EnterGame.show=function() end
for index=1,30 do
    local name=debug.getupvalue(EnterGame.loginSuccess,index)
    if not name then break end
    if name=='onCharacterList' then
        debug.setupvalue(EnterGame.loginSuccess,index,function(_,value,acc) characters,account=value,acc end)
    end
end
EnterGame.tryHttpLogin(1533,true)
eq(requests[1].url,'https://www.tibia.com:443/clientservices/loginservice.php')
eq(requests[1].body.clientversion,'15.33.9d148f')
eq(requests[1].body.assetversion,string.rep('d',64))
eq(requests[1].body.clienttype,5); eq(requests[1].body.operatingsystem,'Windows')
eq(requests[1].body.email,'test-account'); eq(requests[1].body.password,'test-password')
eq(requests[1].body.type,'login'); eq(requests[1].body.stayloggedin,true)
local oldId=G.requestId
EnterGame.tryHttpLogin(1533,true,'123456')
eq(G.requestId,oldId+1); eq(cancelled[1],true)
eq(requests[2].body.token,'123456'); eq(requests[2].body.authenticatorToken,'123456')
requests[1].callback(nil,'stale failure'); eq(errorMessage,nil)
requests[2].callback(nil,'TLS failure')
eq(#requests,2); eq(errorMessage,'TLS failure') -- no downgrade to plaintext
for _,message in ipairs(logs) do
    eq(message:find('test-account',1,true),nil)
    eq(message:find('test-password',1,true),nil)
    eq(message:find('123456',1,true),nil)
end
-- Configuration changes must reach both wire paths without engine recompilation.
Servers_init[url].clientRevision = '  '..string.rep('a',40)..'  '
Servers_init[url].assetVersion = '  '..string.rep('b',64)..'\n'
EnterGame.tryHttpLogin(1533,true)
local configured = requests[#requests].body
eq(configured.clientversion,'15.33.aaaaaa')
eq(configured.assetversion,string.rep('b',64))
eq(ProtocolGame:getLoginClientVersion(),'15.33.'..string.rep('a',40))
eq(ProtocolGame:getLoginAssetVersion(),string.rep('b',64))
Servers_init[url].assetVersion = '  '
eq(g_game.getLoginAssetVersion(),string.rep('d',64))
eq(ProtocolGame:getLoginAssetVersion(),string.rep('d',64))
Servers_init[url].assetVersion = nil
Servers_init[url].clientRevision = '9d148fe0199b24ebf68217e93948da003f5063b6'
local function fixture()
    return {
        session={sessionkey='test-session', premiumuntil=0},
        playdata={characters={{name='Tester',worldid=7}},worlds={{id=7,name='Testworld',
            externaladdressprotected='protected.test',externalportprotected='7172',
            externaladdressunprotected='plain.test',externalportunprotected=7173,previewstate=1}}}
    }
end
errorMessage=nil
EnterGame.tryHttpLogin(1533,true)
requests[#requests].callback(fixture(),nil)
eq(characters[1].worldName,'Testworld'); eq(characters[1].worldIp,'protected.test')
eq(characters[1].worldPort,7172); eq(characters[1].previewState,true)
eq(G.sessionKey,'test-session'); eq(account.subStatus,SubscriptionStatus.Free)
Servers_init[url].gameEndpoint='unprotected'
EnterGame.tryHttpLogin(1533,true)
requests[#requests].callback(fixture(),nil)
eq(characters[1].worldIp,'plain.test'); eq(characters[1].worldPort,7173)
Servers_init[url].gameEndpoint=nil
for _,invalid in ipairs({'world','port','session'}) do
    local data=fixture(); errorMessage=nil
    if invalid=='world' then data.playdata.characters[1].worldid=99 end
    if invalid=='port' then data.playdata.worlds[1].externalportprotected=0 end
    if invalid=='session' then data.session.sessionkey='' end
    EnterGame.tryHttpLogin(1533,true); requests[#requests].callback(data,nil)
    eq(type(errorMessage),'string')
end
local shownTwoFactor=false
EnterGame.showTwoFactorWindow=function() shownTwoFactor=true end
EnterGame.tryHttpLogin(1533,true); requests[#requests].callback({errorCode=6},nil)
eq(shownTwoFactor,true)
EnterGame.tryHttpLogin(1533,true); local active=requests[#requests]
boxes[#boxes].handlers.onCancel()
errorMessage=nil; active.callback(fixture(),nil); eq(G.requestId,0); eq(errorMessage,nil)
-- Even an invalid new attempt must invalidate/cancel the previous HTTP callback.
EnterGame.tryHttpLogin(1533,true)
local superseded, supersededId = requests[#requests], #requests
G.host = 'https://bad.test:0/login'
EnterGame.tryHttpLogin(1533,true)
local endpointError = errorMessage
characters=nil
superseded.callback(fixture(),nil)
eq(cancelled[supersededId],true); eq(G.requestId,0)
eq(characters,nil); eq(errorMessage,endpointError)
-- Existing OTServer login remains HTTP with its original numeric version.
G.host,G.port='http://127.0.0.1/login.php',80
EnterGame.tryHttpLogin(1530,true)
eq(G.key,OTSERV_RSA); eq(G.os,-1)
eq(requests[#requests].url,'http://127.0.0.1:80/login.php')
eq(requests[#requests].body.clientversion,nil)
eq(ProtocolGame:getLoginClientVersion(),'1530')
print('PASS: '..checks..' Rubik login checks')
