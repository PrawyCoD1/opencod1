/*
 * qcommon/cm_load.c
 *
 * Original translation unit:
 *   /Volumes/BigCheese/ Source/AspyrP4/CoD/Source/qcommon/cm_load.c
 *
 * Retail range 0x00418FF0-0x0041A4D0, 26 functions.
 *
 * @fidelity: verified
 */

#include "cm_local.h"

extern void Sys_OutOfMemoryPrep( void );
extern char *SEH_GetLocalizedString_m( const char *key );
extern qboolean FS_MakeReadOnly( const char *path, qboolean readOnly );
extern void FS_BuildOSPath_Internal( const char *base, const char *qpath,
									 char *fsPath, int flags );
extern cvar_t *fs_homepath;

#define CM_FILE_ATTRIBUTE_READONLY  0x00000001
#define CM_MB_ICONHAND              0x00000010
__declspec( dllimport ) unsigned long __stdcall GetFileAttributesA( const char *name );
__declspec( dllimport ) int __stdcall SetFileAttributesA( const char *name,
														  unsigned long attributes );
__declspec( dllimport ) int __stdcall MessageBoxA( void *wnd, const char *text,
												   const char *caption,
												   unsigned int type );

char cm_mapName[MAX_QPATH];
cmodel_t cm_boxModel;

extern float cm_boxModelMinsX, cm_boxModelMinsY, cm_boxModelMinsZ;
extern float cm_boxModelMaxsX, cm_boxModelMaxsY, cm_boxModelMaxsZ;

static unsigned int cm_lastChecksum;

static void *cm_lumpData;

/* ---- CMod_LoadShaders  0x00418FF0 ---- VERIFIED */
static void CMod_LoadShaders( lump_t *l ) {
	dmaterial_t *in;
	int count;

	in = (dmaterial_t *)( cmod_base + l->fileofs );
	if ( l->filelen % sizeof( dmaterial_t ) ) {
		Com_Error( ERR_DROP, "\x15" "CMod_LoadShaders: funny lump size" );
	}
	count = l->filelen / sizeof( dmaterial_t );
	if ( count < 1 ) {
		Com_Error( ERR_DROP, "\x15" "Map with no shaders" );
	}

	cm_materials = (dmaterial_t *)Hunk_AllocAlignInternal(
		sizeof( dmaterial_t ) * count + sizeof( dmaterial_t ), 32 ) + 1;
	cm_numMaterials = count;

	Com_Memcpy( cm_materials, in, count * sizeof( dmaterial_t ) );
}

/* ---- CMod_LoadSubmodels  0x00419070 ---- VERIFIED */
static void CMod_LoadSubmodels( lump_t *l ) {
	const float *in;
	cmodel_t *out;
	int *indexes;
	int count, i, j, n;

	in = (const float *)( cmod_base + l->fileofs );
	if ( l->filelen % 48 ) {
		Com_Error( ERR_DROP, "\x15" "CMod_LoadSubmodels: funny lump size" );
	}
	count = l->filelen / 48;
	if ( count < 1 ) {
		Com_Error( ERR_DROP, "\x15" "Map with no models" );
	}

	cm_cmodels = (cmodel_t *)Hunk_AllocAlignInternal( count * sizeof( cmodel_t ), 32 );
	cm_numSubModels = count;

	if ( count > MAX_SUBMODELS ) {
		Com_Error( ERR_DROP, "\x15" "MAX_SUBMODELS exceeded" );
	}

	for ( i = 0; i < count; i++, in += 12 ) {
		const int *ini = (const int *)in;

		out = &cm_cmodels[i];

		out->mins[0] = in[0] - 1.0f;
		out->maxs[0] = in[3] + 1.0f;
		out->mins[1] = in[1] - 1.0f;
		out->maxs[1] = in[4] + 1.0f;
		out->mins[2] = in[2] - 1.0f;
		out->maxs[2] = in[5] + 1.0f;

		if ( i == 0 ) {
			continue;
		}

		n = ini[11];
		out->leaf.numLeafBrushes = (unsigned short)n;
		if ( (unsigned short)n != n ) {
			Com_Error( ERR_DROP, "\x15" "CMod_LoadSubmodels: numLeafBrushes exceeded" );
		}

		indexes = (int *)Hunk_AllocAlignInternal( n * sizeof( int ), 32 );
		out->leaf.firstLeafBrush = indexes - cm_leafbrushes;
		for ( j = 0; j < n; j++ ) {
			indexes[j] = ini[10] + j;
		}

		out->leaf.numLeafSurfaces = (unsigned short)ini[9];
		if ( (unsigned short)ini[9] != ini[9] ) {
			Com_Error( ERR_DROP, "\x15" "CMod_LoadSubmodels: numLeafSurfaces exceeded" );
		}
		out->leaf.firstLeafSurface = (unsigned short)ini[8];
		if ( (unsigned short)ini[8] != ini[8] ) {
			Com_Error( ERR_DROP, "\x15" "CMod_LoadSubmodels: firstLeafSurface exceeded" );
		}
	}
}

