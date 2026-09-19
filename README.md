# OpenCoD1

An open-source reconstruction of **Call of Duty** (2003) multiplayer.

> **Game content not included.** OpenCoD1 is source only and ships no game data. You
> need a legitimate copy of Call of Duty 1 (version 1.1), and its assets.

## Status

Reconstruction is **incomplete and ongoing.** The engine, dedicated server, and
the game/cgame/ui modules build and link; some subsystems (the effects system
especially) are still being reconstructed, so expect rough edges.

## Recommended Branches
Branch | Description
--------|--------
[main](https://github.com/opencod1/opencod1/tree/main) | Main branch with many enhancements and bug fixes, this is the most recommended one for a stable and improved modding/playing experience.
[vanilla](https://github.com/opencod1/opencod1/tree/vanilla) | Version of the main branch but with all new features and enhancements removed. Recommended for a vanilla Call of Duty 1.1 experience.

## Requirements

- Windows
- Visual Studio
- A legitimate copy of **Call of Duty 1, running version 1.1**

The project generator ([premake](https://premake.github.io/)) is bundled, so no
Python or other tooling is required.

## Building

1. **Clone** the repository.
2. Run **`generate.bat`** to generate the Visual Studio solution. To target a
   different toolset, pass the premake action, e.g. `generate.bat vs2019`.
3. Open **`src/opencod1.sln`** and build.

Output lands in:

| Target       | File                          |
| ------------ | ----------------------------- |
| Client       | `build/CodMP.exe`             |
| Dedicated    | `build/CodMP-ded.exe`         |
| Game module  | `build/main/game_mp_x86.dll`  |
| CGame module | `build/main/cgame_mp_x86.dll` |
| UI module    | `build/main/ui_mp_x86.dll`    |

## Installing & running

1. Copy the **`build/CodMP.exe`** into your Call of Duty 1 folder.
2. Copy the three DLLs from **`build/main/`** into the game's **`main/`** folder.
3. Run **`CodMP.exe`**.



