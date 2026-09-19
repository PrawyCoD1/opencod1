#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <time.h>
#include "../qcommon/qcommon.h"
#include "../qcommon/cod1_globals.h"
#include "cl_records.h"
#include "cl_vm.h"
#include "cl_discord.h"
#include "../third_party/discord-rpc/discord_rpc.h"

extern int GetConfigString( int index, char *buf, int size );

static cvar_t *discordEnabled, *discordAppId, *discordIcon;
static int discordInitialized;
static DWORD nextUpdate;
static int attempted;
static int64_t startTime;

static const char *DiscordGametypeName( const char *type )
{
    static const struct { const char *code, *name; } types[] = {
        { "dm", "Deathmatch" }, { "tdm", "Team Deathmatch" },
        { "sd", "Search and Destroy" }, { "re", "Retrieval" },
        { "bel", "Behind Enemy Lines" }, { "hq", "Headquarters" },
        { "ctf", "Capture the Flag" }, { "zom", "Zombies" },
        { "zombies", "Zombies" }, { "gg", "Gun Game" },
        { "deathrun", "Deathrun" }, { "deathrun", "Deathrun" }
    };
    unsigned int i;
    for ( i = 0; i < sizeof(types) / sizeof(types[0]); ++i )
        if ( !Q_stricmp( type, types[i].code ) ) return types[i].name;
    return type;
}

static const char *DiscordMapImage( const char *map )
{
    /* Uploaded assets for application 804794761696509972. Unknown maps
     * retain the configured main image instead of requesting a missing asset. */
    static const char *maps[] = {
        "mp_ship", "mp_depot", "mp_rocket", "mp_brecourt", "mp_powcamp",
        "mp_chateau", "mp_dawnville", "mp_hurtgen", "mp_railyard",
        "mp_harbor", "mp_carentan", "mp_stanjel", "mp_neuville",
        "mp_tigertown", "mp_stalingrad", "german_town", "xp_hanoi",
        "gg_harbor_extra", "xp_standoff", "bx_300", "aim_map_lights",
        "nazi_zombies", "zom_city", "mp_deathrun_sweep", "mp_deathrun_long",
        "mp_deathrun_smartrun", "mp_deathrun_speed", "mp_deathrun_spike",
        "mp_deathrun_splash", "mp_deathrun_stacked", "mp_deathrun_portalv2",
        "dr_minecraft", "dr_speed", "dr_stacked", "dr_long", "dr_smartrun",
        "dr_sweep", "dr_spike", "dr_splash", "dr_portalv2", "dr_dupa",
        "dr_tunnel", "dr_road", "dr_circle", "dr_prison", "dr_mario",
        "dr_sky", "dr_breakout"
    };
    unsigned int i;
    if ( strcmp( discordAppId->string, "804794761696509972" ) ) return NULL;
    for ( i = 0; i < sizeof(maps) / sizeof(maps[0]); ++i )
        if ( !Q_stricmp( map, maps[i] ) ) return maps[i];
    return NULL;
}

/* CoD's color codes/control bytes are not Discord display text. */
static void DiscordText( char *out, int size, const char *in )
{
    int n = 0;
    while ( *in && n < size - 1 ) {
        unsigned char ch = (unsigned char)*in++;
        if ( ch == '^' && *in >= '0' && *in <= '9' ) {
            ++in;
        } else if ( ch >= 32 && ch < 127 ) {
            out[n++] = ch;
        }
    }
    out[n] = 0;
}

static void DiscordReady( const DiscordUser *user )
{
    (void)user;
    nextUpdate = 0;
}

void CL_DiscordShutdown( void )
{
    if ( discordInitialized ) {
        Discord_ClearPresence();
        Discord_Shutdown();
        discordInitialized = 0;
    }
    attempted = 0;
}

void CL_DiscordInit( void )
{
    discordEnabled = Cvar_Get( "cl_discord", "1", CVAR_ARCHIVE );
    discordAppId = Cvar_Get( "cl_discordAppId", "804794761696509972", CVAR_ARCHIVE );
    discordIcon = Cvar_Get( "cl_discordIcon", "main", CVAR_ARCHIVE );
    startTime = (int64_t)time( NULL );
}

static void DiscordStart( void )
{
    const char *id = discordAppId->string;
    DiscordEventHandlers handlers;

    attempted = 1;
    if ( strlen( id ) < 17 || strlen( id ) > 20 || strspn( id, "0123456789" ) != strlen( id ) ) {
        Com_Printf( "Discord: cl_discordAppId must be a numeric application ID.\n" );
        return;
    }
    memset( &handlers, 0, sizeof(handlers) );
    handlers.ready = DiscordReady;
    Discord_Initialize( id, &handlers, 0, NULL );
    discordInitialized = 1;
    nextUpdate = 0;
}

void CL_DiscordFrame( void )
{
    DiscordRichPresence presence;
    char info[BIG_INFO_STRING], hostname[128], map[64], type[32], imageText[128];
    int players, capacity;
    const char *mapImage;
    DWORD now;

    if ( !discordEnabled ) return;
    if ( discordEnabled->modified || discordAppId->modified ) {
        CL_DiscordShutdown();
        discordEnabled->modified = discordAppId->modified = qfalse;
    }
    if ( !discordEnabled->integer ) return;
    if ( !attempted ) DiscordStart();
    if ( !discordInitialized ) return;
    Discord_RunCallbacks();
    now = GetTickCount();
    if ( nextUpdate && (LONG)(now - nextUpdate) < 0 ) return;
    nextUpdate = now + 5000;
    memset( &presence, 0, sizeof(presence) );
    presence.startTimestamp = startTime;
    presence.largeImageKey = discordIcon->string;
    presence.largeImageText = "Call of Duty 1.1x Multiplayer";
    presence.details = "In main menu";
    if ( cls_state == CA_ACTIVE ) {
        info[0] = 0;
        GetConfigString( 0, info, sizeof(info) );
        DiscordText( hostname, sizeof(hostname), Info_ValueForKey(info, "sv_hostname") );
        DiscordText( map, sizeof(map), Info_ValueForKey(info, "mapname") );
        DiscordText( type, sizeof(type), Info_ValueForKey(info, "g_gametype") );
        capacity = atoi( Info_ValueForKey(info, "sv_maxclients") );
        players = cl_snap_valid ? *(int *)((byte *)&cl_snap_valid + SNAP_NUMCLIENTS) : 0;
        if ( players < 0 ) players = 0;
        if ( players > MAX_CLIENTS ) players = MAX_CLIENTS;
        if ( capacity < players ) capacity = players;
        if ( capacity > MAX_CLIENTS ) capacity = MAX_CLIENTS;
        if ( !map[0] ) Q_strncpyz( map, "Unknown map", sizeof(map) );
        if ( !type[0] ) Q_strncpyz( type, "Multiplayer", sizeof(type) );
        Com_sprintf( imageText, sizeof(imageText), "%s | %d/%d players", map, players, capacity );
        presence.largeImageText = imageText;
        mapImage = DiscordMapImage( map );
        if ( mapImage ) {
            presence.largeImageKey = mapImage;
            presence.smallImageKey = "main_small";
            presence.smallImageText = "Call of Duty 1.1x Multiplayer";
        }
        presence.details = hostname[0] ? hostname : "Playing multiplayer";
        presence.state = DiscordGametypeName(type);
        if ( clc_demoplaying ) presence.details = "Watching a demo";
    } else if ( cls_state >= CA_CONNECTING && cls_state < CA_ACTIVE ) {
        presence.details = "Connecting / loading";
    }
    Discord_UpdatePresence( &presence );
}