/* ---- CMod_LoadNodes  0x00419210 ---- VERIFIED */
static void CMod_LoadNodes( lump_t *l ) {
	const int *in;
	cNode_t *out;
	int count, i, j, child;

	in = (const int *)( cmod_base + l->fileofs );
	if ( l->filelen % 36 ) {
		Com_Error( ERR_DROP, "\x15" "MOD_LoadBmodel: funny lump size" );
	}
	count = l->filelen / 36;
	if ( count < 1 ) {
		Com_Error( ERR_DROP, "\x15" "Map has no nodes" );
	}

	cm_nodes = (cNode_t *)Hunk_AllocAlignInternal( count * sizeof( cNode_t ), 32 );
	cm_numNodes = count;

	out = cm_nodes;
	for ( i = 0; i < count; i++, out++, in += 9 ) {
		out->plane = cm_planes + in[0];
		for ( j = 0; j < 2; j++ ) {
			child = in[1 + j];
			out->children[j] = (short)child;
			if ( (short)child != child ) {
				Com_Error( ERR_DROP, "\x15" "CMod_LoadNodes: children exceeded" );
			}
		}
	}
}

/* ---- CMod_LoadBrushes  0x00419300 ---- VERIFIED */
static void CMod_LoadBrushes( lump_t *sidesLump, lump_t *brushLump ) {
	const short *in;
	const int *sideIn;
	cbrush_t *out;
	cbrushside_t *side;
	int numBrushes, numSides;
	int i, axis, end, materialNum;

	sideIn = (const int *)( cmod_base + sidesLump->fileofs );
	if ( brushLump->filelen & 3 ) {
		Com_Error( ERR_DROP, "\x15" "CMod_LoadBrushes: funny lump size" );
	}
	numBrushes = brushLump->filelen >> 2;

	in = (const short *)( cmod_base + brushLump->fileofs );
	if ( sidesLump->filelen & 7 ) {
		Com_Error( ERR_DROP, "\x15" "CMod_LoadBrushes: funny lump size" );
	}

	numSides = ( sidesLump->filelen >> 3 ) - 6 * numBrushes;
	if ( numSides < 0 ) {
		Com_Error( ERR_DROP, "\x15" "CMod_LoadBrushes: bad side count" );
	}

	if ( numSides ) {
		cm_brushsides = (cbrushside_t *)Hunk_AllocAlignInternal(
			numSides * sizeof( cbrushside_t ), 32 );
	} else {
		cm_brushsides = NULL;
	}
	cm_numBrushSides = numSides;

	cm_brushes = (cbrush_t *)Hunk_AllocAlignInternal(
		( numBrushes + 1 ) * sizeof( cbrush_t ), 32 );
	cm_numBrushes = numBrushes;

	side = cm_brushsides;
	out = cm_brushes;

	for ( i = 0; i < numBrushes; i++, out++, in += 2 ) {
		out->numSides = in[0] - 6;
		if ( out->numSides < 0 ) {
			Com_Error( ERR_DROP, "\x15" "CMod_LoadBrushes: brush has less than 6 sides" );
		}
		out->sides = out->numSides ? side : NULL;

		for ( axis = 0; axis < 3; axis++ ) {
			for ( end = 0; end < 2; end++, sideIn += 2 ) {
				( &out->mins[0] )[end * 3 + axis] = *(const float *)&sideIn[0];

				materialNum = sideIn[1];
				if ( materialNum < 0 || materialNum >= cm_numMaterials ) {
					Com_Error( ERR_DROP, "\x15" "CMod_LoadBrushes: bad shaderNum: %i",
							   materialNum );
				}
				out->axialMaterialNum[end * 3 + axis] = (short)materialNum;
				if ( (short)materialNum != materialNum ) {
					Com_Error( ERR_DROP, "\x15" "CMod_LoadBrushes: axialShaderNum exceeded" );
				}
			}
		}

		{
			int n;
			for ( n = 0; n < out->numSides; n++, side++, sideIn += 2 ) {
				side->plane = cm_planes + sideIn[0];
				side->materialNum = sideIn[1];
				if ( side->materialNum < 0 || side->materialNum >= cm_numMaterials ) {
					Com_Error( ERR_DROP, "\x15" "CMod_LoadBrushes: bad shaderNum: %i",
							   side->materialNum );
				}
			}
		}

		materialNum = in[1];
		if ( materialNum < 0 || materialNum >= cm_numMaterials ) {
			Com_Error( ERR_DROP, "\x15" "CMod_LoadBrushes: bad shaderNum: %i", materialNum );
		}
		out->contents = cm_materials[materialNum].contentFlags & 0xDFFF7FFB;
	}
}

