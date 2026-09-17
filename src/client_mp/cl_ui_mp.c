/*
 * @fidelity: likely
 * @fidelity-default: unreviewed
 */

#include "../qcommon/qcommon.h"
#include "../qcommon/hexrays_shim.h"
#include "../qcommon/cod1_globals.h"
#include "cl_records.h"
#include "../botlib/l_script.h"
#include "cl_refapi.h"
#include "cl_vm.h"
#include "../universal/com_sndalias.h"

vm_t    *uivm = NULL;               /* retail 0x0161747C */

#define UI_GETAPIVERSION        0   /* 0x00418F4C */
#define UI_INIT                 1   /* 0x00418F7C */
#define UI_SHUTDOWN             2   /* 0x00418E4B */
#define UI_KEY_EVENT            3   /* 0x0040DEA8, 0x0040DF48, 0x0040E02D */
#define UI_MOUSE_EVENT          4   /* 0x0040B0BE */
#define UI_REFRESH              5
#define UI_IS_FULLSCREEN        6
#define UI_SET_ACTIVE_MENU      7   /* 0x0040DE91 */
#define UI_GET_ACTIVE_MENU      8   /* 0x0040E012 */
#define UI_API_VERSION          7   /* 0x00418F56 -- UO is 9 */

extern int CIN_DrawCinematic();
extern int CIN_PlayCinematic();
extern int CIN_RunCinematic( int handle );
extern void CIN_SetExtents( int handle, int x, int y, int w, int h );
extern int CIN_StopCinematic();
extern int CL_GetAutoUpdate();
extern int CL_GetPing();
extern int CL_GetPingQueueCount();
extern int CL_ServerStatus();
extern int CL_UpdateVisiblePings_f( int source );
extern int *Com_FindSoundAlias( const char *name, int source );
void Com_LoadSoundAliases( const char *name, int source );  /* 0x00433E90 */
extern int Com_RealTime();
void Com_UnloadSoundAliasSounds( int source );   /* 0x00433E20 */
extern int FS_Delete();
extern int Key_ClearStates();
extern int Key_IsDown();
extern int Key_KeynumToString();
extern int Key_SetBinding();
extern int MSS_FadeAllSounds();
extern int MSS_PlayLocalSoundAlias();
extern const char *NET_AdrToString( netadr_t a );
extern qboolean NET_CompareAdr( netadr_t a, netadr_t b );
extern qboolean NET_StringToAdr( const char *s, netadr_t *a );
extern void SCR_UpdateScreen( void );
extern int SEH_GetLanguageName();
extern int SEH_StringEd_GetString();
extern int SEH_VerifyLanguageSelection();
/* retail 0x00463300 returns a Z_Malloc'd char *, NULL when the clipboard has no CF_TEXT. */
char *Sys_GetClipboardData( void );     /* 0x00463300 */

/* ---- GetClientState  0x00417410 ----  [CONFIRMED] */
char *__cdecl GetClientState(int a1)
{
  char *result;

  *(_DWORD *)(a1 + 4) = clc_connectPacketCount;
  *(_DWORD *)a1 = cls_state;
  strncpy((char *)(a1 + 12), cls_servername, 0x3FFu);
  *(_BYTE *)(a1 + 1035) = 0;
  strncpy((char *)(a1 + 1036), cls_updateInfoString, 0x3FFu);
  *(_BYTE *)(a1 + 2059) = 0;
  result = strncpy((char *)(a1 + 2060), clc_serverMessage, 0x3FFu);
  *(_BYTE *)(a1 + 3083) = 0;
  *(_DWORD *)(a1 + 8) = dword_1432A2C;
  return result;
}

/* ---- LAN_LoadCachedServers  0x00417490 ----  [CONFIRMED] */
void LAN_LoadCachedServers()
{
  fileHandle_t v0;
  fileHandle_t f;
  int Buffer;

  cls_numglobalservers = 0;
  cls_numfavoriteservers = 0;
  cls_numGlobalServerAddresses = 0;
  if ( FS_SV_FOpenFileRead("servercache.dat", &f) )
  {
    v0 = f;
    FS_Read(&cls_numglobalservers, 4, f);
    FS_Read(&cls_numfavoriteservers, 4, v0);
    FS_Read(&cls_numGlobalServerAddresses, 4, v0);
    FS_Read(&Buffer, 4, v0);
    if ( Buffer == 412672 )
    {
      FS_Read(&unk_1565008, 376832, v0);
      FS_Read(&unk_15C4010, 23552, v0);
      FS_Read(&unk_15C100C, 12288, v0);
    }
    else
    {
      cls_numglobalservers = 0;
      cls_numfavoriteservers = 0;
      cls_numGlobalServerAddresses = 0;
    }
    FS_FCloseFile(v0);
  }
}

/* ---- LAN_SaveServersToCache  0x00417570 ----  [CONFIRMED] */
void LAN_SaveServersToCache()
{
  fileHandle_t v0;
  int Buffer;

  v0 = FS_SV_FOpenFileWrite("servercache.dat");
  FS_Write(&cls_numglobalservers, 4, v0);
  FS_Write(&cls_numfavoriteservers, 4, v0);
  FS_Write(&cls_numGlobalServerAddresses, 4, v0);
  Buffer = 412672;
  FS_Write(&Buffer, 4, v0);
  FS_Write(&unk_1565008, 376832, v0);
  FS_Write(&unk_15C4010, 23552, v0);
  FS_Write(&unk_15C100C, 12288, v0);
  FS_FCloseFile(v0);
}

/* ---- LAN_ResetPings  0x00417600 ----  [CONFIRMED] */
_DWORD *__cdecl LAN_ResetPings(int a1)
{
  int v1;
  _DWORD *result;
  char *v3;
  int v4;

  if ( a1 )
  {
    v1 = a1 - 1;
    if ( !v1 )
    {
      v3 = (char *)&unk_1565008;
      v4 = 2048;
      goto LABEL_8;
    }
    result = (_DWORD *)(v1 - 1);
    if ( result )
      return result;
    v3 = (char *)&unk_15C4010;
  }
  else
  {
    v3 = (char *)&unk_155F404;
  }
  v4 = 128;
LABEL_8:
  result = v3 + 168;
  do
  {
    *result = -1;
    result += 46;
    --v4;
  }
  while ( v4 );
  return result;
}

/* ---- LAN_AddServer  0x00417640 ----  [HIGH] */
int __cdecl LAN_AddServer(int a1, char *Source, char *s)
{
  int v3;
  int *v4;
  int result;
  int v6;
  bool v7;
  char *v8;
  netadr_t v9;
  netadr_t v10;
  char *v11;
  netadr_t a;
  unsigned int v13;
  unsigned int retaddr;
  char *v15;

  v13 = retaddr ^ _security_cookie;
  v3 = 128;
  if ( a1 )
  {
    if ( a1 == 1 )
    {
      v3 = 2048;
      v4 = &cls_numglobalservers;
      v15 = (char *)&unk_1565008;
    }
    else
    {
      if ( a1 != 2 )
        return -1;
      v4 = &cls_numfavoriteservers;
      v15 = (char *)&unk_15C4010;
    }
  }
  else
  {
    v4 = &cls_numlocalservers;
    v15 = (char *)&unk_155F404;
  }
  if ( *v4 >= v3 )
    return -1;
  if ( NET_StringToAdr(s, &a) == qfalse )
    return -2;
  v6 = 0;
  if ( *v4 > 0 )
  {
    v11 = v15;
    do
    {
      qmemcpy(&v10, &a, sizeof(v10));
      qmemcpy(&v9, v11, sizeof(v9));
      if ( NET_CompareAdr(v9, v10) )
        break;
      v7 = ++v6 < *v4;
      v11 += 184;
    }
    while ( v7 );
  }
  if ( v6 < *v4 )
    return 0;
  qmemcpy(&v15[184 * *v4], &a, 0x14u);
  v8 = &v15[184 * *v4 + 20];
  strncpy(v8, Source, 0x1Fu);
  v8[31] = 0;
  result = 1;
  *(_DWORD *)&v15[184 * (*v4)++ + 172] = 1;
  return result;
}

