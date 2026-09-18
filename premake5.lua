--
-- premake5.lua -- generates the Visual Studio solution for opencod1.
--
--   generate.bat            -> vs2022 (default)
--   generate.bat vs2019     -> VS 2019, etc. (any premake vsXXXX action)
--
-- Ports tools/gen_sln.py.  The load-bearing retail settings it encoded are
-- reproduced verbatim below; the inline notes explain the ones that are
-- behavioural rather than cosmetic (the 8 MB no-ASLR stack especially).
--

-- Works from either layout without edits: the working tree (opencod1/src +
-- opencod1/build) or a flat public export (src + build at the repo root).
-- premake runs with the repo root as its cwd (generate.bat cd's there).
local base  = os.isdir("opencod1/src") and "opencod1/" or ""
local SRC   = base.."src"
local BUILD = base.."build"

-- Only these universal/ units are statically linked into each DLL image:
-- com_math.c and q_shared (our tree holds it as q_shared.c + q_parse.c), the
-- set tools/build_game.py establishes from the IDB's function folders.  The
-- exes, by contrast, link the whole of universal/.
local UNIVERSAL = {
    SRC.."/universal/com_math.c",
    SRC.."/universal/q_shared.c",
    SRC.."/universal/q_parse.c",
    SRC.."/universal/q_shared.h",
}

-- The bg_ units the cgame DLL statically links, built with /D CGAMEDLL (two
-- functions in bg_animation.c differ between the game and cgame images).
local CGAME_BG = {
    SRC.."/game_mp/bg_animation.c", SRC.."/game_mp/bg_misc.c",
    SRC.."/game_mp/bg_pmove.c",     SRC.."/game_mp/bg_slidemove.c",
    SRC.."/game_mp/bg_weapon.c",    SRC.."/game_mp/bg_public.h",
    SRC.."/game_mp/bg_local.h",
}

local function moduleGlobs()
    return { SRC.."/**.c", SRC.."/**.cpp", SRC.."/**.h", SRC.."/**.hpp" }
end

-- /DEF: for a DLL, absolute so it resolves regardless of the linker's cwd.
local function defOpt(rel)
    return "/DEF:\""..path.getabsolute(SRC.."/"..rel).."\""
end

workspace "opencod1"
    location (SRC)                       -- opencod1/src/opencod1.sln + the vcxproj
    configurations { "Debug", "Release" }
    platforms { "Win32" }
    architecture "x86"
    startproject "CodMP"

    -- ---- settings common to every project (gen_sln.py's ItemDefinitionGroup)
    language "C++"                       -- container; compileas C forces /TC
    compileas "C"                        -- /TC: every unit is C, incl. the .cpp
    characterset "MBCS"
    staticruntime "On"                   -- /MT  (+ /MTd in Debug, via runtime)
    symbols "On"                         -- /Zi in BOTH configs (PDB for scans)
    editandcontinue "Off"                --   ... /Zi, never /ZI
    incrementallink "Off"                -- LinkIncremental=false
    buildoptions { "/W1" }               -- WarningLevel=Level1
    defines {
        "WIN32", "_CRT_SECURE_NO_WARNINGS", "_CRT_NONSTDC_NO_DEPRECATE",
        "_WINSOCK_DEPRECATED_NO_WARNINGS",
    }
    includedirs { SRC.."/qcommon" }      -- qcommon on every project's -I
    libdirs { SRC.."/miles" }            -- mss32.lib for the #pragma comment(lib)
    targetdir (BUILD)                    -- exes: the game root, run in place
    objdir (BUILD.."/vs/%{prj.name}/%{cfg.buildcfg}")
    linkoptions { "/MAP" }               -- whereis.py reads the .map

    filter "configurations:Debug"
        defines { "_DEBUG" }
        runtime "Debug"
        optimize "Off"
    filter "configurations:Release"
        defines { "NDEBUG" }
        runtime "Release"
        optimize "Speed"                 -- /O2
        inlining "Explicit"              -- /Ob1: keep single-caller statics
        vectorextensions "IA32"          -- /arch:IA32: x87, not SSE
        linkoptions { "/OPT:REF", "/OPT:ICF" }   -- /DEBUG turns these off; re-arm
    filter {}

-- Load-bearing exe link flags.  Retail CoDMP.exe links /STACK:0x800000 and
-- predates ASLR and DEP (DllCharacteristics 0).  Not cosmetic: an 8 MB stack
-- cannot fit under the 0x00400000 image, so Windows places it ~0x021xxxxx, and
-- bit 25 of that address happens to be CONTENTS_BODY, which is the accident the
-- retail teammate-name crosshair trace passes on.  A 1 MB ASLR'd stack breaks
-- it.  See tools/gen_sln.py for the full trace.
local function exeLinkFlags()
    linkoptions { "/STACK:8388608", "/DYNAMICBASE:NO", "/NXCOMPAT:NO" }
end

local WIN_LIBS = { "kernel32","user32","gdi32","advapi32","winmm",
                   "wsock32","ole32","shell32" }
local DLL_LIBS = { "kernel32","user32" }

-- ============================ CodMP (client) ============================
project "CodMP"
    kind "WindowedApp"                   -- /subsystem:windows, WinMainCRTStartup
    targetname "CodMP"
    files (moduleGlobs())
    removefiles {
        SRC.."/game_mp/**", SRC.."/cgame_mp/**", SRC.."/ui_mp/**",
        SRC.."/null/**", SRC.."/Debug/**", SRC.."/Release/**",
        SRC.."/third_party/**",
    }
    files { SRC.."/win32/cod.rc" }       -- app icon (cod.rc -> cod.ico)
    links (WIN_LIBS)
    links { "discord_rpc", "psapi" }
    exeLinkFlags()

    postbuildcommands {
        '{COPYFILE} "'..path.getabsolute(SRC..'/third_party/discord-rpc/LICENSE')..'" "%{cfg.targetdir}/discord-rpc-LICENSE.txt"',
        '{COPYFILE} "'..path.getabsolute(SRC..'/third_party/rapidjson/license.txt')..'" "%{cfg.targetdir}/rapidjson-LICENSE.txt"',
    }

-- ========================= CodMP-ded (server) ==========================
project "CodMP-ded"
    kind "ConsoleApp"                    -- /subsystem:console (see gen_sln note)
    targetname "CodMP-ded"
    defines { "DEDICATED" }
    files (moduleGlobs())
    removefiles {
        SRC.."/renderer/**", SRC.."/client_mp/**", SRC.."/miles/**",
        SRC.."/botlib/**", SRC.."/game_mp/**", SRC.."/cgame_mp/**",
        SRC.."/ui_mp/**", SRC.."/Debug/**", SRC.."/Release/**",
        SRC.."/third_party/**",
        -- win32/ and xanim/ hold client and server units side by side; the
        -- render window, its input/sound/GL bindings and the GL VBO optimiser go.
        SRC.."/xanim/xmodel_optimize.c",
        SRC.."/win32/win_glimp.c", SRC.."/win32/win_gamma.c",
        SRC.."/win32/win_qgl.c",   SRC.."/win32/win_snd.c",
        SRC.."/win32/win_wndproc.c", SRC.."/win32/win_wndproc_main.c",
    }
    files { SRC.."/win32/cod.rc" }
    links (WIN_LIBS)
    exeLinkFlags()

-- ======================== Discord RPC (built in) =======================
project "discord_rpc"
    kind "StaticLib"
    compileas "C++"
    cppdialect "C++14"
    exceptionhandling "On"
    removelinkoptions { "/MAP", "/OPT:REF", "/OPT:ICF" }
    defines { "DISCORD_WINDOWS" }
    includedirs { SRC.."/third_party/discord-rpc", SRC.."/third_party/rapidjson/include" }
    files { SRC.."/third_party/discord-rpc/src/**.cpp", SRC.."/third_party/discord-rpc/src/**.h",
            SRC.."/third_party/discord-rpc/**.h" }

-- ============================ game_mp_x86.dll ==========================
project "game_mp"
    kind "SharedLib"
    targetname "game_mp_x86"
    targetprefix ""
    targetdir (BUILD.."/main")           -- fs_game=main loads the DLLs from here
    files { SRC.."/game_mp/**.c", SRC.."/game_mp/**.h" }
    files (UNIVERSAL)
    linkoptions { defOpt("game_mp/game_mp.def") }
    links (DLL_LIBS)

-- ========================== cgame_mp_x86.dll ==========================
project "cgame_mp"
    kind "SharedLib"
    targetname "cgame_mp_x86"
    targetprefix ""
    targetdir (BUILD.."/main")
    defines { "CGAMEDLL" }
    includedirs { SRC.."/cgame_mp", SRC.."/game_mp" }
    files { SRC.."/cgame_mp/**.c", SRC.."/cgame_mp/**.h" }
    files (UNIVERSAL)
    files (CGAME_BG)
    -- ui_mp/ is NOT taken whole (a second vmMain/dllEntry would break the link);
    -- cgame links ui_shared only, exactly what the manifest puts in its image.
    files { SRC.."/ui_mp/ui_shared_mp.c", SRC.."/ui_mp/ui_shared.h" }
    linkoptions { defOpt("cgame_mp/cgame_mp.def") }
    links (DLL_LIBS)

-- =========================== ui_mp_x86.dll ============================
project "ui_mp"
    kind "SharedLib"
    targetname "ui_mp_x86"
    targetprefix ""
    targetdir (BUILD.."/main")
    defines { "UIDLL" }
    includedirs { SRC.."/ui_mp" }
    files { SRC.."/ui_mp/**.c", SRC.."/ui_mp/**.h" }
    files (UNIVERSAL)
    linkoptions { defOpt("ui_mp/ui_mp.def") }
    links (DLL_LIBS)