/* ---- CMod_LoadLeafs  0x00419560 ---- VERIFIED */
static void CMod_LoadLeafs( lump_t *l ) {
	const int *in;
	cLeaf_t *out;
	int count, i;

	in = (const int *)( cmod_base + l->fileofs );
	if ( l->filelen % 36 ) {
		Com_Error( ERR_DROP, "\x15" "CMod_LoadLeafs: funny lump size" );
	}
	count = l->filelen / 36;
	if ( count < 1 ) {
		Com_Error( ERR_DROP, "\x15" "Map with no leafs" );
	}

	cm_leafs = (cLeaf_t *)Hunk_AllocAlignInternal( count * sizeof( cLeaf_t ), 32 );
	cm_numLeafs = count;

	out = cm_leafs;
	for ( i = 0; i < count; i++, out++, in += 9 ) {
		out->cellnum = (short)in[6];
		if ( (short)in[6] != in[6] ) {
			Com_Error( ERR_DROP, "\x15" "CMod_LoadLeafs: cellnum exceeded" );
		}
		out->cluster = (short)in[0];
		if ( (short)in[0] != in[0] ) {
			Com_Error( ERR_DROP, "\x15" "CMod_LoadLeafs: cluster exceeded" );
		}
		out->area = (short)in[1];
		if ( (short)in[1] != in[1] ) {
			Com_Error( ERR_DROP, "\x15" "CMod_LoadLeafs: area exceeded" );
		}
		out->firstLeafBrush = in[4];
		out->numLeafBrushes = (unsigned short)in[5];
		if ( (unsigned short)in[5] != in[5] ) {
			Com_Error( ERR_DROP, "\x15" "CMod_LoadLeafs: numLeafBrushes exceeded" );
		}
		out->firstLeafSurface = (unsigned short)in[2];
		if ( (unsigned short)in[2] != in[2] ) {
			Com_Error( ERR_DROP, "\x15" "CMod_LoadLeafs: firstLeafSurface exceeded" );
		}
		out->numLeafSurfaces = (unsigned short)in[3];
		if ( (unsigned short)in[3] != in[3] ) {
			Com_Error( ERR_DROP, "\x15" "CMod_LoadLeafs: numLeafSurfaces exceeded" );
		}

		if ( in[0] >= cm_numClusters ) {
			cm_numClusters = in[0] + 1;
		}
		if ( in[1] >= cm_numAreas ) {
			cm_numAreas = in[1] + 1;
		}
	}

	cm_areas = (cArea_t *)Hunk_AllocAlignInternal(
		cm_numAreas * sizeof( cArea_t ), 32 );
	cm_areaPortals = (int *)Hunk_AllocAlignInternal(
		cm_numAreas * cm_numAreas * sizeof( int ), 32 );
}

