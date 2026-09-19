# Discord Rich Presence

Official Discord RPC v3.4.0, compiled from source into a Windows x86 static library.
Source/release: https://github.com/discord/discord-rpc/releases/tag/v3.4.0
Archive: https://github.com/discord/discord-rpc/archive/refs/tags/v3.4.0.zip
The headers and Windows/common source files are unmodified upstream files.
License: MIT, included in `LICENSE`.
RapidJSON v1.1.0 headers are in `../rapidjson`, with their license in
`../rapidjson/license.txt`. Source: https://github.com/Tencent/rapidjson/tree/v1.1.0

This is Discord's deprecated legacy RPC library, used for its native Win32
support. It is not the current Discord Social SDK.

## Installation

Build CodMP after running `generate.bat`. Its `discord_rpc` project dependency
builds the SDK with the matching Debug/Release static C runtime and links it
into `CodMP.exe`. No Discord DLL is required. Include `discord-rpc-LICENSE.txt`
and `rapidjson-LICENSE.txt` when distributing the EXE; the post-build step
copies them to `build`. No cgame/UI replacement is needed.
The dedicated server has no Discord dependency.

The game still runs without Discord installed. Discord's desktop client must be running with activity
sharing enabled to display the presence. The SDK handles reconnecting if
Discord starts later. No Discord login, token, or client secret is needed.

## Settings

- `cl_discord 1`: enable (default); `cl_discord 0`: clear and disable.
- `cl_discordAppId 804794761696509972`: application ID (default).
- `cl_discordIcon main`: uploaded Rich Presence image asset key (default).

Changes apply while running. The existing application has a
Rich Presence asset named `main`; this is separate from its General Information
application icon. A replacement application needs its own asset key.

Presence updates every five seconds and shows menu/loading/gameplay state,
hostname without CoD color codes, map, game type, snapshot client count / server
capacity, and elapsed application time. Text is bounded and non-ASCII legacy
game bytes are stripped. No server IP, password, join secret, or account data
is published. Demo playback is labeled separately.

Runtime verification: check the Discord profile in menus, on a populated server,
after disconnecting, after `vid_restart`, after restarting Discord, and after
toggling `cl_discord`. Quitting the game shuts down the SDK. The integration's
display still needs an in-game check with the actual Discord desktop client.