/* ---- LAN_RemoveServer  0x004177D0 ----  VERIFIED */
void __cdecl LAN_RemoveServer( int source, const char *addr )
{
  int *count;
  char *servers;
  netadr_t comp;
  netadr_t entry;
  int i, j;

  switch ( source )
  {
    case 0:
      count = &cls_numlocalservers;
      servers = (char *)&unk_155F404;
      break;
    case 1:
      count = &cls_numglobalservers;
      servers = (char *)&unk_1565008;
      break;
    case 2:
      count = &cls_numfavoriteservers;
      servers = (char *)&unk_15C4010;
      break;
    default:
      return;
  }

  NET_StringToAdr( addr, &comp );

  for ( i = 0; i < *count; i++ )
  {
    qmemcpy( &entry, servers + 184 * i, sizeof( entry ) );
    if ( NET_CompareAdr( comp, entry ) )
    {
      for ( j = i; j < *count - 1; j++ )
        Com_Memcpy( servers + 184 * j, servers + 184 * ( j + 1 ), 184 );
      ( *count )--;
      return;
    }
  }
}

/* ---- LAN_GetServerAddressString  0x00417920 ----  [HIGH] */
char *__cdecl LAN_GetServerAddressString(
        char *result,
        unsigned int a2,
        int a3,
        char *Destination)
{
  const char *v4;
  const char *v5;
  const char *v6;
  netadr_t v7;

  if ( result )
  {
    if ( --result )
    {
      if ( !--result && a2 < 0x80 )
      {
        qmemcpy(&v7, (char *)&unk_15C4010 + 184 * a2, sizeof(v7));
        v4 = NET_AdrToString(v7);
        result = strncpy(Destination, v4, a3 - 1);
        Destination[a3 - 1] = 0;
        return result;
      }
    }
    else if ( a2 < 0x800 )
    {
      qmemcpy(&v7, (char *)&unk_1565008 + 184 * a2, sizeof(v7));
      v5 = NET_AdrToString(v7);
      result = strncpy(Destination, v5, a3 - 1);
      Destination[a3 - 1] = 0;
      return result;
    }
  }
  else if ( a2 < 0x80 )
  {
    qmemcpy(&v7, (char *)&unk_155F404 + 184 * a2, sizeof(v7));
    v6 = NET_AdrToString(v7);
    result = strncpy(Destination, v6, a3 - 1);
    Destination[a3 - 1] = 0;
    return result;
  }
  *Destination = 0;
  return result;
}

/* ---- LAN_GetServerInfo  0x00417A10 ----  [HIGH] */
char *__cdecl LAN_GetServerInfo(char *buf, char *source, unsigned int n, int buflen)
{
  char *v5;
  netadr_t adr;
  char Source[1024];

  Source[0] = 0;
  if ( source == (char *)1 )
  {
    if ( n >= 0x800 )
      goto LABEL_13;
    v5 = (char *)&unk_1565008 + 184 * n;
  }
  else if ( source == (char *)2 )
  {
    if ( n >= 0x80 )
      goto LABEL_13;
    v5 = (char *)&unk_15C4010 + 184 * n;
  }
  else if ( !source )
  {
    if ( n >= 0x80 )
      goto LABEL_13;
    v5 = (char *)&unk_155F404 + 184 * n;
  }
  else
  {
    goto LABEL_13;
  }
  if ( v5 )
  {
    if ( buf )
    {
      *buf = 0;
      Info_SetValueForKey(Source, "hostname", v5 + 0x14);
      Info_SetValueForKey(Source, "mapname", v5 + 0x34);
      Info_SetValueForKey(Source, "clients", va("%i", *(int *)(v5 + 0x98)));
      Info_SetValueForKey(Source, "sv_maxclients", va("%i", *(int *)(v5 + 0x9C)));
      Info_SetValueForKey(Source, "ping", va("%i", *(int *)(v5 + 0xA8)));
      Info_SetValueForKey(Source, "minping", va("%i", *(int *)(v5 + 0xA0)));
      Info_SetValueForKey(Source, "maxping", va("%i", *(int *)(v5 + 0xA4)));
      Info_SetValueForKey(Source, "game", v5 + 0x54);
      Info_SetValueForKey(Source, "gametype", v5 + 0x78);
      Info_SetValueForKey(Source, "nettype", va("%i", *(int *)(v5 + 0x74)));
      qmemcpy(&adr, v5, sizeof(adr));
      Info_SetValueForKey(Source, "addr", NET_AdrToString(adr));
      Info_SetValueForKey(Source, "sv_allowAnonymous", va("%i", *(int *)(v5 + 0xB0)));
      Info_SetValueForKey(Source, "pswrd", va("%i", *(int *)(v5 + 0xB4)));
      strncpy(buf, Source, buflen - 1);
      buf[buflen - 1] = 0;
    }
    return buf;
  }
LABEL_13:
  if ( buf )
    *buf = 0;
  return buf;
}

/* ---- LAN_GetServerPtr  0x00417CE0 ----  [CONFIRMED] */
char *__cdecl LAN_GetServerPtr(unsigned int a1, int a2)
{
  int v2;

  if ( a2 )
  {
    v2 = a2 - 1;
    if ( v2 )
    {
      if ( v2 == 1 && a1 < 0x80 )
        return (char *)&unk_15C4010 + 184 * a1;
    }
    else if ( a1 < 0x800 )
    {
      return (char *)&unk_1565008 + 184 * a1;
    }
  }
  else if ( a1 < 0x80 )
  {
    return (char *)&unk_155F404 + 184 * a1;
  }
  return 0;
}

/* ---- LAN_CompareServers  0x00417D40 ----  [HIGH] */
int __cdecl LAN_CompareServers(unsigned int a1, int a2, int a3, int a4, unsigned int a5)
{
  char *ServerPtr;
  int v6;
  char *v7;
  int result;
  int v9;
  int v10;
  int v11;
  int v12;

  v6 = (int)LAN_GetServerPtr(a1, a3);
  ServerPtr = LAN_GetServerPtr(a5, a3);
  v7 = ServerPtr;
  if ( !v6 || !ServerPtr )
    return 0;
  result = 0;
  switch ( a2 )
  {
    case 0:
      v9 = *(_DWORD *)(v6 + 180);
      if ( v9 == *((_DWORD *)v7 + 45) )
        result = 0;
      else
        result = 2 * (v9 != 0) - 1;
      break;
    case 1:
      result = Q_stricmp(v7 + 20, (const char *)(v6 + 20));
      break;
    case 2:
      result = Q_stricmp(v7 + 52, (const char *)(v6 + 52));
      break;
    case 3:
      v10 = *(_DWORD *)(v6 + 152);
      v11 = *((_DWORD *)v7 + 38);
      goto LABEL_14;
    case 4:
      v12 = Q_stricmp(v7 + 120, (const char *)(v6 + 120));
      if ( v12 >= 0 )
        result = v12 > 0;
      else
        result = -1;
      break;
    case 5:
      v10 = *(_DWORD *)(v6 + 168);
      v11 = *((_DWORD *)v7 + 42);
LABEL_14:
      if ( v10 >= v11 )
        result = v10 > v11;
      else
        result = -1;
      break;
    default:
      break;
  }
  if ( a4 )
  {
    if ( result >= 0 )
      return (result <= 0) - 1;
    else
      return 1;
  }
  return result;
}