/* ---- CMod_LoadPlanes  0x00419700 ---- VERIFIED */
static void CMod_LoadPlanes( lump_t *l ) {
	const float *in;
	cplane_t *out;
	unsigned int count;
	unsigned int i;
	int bits;

	in = (const float *)( cmod_base + l->fileofs );
	if ( l->filelen & 15 ) {
		Com_Error( ERR_DROP, "\x15" "MOD_LoadBmodel: funny lump size" );
	}
	count = (unsigned int)l->filelen >> 4;
	if ( !count ) {
		Com_Error( ERR_DROP, "\x15" "Map with no planes" );
	}

	cm_planes = (cplane_t *)Hunk_AllocAlignInternal( count * sizeof( cplane_t ), 32 );
	cm_numPlanes = count;

	out = cm_planes;
	for ( i = 0; i < count; i++, out++, in += 4 ) {
		bits = 0;

		out->normal[0] = in[0];
		if ( in[0] < 0.0f ) {
			bits |= 1;
		}
		out->normal[1] = in[1];
		if ( in[1] < 0.0f ) {
			bits |= 2;
		}
		out->normal[2] = in[2];
		if ( in[2] < 0.0f ) {
			bits |= 4;
		}
		out->dist = in[3];

		if ( out->normal[0] == 1.0f ) {
			out->type = 0;
		} else if ( out->normal[1] == 1.0f ) {
			out->type = 1;
		} else if ( out->normal[2] == 1.0f ) {
			out->type = 2;
		} else {
			out->type = 3;
		}
		out->signbits = (byte)bits;
	}
}

/* ---- CMod_LoadLeafBrushes  0x00419810 ---- VERIFIED */
static void CMod_LoadLeafBrushes( lump_t *l ) {
	const int *in;
	int *out;
	int count, i;

	in = (const int *)( cmod_base + l->fileofs );
	if ( l->filelen & 3 ) {
		Com_Error( ERR_DROP, "\x15" "MOD_LoadBmodel: funny lump size" );
	}
	count = (unsigned int)l->filelen >> 2;

	out = (int *)Hunk_AllocAlignInternal( count * sizeof( int ) + sizeof( int ), 32 );
	cm_leafbrushes = out;
	cm_numLeafBrushes = count;

	for ( i = 0; i < count; i++ ) {
		out[i] = in[i];
	}
}

/* ---- CMod_LoadLeafSurfaces  0x00419870 ---- VERIFIED */
static void CMod_LoadLeafSurfaces( lump_t *l ) {
	const int *in;
	int *out;
	int count, i;

	in = (const int *)( cmod_base + l->fileofs );
	if ( l->filelen & 3 ) {
		Com_Error( ERR_DROP, "\x15" "CMod_LoadLeafSurfaces: funny lump size" );
	}
	count = (unsigned int)l->filelen >> 2;

	out = (int *)Hunk_AllocAlignInternal( count * sizeof( int ), 32 );
	cm_leafsurfaces = out;
	cm_numLeafSurfaces = count;

	for ( i = 0; i < count; i++ ) {
		out[i] = in[i];
	}
}

#define MAX_TERRAIN_VERTS       0x2000
#define MAX_TERRAIN_INDEXES     0xC000

