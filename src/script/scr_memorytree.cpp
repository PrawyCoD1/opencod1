/*
 * script/scr_memorytree.cpp
 *
 * Original translation unit:
 *   /Volumes/BigCheese/ Source/AspyrP4/CoD/Source/script/scr_memorytree.cpp
 *
 * Retail range 0x0046DB80-0x0046E580, 16 functions.
 *
 * @fidelity: verified
 */

#include "scr_local.h"

scrMemBlock_t   scrMemTree_blocks[SCR_MT_BLOCK_COUNT];

unsigned short  scrMemTree_freeRoots[SCR_MT_BUCKET_COUNT];

int             scrMemTree_allocBuckets;    /* retail 0x008E6310 */
int             scrMemTree_allocInstances;  /* retail 0x008E60D8 */

extern int (*GetRefString_var)();       /* == scrMemTree_arenaBase, 0x008E6208 */

/* MT_InitBits fills these once.  retail 0x008E60E0 / 0x008E6210 / 0x00966318 */
static unsigned char    mt_popcount[SCR_MT_LOOKUP_COUNT];
static unsigned char    mt_trailingZeroBits[SCR_MT_LOOKUP_COUNT];
static unsigned char    mt_bitWidth[SCR_MT_LOOKUP_COUNT];

/* ---- MT_InitBits  0x0046DC90 ---- VERIFIED */
static void MT_InitBits( void ) {
	int             value;
	int             bits;
	unsigned char   count;

	for ( value = 0; value < SCR_MT_LOOKUP_COUNT; value++ ) {
		count = 0;
		for ( bits = value; bits; bits >>= 1 ) {
			if ( bits & 1 ) {
				count++;
			}
		}
		mt_popcount[value] = count;

		count = 8;
		if ( value ) {
			while ( ( ( ( 1 << count ) - 1 ) & value ) != 0 ) {
				count--;
			}
		}
		mt_trailingZeroBits[value] = count;

		count = 0;
		for ( bits = value; bits; bits >>= 1 ) {
			count++;
		}
		mt_bitWidth[value] = count;
	}
}

/* ---- MT_GetScore  0x0046DD00 ---- VERIFIED */
static int MT_GetScore( unsigned short blockIndex ) {
	unsigned int    distance;
	unsigned int    lowByte;
	unsigned int    highByte;
	unsigned char   trailingZeros;

	distance = (unsigned int)SCR_MT_BLOCK_COUNT - blockIndex;
	lowByte  = distance & 0xFF;
	highByte = ( distance >> 8 ) & 0xFF;

	trailingZeros = mt_trailingZeroBits[lowByte];
	if ( lowByte == 0 ) {
		trailingZeros = (unsigned char)( trailingZeros + mt_trailingZeroBits[highByte] );
	}

	return ( 1 << trailingZeros )
		 + (int)( distance - mt_popcount[lowByte] - mt_popcount[highByte] );
}

/* ---- MT_GetSubTreeSize  0x0046DB80 ---- VERIFIED */
static int MT_GetSubTreeSize( unsigned short blockIndex ) {
	if ( blockIndex == 0 ) {
		return 0;
	}
	return MT_GetSubTreeSize( scrMemTree_blocks[blockIndex].node.right )
		 + MT_GetSubTreeSize( scrMemTree_blocks[blockIndex].node.left )
		 + 1;
}

/* ---- MT_DumpTree  0x0046DBC0 ---- VERIFIED */
void MT_DumpTree( void ) {
	int bucket;
	int nodes;

	Com_Printf( "********************************\n" );
	for ( bucket = 0; bucket <= SCR_MT_MAX_BUCKET; bucket++ ) {
		nodes = 0;
		if ( scrMemTree_freeRoots[bucket] ) {
			nodes = MT_GetSubTreeSize( scrMemTree_freeRoots[bucket] );
		}
		Com_Printf( "%d subtree has %d * %d = %d free buckets\n",
					bucket, nodes, 1 << bucket, nodes * ( 1 << bucket ) );
	}
	Com_Printf( "********************************\n" );
	Com_Printf( "********************************\n" );
	Com_Printf( "total memory alloc buckets: %d (%d instances)\n",
				scrMemTree_allocBuckets, scrMemTree_allocInstances );
	Com_Printf( "total memory free buckets: %d\n",
				0xFFFF - scrMemTree_allocBuckets );
	Com_Printf( "********************************\n" );
}

/* ---- Scr_GetStringUsage  0x0046DC80 ---- VERIFIED */
int Scr_GetStringUsage( void ) {
	return scrMemTree_allocBuckets;
}