/* ---- LAN_GetPingQueueCount  0x00417E40 ----  [CONFIRMED] */
int __cdecl LAN_GetPingQueueCount()
{
  return CL_GetPingQueueCount();
}

/* ---- LAN_ClearPing  0x00417E50 ----  [CONFIRMED] */
unsigned int __cdecl LAN_ClearPing(unsigned int result)
{
  if ( result < 0x10 )
  {
    result *= 1052;
    *(__int16 *)( (char *)unk_15CA660 + 0x12 + result ) = 0;
  }
  return result;
}

/* ---- LAN_GetPing  0x00417E70 ----  VERIFIED */
void __cdecl LAN_GetPing( int n, char *buf, int buflen, int *pingtime )
{
  CL_GetPing( n, buf, buflen, pingtime );
}

/* ---- LAN_GetPingInfo  0x00417E80 ----  [CONFIRMED] */
char *__cdecl LAN_GetPingInfo(int a1, char *a2, int a3)
{
  char *result;

  result = (char *)(1052 * a1);
  /* port at +0x12 and info at +0x1C, both inside unk_15CA660 (see LAN_ClearPing) */
  if ( *(__int16 *)( (char *)unk_15CA660 + 0x12 + (_DWORD)result ) )
  {
    result = strncpy(a2, (char *)unk_15CA660 + 0x1C + (_DWORD)result, a3 - 1);
    a2[a3 - 1] = 0;
  }
  else if ( a3 )
  {
    *a2 = 0;
  }
  return result;
}

/* ---- LAN_MarkServerVisible  0x00417EC0 ----  [HIGH] */
unsigned int __cdecl LAN_MarkServerVisible(unsigned int result, int a2, int a3)
{
  int v3;
  void *v4;

  if ( result == -1 )
  {
    v3 = 128;
    if ( a2 )
    {
      if ( a2 == 1 )
      {
        v4 = &unk_1565008;
        v3 = 2048;
      }
      else
      {
        result = a2 - 2;
        if ( a2 != 2 )
          return result;
        v4 = &unk_15C4010;
      }
    }
    else
    {
      v4 = &unk_155F404;
    }
    result = (unsigned int)v4 + 172;
    do
    {
      *(_DWORD *)result = a3;
      result += 184;
      --v3;
    }
    while ( v3 );
  }
  else if ( a2 )
  {
    if ( a2 == 1 )
    {
      if ( result < 0x800 )
      {
        result *= 184;
        *(int *)((char *)&unk_1565008 + 172 + result) = a3;
      }
    }
    else if ( a2 == 2 && result < 0x80 )
    {
      result *= 184;
      *(int *)((char *)&unk_15C4010 + 172 + result) = a3;
    }
  }
  else if ( result < 0x80 )
  {
    result *= 184;
    *(int *)((char *)&unk_155F404 + 172 + result) = a3;
  }
  return result;
}

/* ---- LAN_UpdateVisiblePings  0x00417FC0 ----  [CONFIRMED] */
int __cdecl LAN_UpdateVisiblePings( int source )
{
  return CL_UpdateVisiblePings_f( source );
}

/* ---- LAN_GetServerStatus  0x00417FD0 ----  [CONFIRMED] */
qboolean __cdecl LAN_GetServerStatus(char *Destination, char *Source, int a3)
{
  return CL_ServerStatus(Source, Destination, a3);
}

/* ---- CL_GetGlconfig  0x00417FE0 ----  [CONFIRMED] */
char **__cdecl CL_GetGlconfig(char **result)
{
  qmemcpy(result, &cls_glconfig, 0xA0u);
  return result;
}

/* ---- GetClipboardData_CL_ui  0x00418000 ----  VERIFIED */
void __cdecl GetClipboardData_CL_ui( char *buf, int buflen )
{
  char *cbd;

  cbd = Sys_GetClipboardData();
  if ( cbd )
  {
    strncpy( buf, cbd, buflen - 1 );
    buf[buflen - 1] = 0;
    free( cbd );
  }
  else
  {
    *buf = 0;
  }
}

/* ---- Key_KeynumToStringBuf  0x00418030 ----  VERIFIED */
char *__cdecl Key_KeynumToStringBuf( int keynum, char *buf, int buflen )
{
  char *value;

  value = Key_KeynumToString( keynum );
  strncpy( buf, value, buflen - 1 );
  buf[buflen - 1] = 0;
  return value;
}

/* ---- Key_GetBindingBuf  0x00418050 ----  VERIFIED */
void __cdecl Key_GetBindingBuf( int keynum, int buflen, char *buf )
{
  char *value;

  if ( keynum == -1 )
    value = (char *)&empty_string;
  else
    value = (char *)dword_142F780[3 * keynum + 2];

  if ( value )
  {
    strncpy( buf, value, buflen - 1 );
    buf[buflen - 1] = 0;
  }
  else
  {
    *buf = 0;
  }
}

/* ---- Key_GetCatcher  0x00418090 ----  [CONFIRMED] */
int Key_GetCatcher()
{
  return cls_keyCatchers;
}

/* ---- Key_SetCatcher  0x004180A0 ----  [CONFIRMED] */
int __cdecl Key_SetCatcher(int result)
{
  if ( (cls_keyCatchers & 1) != 0 )
    result |= 1u;
  cls_keyCatchers = result;
  return result;
}

/* ---- CLUI_GetCDKey  0x004180C0 ----  VERIFIED */
void __cdecl CLUI_GetCDKey( char *buf, int buflen, char *checksum, int checksumlen )
{
  cvar_t *fs;

  fs = Cvar_Get( "fs_game", (const char *)&empty_string, 24 );
  if ( UI_usesUniqueCDKey() && fs && fs->string[0] != 0 )
  {
    Com_Memcpy( buf, &cl_cdkey[16], 16 );
    buf[16] = 0;
    Com_Memcpy( checksum, &cl_cdkeychecksum[4], 4 );
    checksum[4] = 0;
  }
  else
  {
    Com_Memcpy( buf, cl_cdkey, 16 );
    buf[16] = 0;
    Com_Memcpy( checksum, cl_cdkeychecksum, 4 );
    checksum[4] = 0;
  }
}

/* ---- CLUI_SetCDKey  0x00418170 ----  VERIFIED */
void __cdecl CLUI_SetCDKey( const char *key, const char *checksum )
{
  cvar_t *fs;

  fs = Cvar_Get( "fs_game", (const char *)&empty_string, 24 );
  if ( UI_usesUniqueCDKey() && fs && fs->string[0] != 0 )
  {
    Com_Memcpy( &cl_cdkey[16], key, 16 );
    cl_cdkey[32] = 0;
    Com_Memcpy( &cl_cdkeychecksum[4], checksum, 4 );
    cl_cdkey[8] = 0;
    Com_WriteCDKey( fs->string, &cl_cdkey[16], &cl_cdkeychecksum[4] );
  }
  else
  {
    Com_Memcpy( cl_cdkey, key, 16 );
    Com_Memcpy( cl_cdkeychecksum, checksum, 4 );
    Com_WriteCDKey( "main", cl_cdkey, cl_cdkeychecksum );
  }
}