/* ---- CMod_LoadLeafCurvesAndTerrain  0x004198D0 ---- VERIFIED */
static void CMod_LoadLeafCurvesAndTerrain( lump_t *surfLump, lump_t *vertLump,
										   lump_t *indexLump ) {
	const dcollisionsurface_t *in;
	const float *verts;
	const short *indexes;
	cPatch_t *out;
	int count, i, j, n;
	static vec3_t points[MAX_TERRAIN_VERTS];
	static short heightIndexes[MAX_TERRAIN_INDEXES];

	in = (const dcollisionsurface_t *)( cmod_base + surfLump->fileofs );
	if ( surfLump->filelen & 15 ) {
		Com_Error( ERR_DROP, "\x15" "CMod_LoadLeafpatchesAndTerrain: funny lump size" );
	}
	count = surfLump->filelen >> 4;

	verts = (const float *)( cmod_base + vertLump->fileofs );
	if ( vertLump->filelen % 12 ) {
		Com_Error( ERR_DROP, "\x15" "CMod_LoadLeafpatchesAndTerrain: funny lump size" );
	}

	indexes = (const short *)( cmod_base + indexLump->fileofs );
	if ( indexLump->filelen & 1 ) {
		Com_Error( ERR_DROP, "\x15" "CMod_LoadLeafpatchesAndTerrain: funny lump size" );
	}

	cm_numPatches = count;
	cm_patches = (cPatch_t *)Hunk_AllocAlignInternal( count * sizeof( cPatch_t ), 32 );

	out = cm_patches;
	for ( i = 0; i < count; i++, out++, in++ ) {
		out->checkcount = 0;
		out->materialNum = in->materialIndex;
		out->contents = cm_materials[in->materialIndex].contentFlags;

		if ( in->isTerrain ) {
			const float *src;
			int numVerts;

			numVerts = in->a;
			if ( (unsigned int)numVerts > MAX_TERRAIN_VERTS ) {
				Com_Error( ERR_DROP,
						   "\x15" "CMod_LoadLeafpatchesAndTerrain: more than %i verts in terrain\n",
						   MAX_TERRAIN_VERTS );
			}
			src = verts + 3 * in->firstIndex;
			for ( j = 0; j < numVerts; j++ ) {
				points[j][0] = src[j * 3 + 0];
				points[j][1] = src[j * 3 + 1];
				points[j][2] = src[j * 3 + 2];
			}

			n = (unsigned short)in->b;
			if ( (unsigned int)n > MAX_TERRAIN_INDEXES ) {
				Com_Error( ERR_DROP,
						   "\x15" "CMod_LoadLeafpatchesAndTerrain: more than %i indexes in terrain\n",
						   MAX_TERRAIN_INDEXES );
			}
			if ( n > 0 ) {
				Com_Memcpy( heightIndexes, indexes + in->secondIndex,
							n * sizeof( short ) );
			}

			out->pc = NULL;
			out->tc = CM_GenerateTerrainCollide( n, heightIndexes,
												 numVerts, points,
												 &out->mins );
		} else {
			const float *src;
			int width, height;

			width = in->a;
			height = in->b;
			n = width * height;
			if ( (unsigned int)n > MAX_TERRAIN_VERTS ) {
				Com_Error( ERR_DROP,
						   "\x15" "CMod_LoadLeafpatchesAndTerrain: more than %i verts in curve\n",
						   MAX_TERRAIN_VERTS );
			}
			src = verts + 3 * in->secondIndex;
			for ( j = 0; j < n; j++ ) {
				points[j][0] = src[j * 3 + 0];
				points[j][1] = src[j * 3 + 1];
				points[j][2] = src[j * 3 + 2];
			}

			out->pc = CM_GeneratePatchCollide( width, height, in->firstIndex,
												   points, &out->mins );
			out->tc = NULL;
		}
	}
}

/* ---- CMod_LoadEntityString  0x00419BC0 ---- VERIFIED */
static void CMod_LoadEntityString( lump_t *l ) {
	cm_entityString = (char *)Hunk_AllocAlignInternal( l->filelen, 32 );
	cm_entityStringLen = l->filelen;
	Com_Memcpy( cm_entityString, cmod_base + l->fileofs, l->filelen );
}

/* ---- CMod_LoadVisibility  0x00419C00 ---- VERIFIED */
static void CMod_LoadVisibility( lump_t *l ) {
	const int *in;
	unsigned int len;

	len = l->filelen;
	if ( !len ) {
		cm_clusterBytes = ( cm_numClusters + 31 ) & ~31;
		cm_visibility = (byte *)Hunk_AllocAlignInternal( cm_clusterBytes, 32 );
		Com_Memset( cm_visibility, 255, cm_clusterBytes );
		return;
	}

	in = (const int *)( cmod_base + l->fileofs );
	cm_vised = qtrue;
	cm_visibility = (byte *)Hunk_AllocAlignInternal( len, 32 );
	cm_numClusters = in[0];
	cm_clusterBytes = in[1];
	Com_Memcpy( cm_visibility, in + 2, len - 8 );
}

