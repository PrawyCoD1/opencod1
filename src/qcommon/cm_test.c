/*
 * qcommon/cm_test.c
 *
 * Original translation unit:
 *   /Volumes/BigCheese/ Source/AspyrP4/CoD/Source/qcommon/cm_test.c
 *
 * Retail range 0x00421C30-0x004222B0, 14 functions.
 *
 * @fidelity: verified
 */

#include "cm_local.h"

/* ---- CM_PointLeafnum_r  0x00421C30 ---- VERIFIED */
int CM_PointLeafnum_r( const vec3_t p, int num ) {
	float d;
	cNode_t *node;
	cplane_t *plane;

	while ( num >= 0 ) {
		node = cm_nodes + num;
		plane = node->plane;

		if ( plane->type < 3 ) {
			d = p[plane->type] - plane->dist;
		} else {
			d = DotProduct( plane->normal, p ) - plane->dist;
		}

		if ( d < 0 ) {
			num = node->children[1];
		} else {
			num = node->children[0];
		}
	}

	return -1 - num;
}

/* ---- CM_PointLeafnum  0x00421C90 ---- VERIFIED */
int CM_PointLeafnum( const vec3_t p ) {
	return CM_PointLeafnum_r( p, 0 );
}

/* ---- CM_StoreLeafs  0x00421CA0 ---- VERIFIED */
void CM_StoreLeafs( leafList_t *ll, int nodenum ) {
	int leafNum;

	leafNum = -1 - nodenum;

	if ( cm_leafs[leafNum].cluster != -1 ) {
		ll->lastLeaf = leafNum;
	}

	if ( ll->count >= ll->maxcount ) {
		ll->overflowed = qtrue;
		return;
	}
	ll->list[ll->count++] = leafNum;
}

/* ---- CM_StoreBrushes  0x00421CE0 ---- VERIFIED */
void CM_StoreBrushes( leafList_t *ll, int nodenum ) {
	int i, k;
	cLeaf_t *leaf;
	int brushnum;
	cbrush_t *b;

	leaf = &cm_leafs[-1 - nodenum];

	for ( k = 0; k < leaf->numLeafBrushes; k++ ) {
		brushnum = cm_leafbrushes[leaf->firstLeafBrush + k];
		b = &cm_brushes[brushnum];

		if ( b->checkcount == cm_checkcount ) {
			continue;
		}
		b->checkcount = cm_checkcount;

		for ( i = 0; i < 3; i++ ) {
			if ( b->mins[i] >= ll->bounds[1][i] || b->maxs[i] <= ll->bounds[0][i] ) {
				break;
			}
		}
		if ( i != 3 ) {
			continue;
		}

		if ( ll->count >= ll->maxcount ) {
			ll->overflowed = qtrue;
			return;
		}
		( (cbrush_t **)ll->list )[ll->count++] = b;
	}
}

/* ---- CM_BoxLeafnums_r  0x00421DA0 ---- VERIFIED */
void CM_BoxLeafnums_r( leafList_t *ll, int nodenum ) {
	cplane_t *plane;
	cNode_t *node;
	int s;

	while ( nodenum >= 0 ) {
		node = &cm_nodes[nodenum];
		plane = node->plane;

		s = BoxOnPlaneSide( ll->bounds[0], ll->bounds[1], plane );
		if ( s == 1 ) {
			nodenum = node->children[0];
		} else if ( s == 2 ) {
			nodenum = node->children[1];
		} else {
			CM_BoxLeafnums_r( ll, node->children[0] );
			nodenum = node->children[1];
		}
	}

	ll->storeLeafs( ll, nodenum );
}

/* ---- CM_BoxLeafnums  0x00421E10 ---- VERIFIED */
int CM_BoxLeafnums( const vec3_t mins, const vec3_t maxs,
					int *list, int listsize, int *lastLeaf ) {
	leafList_t ll;

	cm_checkcount++;

	VectorCopy( mins, ll.bounds[0] );
	VectorCopy( maxs, ll.bounds[1] );
	ll.count = 0;
	ll.maxcount = listsize;
	ll.list = list;
	ll.storeLeafs = CM_StoreLeafs;
	ll.lastLeaf = 0;
	ll.overflowed = qfalse;

	CM_BoxLeafnums_r( &ll, 0 );

	*lastLeaf = ll.lastLeaf;
	return ll.count;
}

/* ---- CM_BoxBrushes  0x00421E90 ---- VERIFIED */
int CM_BoxBrushes( const vec3_t mins, const vec3_t maxs,
				   cbrush_t **list, int listsize ) {
	leafList_t ll;

	cm_checkcount++;

	VectorCopy( mins, ll.bounds[0] );
	VectorCopy( maxs, ll.bounds[1] );
	ll.count = 0;
	ll.maxcount = listsize;
	ll.list = (int *)list;
	ll.storeLeafs = CM_StoreBrushes;
	ll.lastLeaf = 0;
	ll.overflowed = qfalse;

	CM_BoxLeafnums_r( &ll, 0 );

	return ll.count;
}