/* ---- GetConfigString  0x00418250 ----  VERIFIED */
int __cdecl GetConfigString( int index, char *buf, int size )
{
  int offset;

  if ( index < 0 || index >= 2048 )
    return 0;

  offset = cl_gameState_stringOffsets[index];
  if ( !offset )
  {
    if ( size )
      *buf = 0;
    return 0;
  }

  strncpy( buf, &cl_gameState_stringData[offset], size - 1 );
  buf[size - 1] = 0;
  return 1;
}

/* ---- GetClientname  0x00418290 ----  [HIGH] */
int __cdecl GetClientname(char *a1, int a2, size_t Count)
{
  int v4;
  byte *v5;

  *a1 = 0;
  if ( !cl_snap_valid )
    return 0;
  v4 = 0;
  if ( dword_1434A54 <= 0 )
    return 0;
  while ( 1 )
  {
    v5 = &byte_1531134[92 * (((_WORD)dword_1434A5C + (_WORD)v4) & 0x7FF)];
    if ( *(_DWORD *)v5 == a2 )
      break;
    if ( ++v4 >= dword_1434A54 )
      return 0;
  }
  strncpy(a1, (const char *)v5 + 60, Count);
  return 1;
}

/* ---- FloatAsInt_CL_ui  0x004182F0 ----  [CONFIRMED] */
int __cdecl FloatAsInt_CL_ui(int a1)
{
  return a1;
}

#define CA_ACTIVE   6

/* ---- VMF  no-address ----  VERIFIED */
static float VMF( int bits ) {
	union { int i; float f; } u;
	u.i = bits;
	return u.f;
}

/* ---- VMI  no-address ----  VERIFIED */
static int VMI( float value ) {
	union { int i; float f; } u;
	u.f = value;
	return u.i;
}

/* ---- CL_UITrapUnavailable  no-address ----  VERIFIED */
static void CL_UITrapUnavailable( int trap, const char *reason ) {
	static unsigned char reported[256];

	if ( trap >= 0 && trap < 256 ) {
		if ( reported[trap] ) {
			return;
		}
		reported[trap] = 1;
	}
	Com_Printf( "UI trap %i (0x%02X) unavailable: %s\n", trap, trap, reason );
}