static void CM_ClearMap( void ) {
	Com_Memset( cm_mapName, 0, sizeof( cm_mapName ) );

	cm_numMaterials = 0;        cm_materials = NULL;
	cm_numBrushSides = 0;       cm_brushsides = NULL;
	cm_numPlanes = 0;           cm_planes = NULL;
	cm_numNodes = 0;            cm_nodes = NULL;
	cm_numLeafs = 0;            cm_leafs = NULL;
	cm_numLeafBrushes = 0;      cm_leafbrushes = NULL;
	cm_numLeafSurfaces = 0;     cm_leafsurfaces = NULL;
	cm_numSubModels = 0;        cm_cmodels = NULL;
	cm_numBrushes = 0;          cm_brushes = NULL;
	cm_numClusters = 0;         cm_clusterBytes = 0;
	cm_visibility = NULL;       cm_vised = qfalse;
	cm_entityStringLen = 0;     cm_entityString = NULL;
	cm_numAreas = 0;            cm_areas = NULL;
	cm_areaPortals = NULL;
	cm_numPatches = 0;          cm_patches = NULL;
	cm_floodvalid = 0;          cm_checkcount = 0;

	box_brush = NULL;
	Com_Memset( &cm_boxModel, 0, sizeof( cm_boxModel ) );

	CM_ClearWorldSectors();
}

/* ---- CM_LoadMap  0x00419C90 ---- VERIFIED */
void CM_LoadMap( const char *name, qboolean clientload, int *checksum ) {
	dheader_t header;
	char mapName[MAX_QPATH];
	void *buf;
	int length;

	if ( !name || !name[0] ) {
		Com_Error( ERR_DROP, "\x15" "CM_LoadMap: NULL name" );
	}

	strncpy( mapName, name, 63 );
	mapName[63] = 0;

	cm_noCurves = Cvar_Get( "cm_noCurves", "0", CVAR_CHEAT );
	cm_playerCurveClip = Cvar_Get( "cm_playerCurveClip", "1", CVAR_CHEAT | CVAR_ARCHIVE );

	Com_DPrintf( "CM_LoadMap( %s, %i )\n", mapName, clientload );

	if ( clientload && com_sv_running->integer ) {
		*checksum = cm_lastChecksum;
		return;
	}

	CM_ClearMap();

	length = FS_ReadFile( mapName, &buf );
	if ( !buf ) {
		Com_Error( ERR_DROP, va( "EXE_ERR_COULDNT_LOAD\x15%s", mapName ) );
	}

	cm_lastChecksum = Com_BlockChecksum( buf, length );
	*checksum = cm_lastChecksum;

	header = *(dheader_t *)buf;

	if ( header.version != CM_BSP_VERSION ) {
		Com_Error( ERR_DROP, va(
			"EXE_ERR_WRONG_MAP_VERSION_NUM\x15%s\x15(%i \x14" "EXE_ERR_SHOULD_BE\x15 %i)",
			mapName, header.version, CM_BSP_VERSION ) );
	}

	cmod_base = (byte *)buf;

	CMod_LoadShaders( &header.lumps[LUMP_MATERIALS] );
	CMod_LoadPlanes( &header.lumps[LUMP_PLANES] );
	CMod_LoadBrushes( &header.lumps[LUMP_BRUSHSIDES], &header.lumps[LUMP_BRUSHES] );
	CMod_LoadNodes( &header.lumps[LUMP_NODES] );
	CMod_LoadLeafs( &header.lumps[LUMP_LEAFS] );
	CMod_LoadLeafBrushes( &header.lumps[LUMP_LEAFBRUSHES] );
	CMod_LoadLeafSurfaces( &header.lumps[LUMP_LEAFSURFACES] );
	CMod_LoadLeafCurvesAndTerrain( &header.lumps[LUMP_LEAFSURFACES_D],
								   &header.lumps[LUMP_COLLISIONVERTS],
								   &header.lumps[LUMP_COLLISIONINDEX] );
	CMod_LoadSubmodels( &header.lumps[LUMP_MODELS] );
	CMod_LoadVisibility( &header.lumps[LUMP_VISIBILITY] );
	CMod_LoadEntityString( &header.lumps[LUMP_ENTITIES] );

	FS_FreeFile( buf );

	CM_InitBoxHull();
	CM_FloodAreaConnections();
	CM_ClearWorld();
	CM_LoadStaticModels();

	if ( !clientload ) {
		strncpy( cm_mapName, mapName, 63 );
		cm_mapName[63] = 0;
	}
}