/* ---- MT_Error  0x0046E210 ---- VERIFIED */
static void MT_Error( const char *caller, int size ) {
	char *msg;

	MT_DumpTree();
	msg = va( "%s: failed allocation of %d bytes for script usage", caller, size );
	Scr_DumpScriptThreads();
	Com_Error( ERR_DROP, "\x15%s", msg );
}

/* ---- MT_GetSize  0x0046E280 ---- VERIFIED */
static int MT_GetSize( int size ) {
	int blocks;

	if ( size >= SCR_MT_MAX_ALLOC_SIZE ) {
		MT_Error( "MT_GetSize: max allocation exceeded", size );
	}
	blocks = ( size + SCR_MT_BLOCK_SIZE - 1 ) / SCR_MT_BLOCK_SIZE - 1;
	if ( blocks > 255 ) {
		return mt_bitWidth[blocks >> 8] + 8;
	}
	return mt_bitWidth[blocks];
}

/* ---- MT_AddMemoryNode  0x0046DD50 ---- VERIFIED */
static void MT_AddMemoryNode( unsigned short blockIndex, int bucket ) {
	unsigned short *link;
	unsigned short  current;
	int             blockKey;
	int             centre;
	int             span;
	int             delta;

	link = &scrMemTree_freeRoots[bucket];
	current = *link;

	if ( current != 0 ) {
		blockKey = MT_GetScore( blockIndex );
		centre = 0;
		span = SCR_MT_BLOCK_COUNT;

		do {
			if ( MT_GetScore( current ) < blockKey ) {
				for ( ;; ) {
					*link = blockIndex;
					scrMemTree_blocks[blockIndex] = scrMemTree_blocks[current];

					if ( current == 0 ) {
						return;
					}

					span >>= 1;
					if ( (int)current < centre ) {
						link = &scrMemTree_blocks[blockIndex].node.left;
						delta = -span;
					} else {
						link = &scrMemTree_blocks[blockIndex].node.right;
						delta = span;
					}
					centre += delta;
					blockIndex = current;
					current = *link;
				}
			}

			span >>= 1;
			if ( (int)blockIndex < centre ) {
				link = &scrMemTree_blocks[current].node.left;
				centre -= span;
			} else {
				link = &scrMemTree_blocks[current].node.right;
				centre += span;
			}
			current = *link;
		} while ( current != 0 );
	}

	*link = blockIndex;
	scrMemTree_blocks[blockIndex].node.left = 0;
	scrMemTree_blocks[blockIndex].node.right = 0;
}

/* ---- MT_RemoveMemoryNode  0x0046DE90 ---- VERIFIED */
static qboolean MT_RemoveMemoryNode( unsigned short blockIndex, int bucket ) {
	unsigned short *link;
	unsigned short  current;
	unsigned short  next;
	int             centre;
	int             span;
	scrMemBlock_t   replacement;
	scrMemBlock_t   moved;

	centre = 0;
	span = SCR_MT_BLOCK_COUNT;
	link = &scrMemTree_freeRoots[bucket];

	for ( ;; ) {
		current = *link;
		if ( current == 0 ) {
			return qfalse;
		}
		if ( blockIndex == current ) {
			break;
		}
		if ( blockIndex == (unsigned short)centre ) {
			return qfalse;
		}

		span >>= 1;
		if ( (int)blockIndex < centre ) {
			link = &scrMemTree_blocks[current].node.left;
			centre -= span;
		} else {
			link = &scrMemTree_blocks[current].node.right;
			centre += span;
		}
	}

	replacement = scrMemTree_blocks[blockIndex];
	for ( ;; ) {
		if ( replacement.node.left == 0 ) {
			next = replacement.node.right;
			*link = next;
			if ( next == 0 ) {
				return qtrue;
			}
			link = &scrMemTree_blocks[next].node.right;
		} else if ( replacement.node.right == 0 ) {
			next = replacement.node.left;
			*link = next;
			link = &scrMemTree_blocks[next].node.left;
		} else if ( MT_GetScore( replacement.node.left )
					< MT_GetScore( replacement.node.right ) ) {
			next = replacement.node.right;
			*link = next;
			link = &scrMemTree_blocks[next].node.right;
		} else {
			next = replacement.node.left;
			*link = next;
			link = &scrMemTree_blocks[next].node.left;
		}

		moved = scrMemTree_blocks[next];
		scrMemTree_blocks[next] = replacement;
		replacement = moved;
	}
}