/* ---- CL_UISystemCalls  0x00418300 ----  VERIFIED */
int CL_UISystemCalls( int *args ) {
	CL_RefApiIntegrityCheck( "CL_UISystemCalls" );

	switch ( args[0] ) {

	case 0x00:
		Com_Error( ERR_DROP, "\x15%s", (const char *) args[1] );
		return 0;
	case 0x01:
		Com_Printf( "%s", (const char *) args[1] );
		return 0;
	case 0x02:
		return SEH_GetLanguageName( args[1] );
	case 0x03:
		return SEH_VerifyLanguageSelection( args[1] );
	case 0x04:
		return Sys_Milliseconds();

	case 0x05:
		Cvar_Register( (vmCvar_t *) args[1], (const char *) args[2],
		               (const char *) args[3], args[4] );
		return 0;
	case 0x06:
		Cvar_Update( (vmCvar_t *) args[1] );
		return 0;
	case 0x07:
		Cvar_Set2( (const char *) args[1], (const char *) args[2], qtrue );
		return 0;
	case 0x08:
		return VMI( Cvar_VariableValue( (const char *) args[1] ) );
	case 0x09:
		Cvar_VariableStringBuffer( (const char *) args[1], (char *) args[2], args[3] );
		return 0;
	case 0x0A:
		Cvar_SetValue( (const char *) args[1], VMF( args[2] ) );
		return 0;
	case 0x0B:
		Cvar_Set2( (const char *) args[1], NULL, qfalse );
		return 0;
	case 0x0C:
		Cvar_Get( (const char *) args[1], (const char *) args[2], args[3] );
		return 0;
	case 0x0D:
		Cvar_InfoStringBuffer( args[1], (char *) args[2], args[3] );
		return 0;

	case 0x0E:  /* UI_ARGC -- inlined in retail; cmd_argc read at 0x0041843E */
		return Cmd_Argc();
	case 0x0F:
		Cmd_ArgvBuffer( args[1], (char *) args[2], args[3] );
		return 0;
	case 0x10:
		Cbuf_ExecuteText( args[1], (const char *) args[2] );
		return 0;

	case 0x11:
		return FS_FOpenFileByMode( (const char *) args[1], (int *) args[2], args[3] );
	case 0x12:
		FS_Read( (void *) args[1], args[2], args[3] );
		return 0;
	case 0x13:
		FS_Seek( args[1], args[2], args[3] );
		return 0;
	case 0x14:
		FS_Write( (const void *) args[1], args[2], args[3] );
		return 0;
	case 0x15:
		FS_FCloseFile( args[1] );
		return 0;
	case 0x16:
		return FS_GetFileList( (const char *) args[1], (const char *) args[2],
		                       (char *) args[3], args[4] );
	case 0x17:
		return FS_Delete( args[1] );

	case 0x18: case 0x19: case 0x1A: case 0x1B: case 0x1C: case 0x1D:
	case 0x1E: case 0x1F: case 0x20: case 0x21: case 0x22: case 0x23:
	case 0x42: case 0x43: case 0x44: case 0x45: case 0x46:
		if ( !cls_rendererBound ) {
			CL_UITrapUnavailable( args[0], "renderer not bound yet -- CL_InitRef has not run" );
			return 0;
		}
		switch ( args[0] ) {
		case 0x18:  /* UI_R_REGISTERMODEL re +0x0C */
			return re.RegisterModel( (const char *) args[1], args[2] );
		case 0x19:  /* UI_R_REGISTERSHADERNOMIP re +0x14 */
			return re.RegisterShaderNoMip( (const char *) args[1], args[2] );
		case 0x1A:  /* UI_R_CLEARSCENE re +0x3C */
			re.ClearScene();
			return 0;
		case 0x1B:  /* UI_R_ADDREFENTITYTOSCENE re +0x40 -- retail passes a literal 0 */
			re.AddRefEntityToScene( (const void *) args[1], 0 );
			return 0;
		case 0x1C:  /* UI_R_ADDPOLYTOSCENE re +0x44 */
			re.AddPolyToScene( args[1], args[2], (const void *) args[3] );
			return 0;
		case 0x1D:  /* UI_R_ADDPOLYSTOSCENE re +0x48 */
			re.AddPolysToScene( args[1], args[2], (const void *) args[3], args[4] );
			return 0;
		case 0x1E:  /* UI_R_ADDLIGHTTOSCENE re +0x4C */
			re.AddLightToScene( (const float *) args[1], VMF( args[2] ),
			                    VMF( args[3] ), VMF( args[4] ), VMF( args[5] ) );
			return 0;
		case 0x1F:  /* UI_R_ADDCORONATOSCENE re +0x54 -- retail passes a literal 1 last */
			re.AddCoronaToScene( (const float *) args[1], VMF( args[2] ),
			                     VMF( args[3] ), VMF( args[4] ), VMF( args[5] ),
			                     args[6], 1 );
			return 0;
		case 0x20:  /* UI_R_RENDERSCENE re +0x64 */
			re.RenderScene( (const void *) args[1] );
			return 0;
		case 0x21:  /* UI_R_SETCOLOR re +0x6C A POINTER to four floats, not four packed values: RE_SetColor (0x004DDCF0) dereferences it and substitutes a default when it is NULL. */
			re.SetColor( (const float *) args[1] );
			return 0;
		case 0x22:  /* UI_R_DRAWSTRETCHPIC re +0x70 */
			re.StretchPic( VMF( args[1] ), VMF( args[2] ), VMF( args[3] ),
			               VMF( args[4] ), VMF( args[5] ), VMF( args[6] ),
			               VMF( args[7] ), VMF( args[8] ), args[9] );
			return 0;
		case 0x23:  /* UI_R_MODELBOUNDS re +0x9C */
			re.ModelBounds( args[1], (float *) args[2], (float *) args[3] );
			return 0;

		case 0x42:  /* UI_R_REGISTERFONT re +0xA8, 4 args */
			return re.RegisterFont( args[1], args[2], args[3], args[4] );
		case 0x43:  /* UI_R_TEXT_WIDTH re +0xCC, 5 args */
			return re.Text_Width( args[1], args[2], args[3], 0, args[4] );
		case 0x44:  /* UI_R_TEXT_HEIGHT re +0xD0, 2 args */
			return re.Text_Height( args[1], args[2] );
		case 0x45:  /* UI_R_TEXT_PAINT re +0xD4, 9 args */
			return re.Text_Paint( args[1], args[2], args[3], args[4], args[5],
			                      args[6], args[7], args[8], args[9] );
		case 0x46:  /* UI_R_TEXT_PAINTWITHCURSOR re +0xE0, 11 args. args[8] is narrowed to its low byte at 0x00418A06 and a literal 0 follows it. */
			return re.Text_PaintWithCursor( args[1], args[2], args[3], args[4],
			                                args[5], args[6], args[7],
			                                (unsigned char) args[8], 0,
			                                args[9], args[10] );
		}
		return 0;

	case 0x24:
		SCR_UpdateScreen();
		return 0;

	case 0x26: {
		int *alias = Com_FindSoundAlias( (const char *) args[1], 0 );
		return alias ? *alias : 0;
	}
	case 0x27:
		MSS_PlayLocalSoundAlias( 0, args[1] );
		return 0;
	case 0x28:
		MSS_FadeAllSounds( args[1], args[2] );
		return 0;

	case 0x29:  /* UI_KEY_KEYNUM_TO_STRING_BUF -- retail case 41, 0x004186B8 */
		Key_KeynumToStringBuf( args[1], (char *) args[2], args[3] );
		return 0;
	case 0x2A:
		Key_GetBindingBuf( args[1], args[3], (char *) args[2] );
		return 0;
	case 0x2B:
		Key_SetBinding( args[1], args[2] );
		return 0;
	case 0x2C:
		return Key_IsDown( args[1] );
	/* 0x0142F628 is the word Key_GetOverstrikeMode (0x0040D480) and Key_SetOverstrikeMode (0x0040D490) own (key_overstrikeMode). */
	case 0x2D:  /* UI_KEY_GET_OVERSTRIKE_MODE -- inlined; 0x00418700 */
		return dword_142F628;
	case 0x2E:  /* UI_KEY_SET_OVERSTRIKE_MODE -- 0x00418710 */
		dword_142F628 = args[1];
		return 0;
	case 0x2F:
		Key_ClearStates();
		return 0;
	case 0x30:  /* UI_KEY_GET_CATCHER -- inlined; cls.keyCatchers at 0x00418726 */
		return Key_GetCatcher();
	case 0x31:
		Key_SetCatcher( args[1] );
		return 0;

	case 0x32:  /* UI_GET_CLIPBOARD_DATA -- retail case 50, 0x0041873F */
		GetClipboardData_CL_ui( (char *) args[1], args[2] );
		return 0;
	case 0x33:
		GetClientState( args[1] );
		return 0;
	case 0x34:
		memcpy( (void *) args[1], &cls_glconfig, 0xA0u );
		return 0;
	case 0x35:
		return GetConfigString( args[1], (char *) args[2], args[3] );
	case 0x36:
		return GetClientname( (char *) args[2], args[1], args[3] );

	case 0x3B:
		return LAN_GetPingQueueCount();
	case 0x3C:
		LAN_ClearPing( args[1] );
		return 0;
	case 0x3D:
		/* Retail case 61 (0x00418800) loads args[1] into EAX -- CL_GetPing's register argument n -- and pushes args[4], args[3], args[2], so the trap order is Q3's (n, buf, buflen, pingtime). */
		LAN_GetPing( args[1], (char *) args[2], args[3], (int *) args[4] );
		return 0;
	case 0x3E:
		LAN_GetPingInfo( args[1], (char *) args[2], args[3] );
		return 0;

	case 0x40:  /* UI_GET_CD_KEY -- retail case 64, 0x0041891D */
		CLUI_GetCDKey( (char *) args[1], args[2], (char *) args[3], args[4] );
		return 0;
	case 0x41:  /* UI_SET_CD_KEY -- retail case 65, 0x0041893A */
		CLUI_SetCDKey( (const char *) args[1], (const char *) args[2] );
		return 0;

	case 0x47:
		return SEH_StringEd_GetString( (const char *) args[1] );
	case 0x48:
		return (int) SEH_LocalizeTextMessage( (char *) args[1], (const char *) args[2], 0 );

	case 0x49:
		return PC_AddGlobalDefine( 0, (const char *) args[1] );
	case 0x4A: {
		int handle = PC_LoadSourceHandle( (const char *) args[1] );
		Com_DPrintf( "UI_PC_LOAD_SOURCE %s -> handle %i\n",
		             (const char *) args[1], handle );
		return handle;
	}
	case 0x4B:
		return PC_FreeSourceHandle( args[1] );
	case 0x4C:
		return PC_ReadTokenHandle( args[1], (pc_token_t *) args[2] );
	case 0x4D:
		return PC_SourceFileAndLine( args[1], (char *) args[2], (int *) args[3] );

	case 0x4E:
		return Com_RealTime( (int *) args[1] );

	case 0x50:
		LAN_GetServerAddressString( (char *) args[1], args[2], args[4], (char *) args[3] );
		return 0;
	case 0x51:
		LAN_GetServerInfo( (char *) args[3], (char *) args[1], args[2], args[4] );
		return 0;
	case 0x52:
		LAN_MarkServerVisible( args[2], args[1], args[3] );
		return 0;
	case 0x53:
		return CL_UpdateVisiblePings_f( args[1] );
	case 0x54:
		LAN_ResetPings( args[1] );
		return 0;
	case 0x55:
		LAN_LoadCachedServers();
		return 0;
	case 0x56:
		LAN_SaveServersToCache();
		return 0;
	case 0x57:
		return LAN_AddServer( args[1], (char *) args[2], (char *) args[3] );
	case 0x58:
		LAN_RemoveServer( args[1], (char *) args[2] );
		return 0;

	case 0x59:
		Com_DPrintf( "UI_CIN_PlayCinematic\n" );
		return CIN_PlayCinematic( (const char *) args[1], args[2], args[3],
		                          args[4], args[5], args[6] );
	case 0x5A:
		return CIN_StopCinematic( args[1] );
	case 0x5B:
		return CIN_RunCinematic( args[1] );
	case 0x5C:
		CIN_DrawCinematic( args[1] );
		return 0;
	case 0x5D:
		CIN_SetExtents( args[1], args[2], args[3], args[4], args[5] );
		return 0;

	case 0x5E:
		return CL_CDKeyValidate( (const char *) args[1], (const char *) args[2] );
	case 0x5F:
		return CL_ServerStatus( (const char *) args[1], (char *) args[2], args[3] );
	case 0x62:
		return LAN_CompareServers( args[4], args[2], args[1], args[3], args[5] );
	case 0x64:
		CL_GetAutoUpdate();
		return 0;

	case 0x3F:
		return *(int *) hunk_totalSize - *(int *) hunk_highTemp - *(int *) hunk_temp_temp;

	case 0x4F:
		switch ( args[1] ) {
		case 0: return cls_numlocalservers;
		case 1: return cls_numglobalservers;
		case 2: return cls_numfavoriteservers;
		default: return 0;
		}

	case 0x60: {
		char *server = LAN_GetServerPtr( args[2], args[1] );
		return server ? *(int *) ( server + 0xA8 ) : -1;
	}

	case 0x61: {
		char *server = LAN_GetServerPtr( args[2], args[1] );
		return server ? *(int *) ( server + 0xAC ) : 0;
	}

	case 0x63:
		return 0;

	case 0x65:
		return ( cls_state == CA_ACTIVE );

	case 0x66:
		return (int) memset( (void *) args[1], args[2], (size_t) args[3] );
	case 0x67:
		return (int) memcpy( (void *) args[1], (const void *) args[2], (size_t) args[3] );
	case 0x68:
		return (int) strncpy( (char *) args[1], (const char *) args[2], (size_t) args[3] );

	case 0x69:
		return VMI( (float) sin( VMF( args[1] ) ) );
	case 0x6A:
		return VMI( (float) cos( VMF( args[1] ) ) );
	case 0x6B:
		return VMI( (float) atan2( VMF( args[1] ), VMF( args[2] ) ) );
	case 0x6C:
		return VMI( (float) sqrt( VMF( args[1] ) ) );
	case 0x6D:
		return VMI( (float) floor( VMF( args[1] ) ) );
	case 0x6E:
		return VMI( (float) ceil( VMF( args[1] ) ) );

	case 0x6F:
		return (int) Z_MallocInternal( args[1] );
	case 0x70:
		free( (void *) args[1] );
		return 0;

	default:
		Com_Error( ERR_DROP, "\x15" "Bad UI system trap: %i", args[0] );
		return 0;
	}
}
#if 0
void __cdecl CL_UISystemCalls()
{
  int v0;
  int v1;
  int v2;
  const char *v3;
  char *v4;
  unsigned int v5;
  int v6;
  char *v7;
  char *v8;
  int v9;
  size_t Size;

  switch ( *(_DWORD *)Size )
  {
    case 0:
      Com_Error(ERR_DROP, "\x15%s", *(const char **)(Size + 4));
    case 1:
      Com_Printf("%s", *(const char **)(Size + 4));
      break;
    case 2:
      SEH_GetLanguageName(*(_DWORD *)(Size + 4));
      break;
    case 3:
      SEH_VerifyLanguageSelection(*(_DWORD *)(Size + 4));
      break;
    case 4:
      Sys_Milliseconds();
      break;
    case 5:
      Cvar_Register(
        *(vmCvar_t **)(Size + 4),
        *(const char **)(Size + 8),
        *(const char **)(Size + 12),
        *(_DWORD *)(Size + 16));
      break;
    case 6:
      Cvar_Update(*(vmCvar_t **)(Size + 4));
      break;
    case 7:
      Cvar_Set2(*(const char **)(Size + 4), *(const char **)(Size + 8), qtrue);
      break;
    case 8:
      Cvar_VariableValue(*(const char **)(Size + 4));
      break;
    case 9:
      Cvar_VariableStringBuffer(*(const char **)(Size + 4), *(char **)(Size + 8), *(_DWORD *)(Size + 12));
      break;
    case 0xA:
      Cvar_SetValue(*(const char **)(Size + 4), *(float *)(Size + 8));
      break;
    case 0xB:
      Cvar_Set2(*(const char **)(Size + 4), 0, qfalse);
      break;
    case 0xC:
      Cvar_Get(*(const char **)(Size + 4), *(const char **)(Size + 8), *(_DWORD *)(Size + 12));
      break;
    case 0xD:
      Cvar_InfoStringBuffer(*(_DWORD *)(Size + 4), *(char **)(Size + 8), *(_DWORD *)(Size + 12));
      break;
    case 0xE:
    case 0x2D:
    case 0x30:
    case 0x3F:
    case 0x4F:
    case 0x60:
    case 0x61:
    case 0x63:
    case 0x65:
    case 0x69:
    case 0x6A:
    case 0x6B:
    case 0x6C:
      return;
    case 0xF:
      Cmd_ArgvBuffer(*(_DWORD *)(Size + 4), *(char **)(Size + 8), *(_DWORD *)(Size + 12));
      break;
    case 0x10:
      Cbuf_ExecuteText(*(_DWORD *)(Size + 4), *(const char **)(Size + 8));
      break;
    case 0x11:
      FS_FOpenFileByMode(*(const char **)(Size + 4), *(fileHandle_t **)(Size + 8), *(fsMode_t *)(Size + 12));
      break;
    case 0x12:
      FS_Read(*(void **)(Size + 4), *(_DWORD *)(Size + 8), *(_DWORD *)(Size + 12));
      break;
    case 0x13:
      FS_Seek(*(_DWORD *)(Size + 4), *(_DWORD *)(Size + 8), *(_DWORD *)(Size + 12));
      break;
    case 0x14:
      FS_Write(*(const void **)(Size + 4), *(_DWORD *)(Size + 8), *(_DWORD *)(Size + 12));
      break;
    case 0x15:
      FS_FCloseFile(*(_DWORD *)(Size + 4));
      break;
    case 0x16:
      FS_GetFileList(*(const char **)(Size + 8), *(const char **)(Size + 12), *(char **)(Size + 16), (int)v7);
      break;
    case 0x17:
      FS_Delete(v7);
      break;
    case 0x18:
      dword_143286C(*(_DWORD *)(Size + 4), *(_DWORD *)(Size + 8));
      break;
    case 0x19:
      Material_RegisterHandle(*(_DWORD *)(Size + 4), *(_DWORD *)(Size + 8));
      break;
    case 0x1A:
      ((void (__cdecl *)())dword_143289C)();
      break;
    case 0x1B:
      dword_14328A0(*(_DWORD *)(Size + 4), 0);
      break;
    case 0x1C:
      dword_14328A4(*(_DWORD *)(Size + 4), *(_DWORD *)(Size + 8), *(_DWORD *)(Size + 12));
      break;
    case 0x1D:
      dword_14328A8(*(_DWORD *)(Size + 4), *(_DWORD *)(Size + 8), *(_DWORD *)(Size + 12), *(_DWORD *)(Size + 16));
      break;
    case 0x1E:
      dword_14328AC(
        *(_DWORD *)(Size + 4),
        *(_DWORD *)(Size + 8),
        *(_DWORD *)(Size + 12),
        *(_DWORD *)(Size + 16),
        *(_DWORD *)(Size + 20));
      break;
    case 0x1F:
      dword_14328B4(
        *(_DWORD *)(Size + 4),
        *(_DWORD *)(Size + 8),
        *(_DWORD *)(Size + 12),
        *(_DWORD *)(Size + 16),
        *(_DWORD *)(Size + 20),
        *(_DWORD *)(Size + 24),
        1);
      break;
    case 0x20:
      dword_14328C4(*(_DWORD *)(Size + 4));
      break;
    case 0x21:
      re_SetColor(*(_DWORD *)(Size + 4));
      break;
    case 0x22:
      re_DrawStretchPic(
        *(_DWORD *)(Size + 4),
        *(_DWORD *)(Size + 8),
        *(_DWORD *)(Size + 12),
        *(_DWORD *)(Size + 16),
        *(_DWORD *)(Size + 20),
        *(_DWORD *)(Size + 24),
        *(_DWORD *)(Size + 28),
        *(_DWORD *)(Size + 32),
        *(_DWORD *)(Size + 36));
      break;
    case 0x23:
      dword_14328FC(*(_DWORD *)(Size + 4), *(_DWORD *)(Size + 8), *(_DWORD *)(Size + 12));
      break;
    case 0x24:
      SCR_UpdateScreen();
      break;
    case 0x26:
      Com_FindSoundAlias(v7, (int)v8);
      break;
    case 0x27:
      MSS_PlayLocalSoundAlias(0, *(_DWORD *)(Size + 4));
      break;
    case 0x28:
      MSS_FadeAllSounds(*(_DWORD *)(Size + 4), *(_DWORD *)(Size + 8));
      break;
    case 0x29:
      Key_KeynumToStringBuf(*(char **)(Size + 8), *(_DWORD *)(Size + 12));
      break;
    case 0x2A:
      Key_GetBindingBuf(*(_DWORD *)(Size + 4), *(_DWORD *)(Size + 12), *(char **)(Size + 8));
      break;
    case 0x2B:
      Key_SetBinding((int)v7, v8);
      break;
    case 0x2C:
      Key_IsDown(Size);
      break;
    case 0x2E:
      dword_142F628 = *(_DWORD *)(Size + 4);
      break;
    case 0x2F:
      Key_ClearStates();
      break;
    case 0x31:
      Key_SetCatcher(*(_DWORD *)(Size + 4));
      break;
    case 0x32:
      GetClipboardData_CL_ui((UINT)v7);
      break;
    case 0x33:
      GetClientState(*(_DWORD *)(Size + 4));
      break;
    case 0x34:
      qmemcpy(*(void **)(Size + 4), &cls_glconfig, 0xA0u);
      break;
    case 0x35:
      GetConfigString(*(_DWORD *)(Size + 4), *(char **)(Size + 8), *(_DWORD *)(Size + 12));
      break;
    case 0x36:
      GetClientname(*(char **)(Size + 8), *(_DWORD *)(Size + 4), *(_DWORD *)(Size + 12));
      break;
    case 0x3B:
      CL_GetPingQueueCount();
      break;
    case 0x3C:
      LAN_ClearPing(*(_DWORD *)(Size + 4));
      break;
    case 0x3D:
      CL_GetPing(*(_DWORD *)(Size + 8), *(char **)(Size + 12), *(_DWORD *)(Size + 16), (int *)v7);
      break;
    case 0x3E:
      LAN_GetPingInfo(*(_DWORD *)(Size + 4), *(char **)(Size + 8), *(_DWORD *)(Size + 12));
      break;
    case 0x40:
      CLUI_GetCDKey(*(_DWORD *)(Size + 4), *(_DWORD *)(Size + 12));
      break;
    case 0x41:
      CLUI_SetCDKey(*(_DWORD **)(Size + 4), *(_DWORD **)(Size + 8));
      break;
    case 0x42:
      dword_1432908(*(_DWORD *)(Size + 4), *(_DWORD *)(Size + 8), *(_DWORD *)(Size + 12), *(_DWORD *)(Size + 16));
      break;
    case 0x43:
      dword_143292C(*(_DWORD *)(Size + 4), *(_DWORD *)(Size + 8), *(_DWORD *)(Size + 12), 0, *(_DWORD *)(Size + 16));
      break;
    case 0x44:
      dword_1432930(*(_DWORD *)(Size + 4), *(_DWORD *)(Size + 8));
      break;
    case 0x45:
      dword_1432934(
        *(_DWORD *)(Size + 4),
        *(_DWORD *)(Size + 8),
        *(_DWORD *)(Size + 12),
        *(_DWORD *)(Size + 16),
        *(_DWORD *)(Size + 20),
        *(_DWORD *)(Size + 24),
        *(_DWORD *)(Size + 28),
        *(_DWORD *)(Size + 32),
        *(_DWORD *)(Size + 36));
      break;
    case 0x46:
      dword_1432940(
        *(_DWORD *)(Size + 4),
        *(_DWORD *)(Size + 8),
        *(_DWORD *)(Size + 12),
        *(_DWORD *)(Size + 16),
        *(_DWORD *)(Size + 20),
        *(_DWORD *)(Size + 24),
        *(_DWORD *)(Size + 28),
        *(unsigned __int8 *)(Size + 32),
        0,
        *(_DWORD *)(Size + 36),
        *(_DWORD *)(Size + 40));
      break;
    case 0x47:
      SEH_StringEd_GetString(*(const char **)(Size + 4));
      break;
    case 0x48:
      SEH_LocalizeTextMessage(*(char **)(Size + 4), *(const char **)(Size + 8), 0);
      break;
    case 0x49:
      PC_AddGlobalDefine(v0, *(const char **)(Size + 4));
      break;
    case 0x4A:
      PC_LoadSourceHandle(v2, v3, *(char **)(Size + 4));
      break;
    case 0x4B:
      PC_FreeSourceHandle(*(_DWORD *)(Size + 4));
      break;
    case 0x4C:
      PC_ReadTokenHandle(*(_DWORD *)(Size + 8), *(_DWORD *)(Size + 4));
      break;
    case 0x4D:
      PC_SourceFileAndLine(*(char **)(Size + 8), *(_DWORD **)(Size + 12), *(_DWORD *)(Size + 4));
      break;
    case 0x4E:
      Com_RealTime(*(int **)(Size + 4));
      break;
    case 0x50:
      LAN_GetServerAddressString(
        *(char **)(Size + 4),
        *(_DWORD *)(Size + 8),
        *(_DWORD *)(Size + 16),
        *(char **)(Size + 12));
      break;
    case 0x51:
      LAN_GetServerInfo(*(char **)(Size + 12), *(char **)(Size + 4), *(_DWORD *)(Size + 8), *(_DWORD *)(Size + 16));
      break;
    case 0x52:
      LAN_MarkServerVisible(*(_DWORD *)(Size + 8), *(_DWORD *)(Size + 4), *(_DWORD *)(Size + 12));
      break;
    case 0x53:
      CL_UpdateVisiblePings_f();
      break;
    case 0x54:
      LAN_ResetPings(*(_DWORD *)(Size + 4));
      break;
    case 0x55:
      LAN_LoadCachedServers();
      break;
    case 0x56:
      LAN_SaveServersToCache();
      break;
    case 0x57:
      LAN_AddServer(*(_DWORD *)(Size + 4), *(char **)(Size + 8), *(char **)(Size + 12));
      break;
    case 0x58:
      LAN_RemoveServer(*(_DWORD *)(Size + 4), *(char **)(Size + 8));
      break;
    case 0x59:
      Com_DPrintf("UI_CIN_PlayCinematic\n");
      CIN_PlayCinematic(
        *(const char **)(Size + 4),
        *(_DWORD *)(Size + 8),
        *(_DWORD *)(Size + 12),
        *(_DWORD *)(Size + 16),
        *(_DWORD *)(Size + 20),
        *(_DWORD *)(Size + 24));
      break;
    case 0x5A:
      CIN_StopCinematic(Size);
      break;
    case 0x5B:
      CIN_RunCinematic(*(_DWORD *)(Size + 4));
      break;
    case 0x5C:
      CIN_DrawCinematic(*(_DWORD *)(Size + 4));
      break;
    case 0x5D:
      CIN_SetExtents(
        *(_DWORD *)(Size + 4),
        *(_DWORD *)(Size + 12),
        *(_DWORD *)(Size + 8),
        *(_DWORD *)(Size + 16),
        *(_DWORD *)(Size + 20));
      break;
    case 0x5E:
      CL_CDKeyValidate(v7, v8);
      break;
    case 0x5F:
      CL_ServerStatus(*(const char **)(Size + 4), *(char **)(Size + 8), *(_DWORD *)(Size + 12));
      break;
    case 0x62:
      LAN_CompareServers(
        *(_DWORD *)(Size + 16),
        *(_DWORD *)(Size + 8),
        *(_DWORD *)(Size + 4),
        *(_DWORD *)(Size + 12),
        *(_DWORD *)(Size + 20));
      break;
    case 0x64:
      CL_GetAutoUpdate();
      break;
    case 0x66:
      v4 = *(char **)(Size + 4);
      LOBYTE(v1) = *(_BYTE *)(Size + 8);
      BYTE1(v1) = v1;
      v5 = *(_DWORD *)(Size + 12);
      v6 = v1 << 16;
      LOWORD(v6) = v1;
      memset32(v4, v6, v5 >> 2);
      memset(&v4[4 * (v5 >> 2)], v1, v5 & 3);
      break;
    case 0x67:
      qmemcpy(*(void **)(Size + 4), *(const void **)(Size + 8), *(_DWORD *)(Size + 12));
      break;
    case 0x68:
      strncpy(*(char **)(Size + 4), *(const char **)(Size + 8), *(_DWORD *)(Size + 12));
      break;
    case 0x6D:
      floor(*(float *)(Size + 4));
      break;
    case 0x6E:
      ceil(*(float *)(Size + 4));
      break;
    case 0x6F:
      Z_MallocInternal(*(_DWORD *)(Size + 4));
      break;
    case 0x70:
      free(*(void **)(Size + 4));
      break;
    default:
      Com_Error(ERR_DROP, "\x15Bad UI system trap: %i", *(_DWORD *)Size);
  }
}
#endif

