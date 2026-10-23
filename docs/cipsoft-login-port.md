# CipSoft login port from the adjacent OTClient

The default profile in `init.lua` uses
`https://www.tibia.com/clientservices/loginservice.php` on port 443.
`clientservices.php` is a separate service; character-list authentication uses
`loginservice.php`. The profile reproduces the supplied Qt 15.33 reference:

- Numeric protocol/client version: 1533.
- HTTPS `clientversion`: `15.33.9d148f`.
- TCP version string: `15.33.9d148fe0199b24ebf68217e93948da003f5063b6`.
- HTTP client type and TCP Qt OS: Windows 5, macOS 6, Linux 7.
- RSA: the existing `CIPSOFT_RSA` modulus.
- `assetversion`: `assetVersion` from the selected server profile; when unset or
  blank, the identifier read from the loaded asset files.

These values match the local reference client's known build; they are not a
claim that the live service will continue accepting this release indefinitely.
The gameplay parser `src/client/protocolgameparse.cpp` remains unchanged
(SHA-256 `4025867eaeed20b4da4540bd540198fb1276697360d8ee928815186a1eb96932`).
The 15.30 implementation was not replaced with the adjacent client's parser.

## Changes

Rubik already uses `HTTP.postJSON` through its Lua enter-game module. That path
now sends the version, revision, asset identifier, client type, operating-system
name, stay-logged state and optional 2FA token. Its existing 2FA dialog remains
in use. The unused native `LoginHttp` path was not selected or modified.

HTTPS URLs override the form/configuration port. The HTTPS-to-HTTP retry was
removed. The active ASIO HTTP path now verifies the certificate chain and host,
uses the shipped `cacert.pem` in addition to system roots, and selects TLS from
the HTTPS scheme rather than port 443 alone. Existing explicitly configured
HTTP services remain available.

Every attempt has a unique request ID. Replaced/cancelled attempts cannot
populate a new character list, even if their callbacks arrive later. The game
world host/port and session key are validated before opening the world.
`gameEndpoint` selects `protected` (default) or `unprotected` response fields.
World names remain the names returned by the service; preview is metadata.

The native Lua binding exposes `g_things.getAssetIdentifier`; the native game
login serializer calls `ProtocolGame:getLoginClientVersion()` for the full
revision. Oversized RSA payloads produce a controlled error. Asset identifiers
are trimmed so a newline does not become part of the transmitted hash.

Staticdata 15.33 has different field numbers from the legacy schema. Both
layouts are retained; version 1533 selects the separate modern message while
15.30 retains the original Rubik layout and house handling.

## Assets and configuration

The runtime catalog and all files referenced by the adjacent client's working
15.33 catalog were copied into `data/things/1533/` (5092 files including the
catalog and identifier). No existing version directory was overwritten.
The copied identifier is
`d28e52d02650671e7fcf52b4c14d0b53e7ad2e3d24ee0ea28b936610ba699047`.
The older, inconsistent `assets.json` and its backup were not copied. This
reuses the previously tested set; it is not a fresh official manifest audit.
Rubik's existing asset-path selection was preserved.

For a private 15.30 service, replace the single `Servers_init` entry, for example:

```lua
["http://127.0.0.1/login.php"] = {
    port = 80,
    protocol = 1530,
    rsa = "otserv",
    httpLogin = true,
    useAuthenticator = false
}
```

A custom CipSoft-compatible service can explicitly choose `rsa="cipsoft"`,
`clientRevision`, `os` and `gameEndpoint`. The revision is configured in `init.lua`; there is no hidden revision literal
in the login helper. Profiles without a revision retain numeric version strings.

## Validation

- `tests/lua/cipsoft_login_test.lua`: 70 checks against the real login functions,
  with only HTTP, UI and native engine boundaries replaced. Covers metadata,
  platform/RSA selection, token forwarding, endpoint parsing, cancellation,
  stale callbacks, session/world validation and the existing OTServer flow.
- `tests/lua/login_transition_test.lua`: existing transition regression passed.
- Lua 5.1 syntax checked for all changed Lua modules and `init.lua`.
- `tests/python/test_staticdata1533.py`: three tests passed, including the actual
  copied asset file (833 monsters, 21 categories, 368 achievements, 995 houses,
  447 bosses, 102 quests).

The full x64 Release build completed successfully with VS 2026/v145. The
result is `build/login-check/bin/otclient.exe`, copied to the project-root
`otclient.exe` for use with the existing `init.lua`, modules and asset directories.
An old executable lacks the asset identifier binding and full-revision serializer. No account credentials were used and no
live login was attempted by these tests.

The control build under `build/login-check` reuses dependencies already installed
in `../otclient/build/windows-release/vcpkg_installed`, with manifest installation
disabled. Rubik requires thread-pool 4.1.0 rather than that tree's 5.1.0; the
upstream 4.1.0 header is staged only under the build directory. Required Vulkan
headers were installed in the build's private `header-deps` directory. A normal
clean build should use Rubik's own `vcpkg.json` and baseline.

Source backups were created under the temporary directory before editing;
`build/login-check/login-port.patch` contains a reviewable diff of the initial
login port. The diff includes HTTP transport hardening and the new regression tests.


## Changing the hashes without recompiling

Edit the selected `Servers_init` entry in `init.lua`:

```lua
clientRevision = "9d148fe0199b24ebf68217e93948da003f5063b6",
assetVersion = "d28e52d02650671e7fcf52b4c14d0b53e7ad2e3d24ee0ea28b936610ba699047",
```

`clientRevision` is the full client build revision (40 hexadecimal characters),
not SHA-256 of `client.exe`. HTTP uses its first six characters; TCP uses it in
full. `assetVersion` is the login asset identifier from `assets.json.sha256`,
not SHA-256 of the individual `appearances-*.dat` file. Both HTTPS and TCP
resolve the same profile field. Surrounding whitespace is stripped.
Set `assetVersion = nil` or `assetVersion = ""` to send the loaded asset
identifier instead. Changing the configuration does not rewrite or validate
local asset files. Restart the client after editing; later hash changes do
not require recompiling the engine.

The initial implementation required a one-time rebuild to add the TCP Lua
callback `getLoginAssetVersion`; older executables always sent the file value.
Regression coverage changes both configured values and checks the actual HTTP
body and ProtocolGame callbacks, including the fallback to loaded assets.