/* ---- CM_SaveLump  0x00419F00 ---- VERIFIED */
void CM_SaveLump( int lump, const void *data, int length, unsigned int *checksum ) {
	dheader_t header;
	void *buf;
	byte *out;
	char fsPath[MAX_OSPATH];
	qboolean wasReadOnly;
	fileHandle_t f;
	size_t total;
	int ofs, i;
	unsigned long attributes;

	FS_ReadFile( cm_mapName, &buf );
	if ( !buf ) {
		Com_Error( ERR_DROP, va( "EXE_ERR_COULDNT_LOAD\x15%s", cm_mapName ) );
	}

	header = *(dheader_t *)buf;

	total = CM_HEADER_SIZE;
	for ( i = 0; i < CM_LUMPS; i++ ) {
		if ( i == lump ) {
			header.lumps[i].filelen = length;
		}
		total += ( header.lumps[i].filelen + 3 ) & ~3;
	}

	out = (byte *)malloc( total );
	if ( !out ) {
		char *title, *body;

		Sys_OutOfMemoryPrep();
		title = SEH_GetLocalizedString_m( "WIN_OUT_OF_MEM_TITLE" );
		body = SEH_GetLocalizedString_m( "WIN_OUT_OF_MEM_BODY" );
		MessageBoxA( 0, body, title, CM_MB_ICONHAND );
		exit( -1 );
	}
	Com_Memset( out, 0, total );

	ofs = CM_HEADER_SIZE;
	for ( i = 0; i < CM_LUMPS; i++ ) {
		if ( header.lumps[i].filelen ) {
			const void *src;

			if ( i == lump ) {
				src = data;
			} else {
				src = (const byte *)buf + header.lumps[i].fileofs;
			}
			Com_Memcpy( out + ofs, src, header.lumps[i].filelen );
		}
		header.lumps[i].fileofs = ofs;
		ofs += ( header.lumps[i].filelen + 3 ) & ~3;
	}

	Com_Memcpy( out, &header, CM_HEADER_SIZE );

	wasReadOnly = FS_MakeReadOnly( cm_mapName, qfalse );
	f = FS_FOpenFileWrite( cm_mapName );
	if ( f ) {
		FS_Write( out, ofs, f );
		FS_FCloseFile( f );
	} else {
		Com_Printf( "Failed to open %s\n", cm_mapName );
	}

	if ( wasReadOnly ) {
		FS_BuildOSPath_Internal( fs_homepath->string, cm_mapName, fsPath, 0 );
		attributes = GetFileAttributesA( fsPath );
		if ( ( attributes | CM_FILE_ATTRIBUTE_READONLY ) != attributes ) {
			SetFileAttributesA( fsPath, attributes | CM_FILE_ATTRIBUTE_READONLY );
		}
	}

	if ( checksum ) {
		*checksum = Com_BlockChecksum( out, ofs );
	}

	free( out );
	FS_FreeFile( buf );
}

/* ---- CM_LoadLump_m  0x0041A180 ---- VERIFIED */
int CM_LoadLump_m( int lump, void **data ) {
	dheader_t header;
	fileHandle_t f;
	int len;

	fs_loadingMode = 1;
	FS_FOpenFileRead( cm_mapName, &f, qfalse );
	if ( !f ) {
		Com_Error( ERR_DROP, va( "EXE_ERR_COULDNT_LOAD\x15%s", cm_mapName ) );
	}

	FS_Read( &header, CM_HEADER_SIZE, f );

	len = header.lumps[lump].filelen;
	if ( !len ) {
		FS_FCloseFile( f );
		return 0;
	}

	FS_Seek( f, header.lumps[lump].fileofs - CM_HEADER_SIZE, FS_SEEK_CUR );
	cm_lumpData = Hunk_AllocateTempMemoryInternal( len );
	FS_Read( cm_lumpData, len, f );
	FS_FCloseFile( f );

	*data = cm_lumpData;
	return len;
}

/* ---- CM_FreeLump_m  0x0041A250 ---- VERIFIED */
void CM_FreeLump_m( void ) {
	Hunk_FreeTempMemoryInternal( cm_lumpData );
}