#if 0
void CL_ShutdownUI()
{
  int *v0;

  cls_keyCatchers &= ~2u;
  cls_loadingPlaque = 0;
  if ( uivm )
  {
    VM_Call(uivm, 2);
    v0 = (int *)uivm;
    if ( *(_DWORD *)&uivm->gap0[136] )
    {
      if ( !FreeLibrary(*(HMODULE *)&uivm->gap0[136]) )
        Com_Error(ERR_FATAL, "\x15Sys_UnloadDll FreeLibrary failed");
    }
    Com_Memset(v0, 0, 0x90u);
    currentVM = 0;
    uivm = 0;
    if ( LOBYTE(snd_aliasLoaded[0]) )
    {
      Com_UnloadSoundAliasSounds(0);
      if ( snd_aliasList[0] )
      {
        free(snd_aliasList[0]);
        snd_aliasList[0] = 0;
        snd_aliasCount[0] = 0;
        memset(snd_aliasHash, 0, 0x400u);
      }
      LOBYTE(snd_aliasLoaded[0]) = 0;
      if ( !snd_aliasLoaded_ui )
        Cmd_RemoveCommand("snd_list");
    }
  }
}
#endif

/* ---- CL_InitUI  0x00418F10 ----  VERIFIED */
int CL_InitUI( void )
{
	int version;

	Com_LoadSoundAliases( "menu", 0 );

	uivm = VM_Create( "ui", CL_UISystemCalls );
	if ( !uivm ) {
		Com_Error( ERR_FATAL, "\x15VM_Create on UI failed" );
	}

	version = VM_Call( uivm, UI_GETAPIVERSION );
	if ( version != UI_API_VERSION ) {
		Com_Error( ERR_FATAL, "\x15User Interface is version %d, expected %d",
		           version, UI_API_VERSION );
		cls_loadingPlaque = 0;
	}

	return VM_Call( uivm, UI_INIT );
}
#if 0
int CL_InitUI()
{
  vm_s *v0;
  int v1;
  int (__cdecl *v3)(int *);

  Com_LoadSoundAliases((int)"menu");
  v0 = VM_Create((const char *)CL_UISystemCalls, v3);
  uivm = v0;
  if ( !v0 )
    Com_Error(ERR_FATAL, "\x15VM_Create on UI failed\x00ui");
  v1 = VM_Call(v0, 0);
  if ( v1 != 7 )
    Com_Error(ERR_FATAL, "\x15User Interface is version %d, expected %d", v1, 7);
  return VM_Call(uivm, 1);
}
#endif