/* ---- CM_PointContents  0x00421F00 ---- VERIFIED */
int CM_PointContents( const vec3_t p, clipHandle_t model ) {
	int leafnum;
	int i, k;
	int brushnum;
	cLeaf_t *leaf;
	cbrush_t *b;
	int contents;
	float d;

	if ( model ) {
		leaf = &CM_ClipHandleToModel( model )->leaf;
	} else {
		leafnum = CM_PointLeafnum_r( p, 0 );
		leaf = &cm_leafs[leafnum];
	}

	contents = 0;
	for ( k = 0; k < leaf->numLeafBrushes; k++ ) {
		brushnum = cm_leafbrushes[leaf->firstLeafBrush + k];
		b = &cm_brushes[brushnum];

		for ( i = 0; i < 3; i++ ) {
			if ( p[i] < b->mins[i] || p[i] > b->maxs[i] ) {
				break;
			}
		}
		if ( i != 3 ) {
			continue;
		}

		for ( i = 0; i < b->numSides; i++ ) {
			d = DotProduct( p, b->sides[i].plane->normal ) - b->sides[i].plane->dist;
			if ( d > 0 ) {
				break;
			}
		}
		if ( i == b->numSides ) {
			contents |= b->contents;
		}
	}

	return contents;
}

/* ---- CM_TransformedPointContents  0x00421FF0 ---- VERIFIED */
int CM_TransformedPointContents( const vec3_t p, clipHandle_t model,
								 const vec3_t origin, const vec3_t angles ) {
	vec3_t p_l;
	vec3_t temp;
	vec3_t forward, right, up;

	VectorSubtract( p, origin, p_l );

	if ( model != BOX_MODEL_HANDLE
		 && ( angles[0] != 0.0f || angles[1] != 0.0f || angles[2] != 0.0f ) ) {
		AngleVectors( angles, forward, right, up );

		VectorCopy( p_l, temp );
		p_l[0] = DotProduct( temp, forward );
		p_l[1] = -DotProduct( temp, right );
		p_l[2] = DotProduct( temp, up );
	}

	return CM_PointContents( p_l, model );
}

/* ---- CM_ClusterPVS  0x004220E0 ---- VERIFIED */
byte *CM_ClusterPVS( int cluster ) {
	if ( cluster >= 0 && cluster < cm_numClusters && cm_vised ) {
		return cm_visibility + cluster * cm_clusterBytes;
	}
	return cm_visibility;
}

/* ---- CM_FloodArea_r  0x00422110 ---- VERIFIED */
void CM_FloodArea_r( int areaNum, int floodnum ) {
	int i;
	cArea_t *area;
	const int *con;

	area = &cm_areas[areaNum];

	if ( area->floodvalid == cm_floodvalid ) {
		if ( area->floodnum == floodnum ) {
			return;
		}
		Com_Error( ERR_DROP, "\x15" "FloodArea_r: reflooded" );
	}

	area->floodnum = floodnum;
	area->floodvalid = cm_floodvalid;

	con = cm_areaPortals + areaNum * cm_numAreas;
	for ( i = 0; i < cm_numAreas; i++ ) {
		if ( con[i] > 0 ) {
			CM_FloodArea_r( i, floodnum );
		}
	}
}

/* ---- CM_FloodAreaConnections  0x00422190 ---- VERIFIED */
void CM_FloodAreaConnections( void ) {
	int i;
	int floodnum;

	cm_floodvalid++;
	floodnum = 0;

	for ( i = 0; i < cm_numAreas; i++ ) {
		if ( cm_areas[i].floodvalid == cm_floodvalid ) {
			continue;
		}
		floodnum++;
		CM_FloodArea_r( i, floodnum );
	}
}

/* ---- CM_ChangeAreaPortalState  0x004221E0 ---- VERIFIED */
void CM_ChangeAreaPortalState( int area1, int area2, qboolean open ) {
	if ( area1 < 0 || area2 < 0 ) {
		return;
	}

	if ( area1 >= cm_numAreas || area2 >= cm_numAreas ) {
		Com_Error( ERR_DROP, "\x15" "CM_ChangeAreaPortalState: bad area number" );
	}

	if ( open ) {
		cm_areaPortals[area1 + area2 * cm_numAreas]++;
		cm_areaPortals[area2 + area1 * cm_numAreas]++;
	} else if ( cm_areaPortals[area2 + area1 * cm_numAreas] ) {
		cm_areaPortals[area1 + area2 * cm_numAreas]--;
		if ( --cm_areaPortals[area2 + area1 * cm_numAreas] < 0 ) {
			Com_Error( ERR_DROP, "\x15" "CM_AdjustAreaPortalState: negative reference count" );
		}
	}

	CM_FloodAreaConnections();
}

/* ---- CM_AreasConnected  0x004222B0 ---- VERIFIED */
qboolean CM_AreasConnected( int area1, int area2 ) {
	if ( area1 < 0 || area2 < 0 ) {
		return qfalse;
	}
	return cm_areas[area1].floodnum == cm_areas[area2].floodnum;
}