/* ---- CM_ClipHandleToModel  0x0041A2A0 ---- VERIFIED */
cmodel_t *CM_ClipHandleToModel( clipHandle_t handle ) {
	if ( handle < 0 ) {
		Com_Error( ERR_DROP, "\x15" "CM_ClipHandleToModel: bad handle %i", handle );
	}
	if ( handle < cm_numSubModels ) {
		return &cm_cmodels[handle];
	}
	if ( handle == BOX_MODEL_HANDLE || handle == CAPSULE_MODEL_HANDLE ) {
		return &cm_boxModel;
	}
	if ( handle < MAX_SUBMODELS ) {
		Com_Error( ERR_DROP, "\x15" "CM_ClipHandleToModel: bad handle %i < %i < %i",
				   cm_numSubModels, handle, MAX_SUBMODELS );
	}
	Com_Error( ERR_DROP, "\x15" "CM_ClipHandleToModel: bad handle %i",
			   handle + MAX_SUBMODELS );

	return NULL;
}

/* ---- CM_InlineModel  0x0041A320 ---- VERIFIED */
clipHandle_t CM_InlineModel( int index ) {
	if ( index < 0 || index >= cm_numSubModels ) {
		Com_Error( ERR_DROP, "\x15" "CM_InlineModel: bad number" );
	}
	return index;
}

/* ---- CM_NumClusters  0x0041A340 ---- VERIFIED */
int CM_NumClusters( void ) {
	return cm_numClusters;
}

/* ---- CM_NumInlineModels  0x0041A350 ---- VERIFIED */
int CM_NumInlineModels( void ) {
	return cm_numSubModels;
}

/* ---- CM_EntityString  0x0041A360 ---- VERIFIED */
const char *CM_EntityString( void ) {
	return cm_entityString;
}

/* ---- CM_LeafCluster  0x0041A370 ---- VERIFIED */
int CM_LeafCluster( int leafnum ) {
	return cm_leafs[leafnum].cluster;
}

/* ---- CM_LeafArea  0x0041A380 ---- VERIFIED */
int CM_LeafArea( int leafnum ) {
	return cm_leafs[leafnum].area;
}

/* ---- CM_InitBoxHull  0x0041A390 ---- VERIFIED */
void CM_InitBoxHull( void ) {
	int i;

	box_brush = &cm_brushes[cm_numBrushes];
	box_brush->numSides = 0;
	box_brush->sides = NULL;
	box_brush->contents = -1;
	for ( i = 0; i < 6; i++ ) {
		box_brush->axialMaterialNum[i] = -1;
	}

	cm_boxModel.leaf.numLeafBrushes = 1;
	cm_boxModel.leaf.firstLeafBrush = cm_numLeafBrushes;
	cm_leafbrushes[cm_numLeafBrushes] = cm_numBrushes;
}

/* ---- CM_TempBoxModel  0x0041A420 ---- VERIFIED */
clipHandle_t CM_TempBoxModel( const vec3_t mins, const vec3_t maxs,
							  int contents, qboolean capsule ) {
	VectorCopy( mins, cm_boxModel.mins );
	VectorCopy( maxs, cm_boxModel.maxs );

	cm_boxModelMinsX = mins[0];  cm_boxModelMinsY = mins[1];  cm_boxModelMinsZ = mins[2];
	cm_boxModelMaxsX = maxs[0];  cm_boxModelMaxsY = maxs[1];  cm_boxModelMaxsZ = maxs[2];

	VectorCopy( mins, box_brush->mins );
	VectorCopy( maxs, box_brush->maxs );
	box_brush->contents = contents;

	return capsule ? CAPSULE_MODEL_HANDLE : BOX_MODEL_HANDLE;
}

/* ---- CM_SetTempBoxModelContents  0x0041A4C0 ---- VERIFIED */
void CM_SetTempBoxModelContents( int contents ) {
	( void )contents;
}

/* ---- CM_ModelBounds  0x0041A4D0 ---- VERIFIED */
void CM_ModelBounds( clipHandle_t model, vec3_t mins, vec3_t maxs ) {
	cmodel_t *cmod;

	cmod = CM_ClipHandleToModel( model );
	VectorCopy( cmod->mins, mins );
	VectorCopy( cmod->maxs, maxs );
}