/* ---- UI_usesUniqueCDKey  0x00418F90 ----  VERIFIED */
qboolean __cdecl UI_usesUniqueCDKey( void )
{
  return (qboolean)( uivm && VM_Call( uivm, 13 ) == 1 );
}

/* ---- UI_checkKeyExec  0x00418FB0 ----  [CONFIRMED] */
int __cdecl UI_checkKeyExec(void *this)
{
  if ( uivm )
    return VM_Call(uivm, 14, this);
  else
    return 0;
}

/* ---- UI_GameCommand  0x00418FD0 ----  VERIFIED */
int __cdecl UI_GameCommand( void )
{
  if ( !uivm )
    return 0;
  return VM_Call( uivm, 11, cls_realtime );
}

/* ---- CL_ShutdownUI  0x00418E30 ----  [CONFIRMED] */
void CL_ShutdownUI( void ) {
	extern int cls_loadingPlaque;   /* 0x0155F3D4 */

	cls_keyCatchers &= ~KEYCATCH_UI;
	cls_loadingPlaque = 0;

	if ( !uivm ) {
		return;
	}

	VM_Call( uivm, UI_SHUTDOWN );
	VM_Free( uivm );
	uivm = NULL;

	if ( snd_aliasSetLoaded[SND_LOCALE_MENU] ) {
		Com_UnloadSoundAliasSounds( SND_LOCALE_MENU );

		if ( snd_aliasTable[SND_LOCALE_MENU] ) {
			free( (void *)snd_aliasTable[SND_LOCALE_MENU] );
			snd_aliasTable[SND_LOCALE_MENU]      = NULL;
			snd_aliasTableCount[SND_LOCALE_MENU] = 0;
			memset( snd_aliasHashTable[SND_LOCALE_MENU], 0,
					SND_ALIAS_HASH_SLICE_BYTES );
		}

		snd_aliasSetLoaded[SND_LOCALE_MENU] = 0;

		if ( !snd_aliasSetLoaded[SND_LOCALE_INGAME] ) {
			Cmd_RemoveCommand( "snd_list" );
		}
	}
}