/* ---- MT_RemoveHeadMemoryNode  0x0046E020 ---- VERIFIED */
static void MT_RemoveHeadMemoryNode( int bucket ) {
	unsigned short *link;
	unsigned short  next;
	scrMemBlock_t   replacement;
	scrMemBlock_t   moved;

	link = &scrMemTree_freeRoots[bucket];
	replacement = scrMemTree_blocks[*link];

	for ( ;; ) {
		if ( replacement.node.left == 0 ) {
			next = replacement.node.right;
			*link = next;
			if ( next == 0 ) {
				return;
			}
			link = &scrMemTree_blocks[next].node.right;
		} else if ( replacement.node.right == 0 ) {
			next = replacement.node.left;
			*link = next;
			link = &scrMemTree_blocks[next].node.left;
		} else if ( MT_GetScore( replacement.node.left )
					< MT_GetScore( replacement.node.right ) ) {
			next = replacement.node.right;
			*link = next;
			link = &scrMemTree_blocks[next].node.right;
		} else {
			next = replacement.node.left;
			*link = next;
			link = &scrMemTree_blocks[next].node.left;
		}

		moved = scrMemTree_blocks[next];
		scrMemTree_blocks[next] = replacement;
		replacement = moved;
	}
}

/* ---- MT_Init  0x0046E190 ---- VERIFIED */
void MT_Init( void ) {
	int bucket;

	GetRefString_var = (int (*)())scrMemTree_blocks;

	MT_InitBits();

	for ( bucket = 0; bucket < SCR_MT_BUCKET_COUNT; bucket++ ) {
		scrMemTree_freeRoots[bucket] = 0;
	}

	scrMemTree_blocks[0].node.left = 0;
	scrMemTree_blocks[0].node.right = 0;

	for ( bucket = 0; bucket < SCR_MT_MAX_BUCKET; bucket++ ) {
		MT_AddMemoryNode( (unsigned short)( 1 << bucket ), bucket );
	}

	scrMemTree_allocInstances = 0;
	scrMemTree_allocBuckets = 0;
}

/* ---- MT_AllocIndex  0x0046E2D0 ---- VERIFIED */
unsigned short MT_AllocIndex( int size, int type ) {
	int             wanted;
	int             bucket;
	unsigned short  blockIndex;

	(void)type;

	wanted = MT_GetSize( size );
	bucket = wanted;

	for ( ;; ) {
		if ( bucket > SCR_MT_MAX_BUCKET ) {
			MT_Error( "MT_AllocIndex", size );
		}
		if ( scrMemTree_freeRoots[bucket] ) {
			break;
		}
		bucket++;
	}

	blockIndex = scrMemTree_freeRoots[bucket];
	MT_RemoveHeadMemoryNode( bucket );

	while ( bucket != wanted ) {
		bucket--;
		MT_AddMemoryNode( (unsigned short)( blockIndex + ( 1 << bucket ) ), bucket );
	}

	scrMemTree_allocInstances++;
	scrMemTree_allocBuckets += 1 << wanted;

	return blockIndex;
}

/* ---- MT_FreeIndex  0x0046E3F0 ---- VERIFIED */
void MT_FreeIndex( unsigned short blockIndex, unsigned int size ) {
	int bucket;
	int bucketSize;

	bucket = MT_GetSize( (int)size );

	scrMemTree_allocInstances--;
	scrMemTree_allocBuckets -= 1 << bucket;

	bucketSize = 1 << bucket;
	while ( bucket != SCR_MT_MAX_BUCKET ) {
		if ( !MT_RemoveMemoryNode( (unsigned short)( blockIndex ^ bucketSize ), bucket ) ) {
			break;
		}
		blockIndex = (unsigned short)( blockIndex & ~bucketSize );
		bucket++;
		bucketSize = 1 << bucket;
	}

	MT_AddMemoryNode( blockIndex, bucket );
}

/* ---- MT_Alloc  0x0046E4A0 ---- VERIFIED */
void *MT_Alloc( int size, int type ) {
	return &scrMemTree_blocks[MT_AllocIndex( size, type )];
}

/* ---- MT_Free  0x0046E4C0 ---- VERIFIED */
void MT_Free( void *p, unsigned int size ) {
	MT_FreeIndex( (unsigned short)( (scrMemBlock_t *)p - scrMemTree_blocks ), size );
}

/* ---- MT_Realloc  0x0046E4E0 ---- VERIFIED */
qboolean MT_Realloc( int oldSize, int newSize ) {
	return MT_GetSize( oldSize ) == MT_GetSize( newSize ) ? qtrue : qfalse;
}

void MT_Error__FPCci( const char *caller, int size ) {
	MT_Error( caller, size );
}

int MT_GetSize__Fi( int size ) {
	return MT_GetSize( size );
}
