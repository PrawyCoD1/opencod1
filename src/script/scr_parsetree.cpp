/*
 * script/scr_parsetree.cpp
 *
 * Original translation unit:
 *   /Volumes/BigCheese/ Source/AspyrP4/CoD/Source/script/scr_parsetree.cpp
 *
 * Retail range 0x0046F1B0-0x0046F7B9, 16 functions.
 *
 * @fidelity: verified
 */

#include "../qcommon/qcommon.h"
#include "../qcommon/hexrays_shim.h"
#include "../qcommon/cod1_globals.h"

typedef unsigned int sval_t;

extern void *Hunk_AllocateTempMemoryHighInternal( int size );

/* ParseTree_Alloc  0x0046F7A0  VERIFIED */
static sval_t *ParseTree_Alloc( int words )
{
	return (sval_t *)Hunk_AllocateTempMemoryHighInternal( words * 4 );
}

/* ---- node1_  0x0046F1B0 ---- */
sval_t node1_( sval_t a )
{
	return a;
}

/* ---- node_pos  0x0046F1C0 ---- */
sval_t node_pos( sval_t sourcePos )
{
	return sourcePos;
}

/* ---- node0  0x0046F1D0 ---- */
sval_t node0( int type )
{
	sval_t *node = ParseTree_Alloc( 1 );

	node[0] = (sval_t)type;
	return (sval_t)node;
}

/* ---- node1  0x0046F230 ---- */
sval_t node1( int type, sval_t a )
{
	sval_t *node = ParseTree_Alloc( 2 );

	node[0] = (sval_t)type;
	node[1] = a;
	return (sval_t)node;
}

/* ---- node2  0x0046F290 ---- */
sval_t node2( int type, sval_t a, sval_t b )
{
	sval_t *node = ParseTree_Alloc( 3 );

	node[0] = (sval_t)type;
	node[1] = a;
	node[2] = b;
	return (sval_t)node;
}

/* ---- node2_  0x0046F300 ---- */
sval_t node2_( sval_t a, sval_t b )
{
	sval_t *node = ParseTree_Alloc( 2 );

	node[0] = a;
	node[1] = b;
	return (sval_t)node;
}

/* ---- node3  0x0046F360 ---- */
sval_t node3( int type, sval_t a, sval_t b, sval_t c )
{
	sval_t *node = ParseTree_Alloc( 4 );

	node[0] = (sval_t)type;
	node[1] = a;
	node[2] = b;
	node[3] = c;
	return (sval_t)node;
}

/* ---- node3_  0x0046F3D0 ---- */
sval_t node3_( sval_t a, sval_t b, sval_t c )
{
	sval_t *node = ParseTree_Alloc( 3 );

	node[0] = a;
	node[1] = b;
	node[2] = c;
	return (sval_t)node;
}

/* ---- node4  0x0046F440 ---- */
sval_t node4( int type, sval_t a, sval_t b, sval_t c, sval_t d )
{
	sval_t *node = ParseTree_Alloc( 5 );

	node[0] = (sval_t)type;
	node[1] = a;
	node[2] = b;
	node[3] = c;
	node[4] = d;
	return (sval_t)node;
}

/* ---- node4_  0x0046F4B0 ---- */
sval_t node4_( sval_t a, sval_t b, sval_t c, sval_t d )
{
	sval_t *node = ParseTree_Alloc( 4 );

	node[0] = a;
	node[1] = b;
	node[2] = c;
	node[3] = d;
	return (sval_t)node;
}

/* ---- node5  0x0046F520 ---- */
sval_t node5( int type, sval_t a, sval_t b, sval_t c, sval_t d, sval_t e )
{
	sval_t *node = ParseTree_Alloc( 6 );

	node[0] = (sval_t)type;
	node[1] = a;
	node[2] = b;
	node[3] = c;
	node[4] = d;
	node[5] = e;
	return (sval_t)node;
}

/* ---- node6  0x0046F5A0 ---- */
sval_t node6( int type, sval_t a, sval_t b, sval_t c, sval_t d, sval_t e, sval_t f )
{
	sval_t *node = ParseTree_Alloc( 7 );

	node[0] = (sval_t)type;
	node[1] = a;
	node[2] = b;
	node[3] = c;
	node[4] = d;
	node[5] = e;
	node[6] = f;
	return (sval_t)node;
}

/* ---- linked_list_end  0x0046F620 ---- */
sval_t linked_list_end( sval_t a )
{
	sval_t *cell;
	sval_t *list;

	cell = ParseTree_Alloc( 2 );
	cell[0] = a;
	cell[1] = 0;

	list = ParseTree_Alloc( 2 );
	list[0] = (sval_t)cell;
	list[1] = (sval_t)cell;
	return (sval_t)list;
}

/* ---- prepend_node  0x0046F6D0 ---- */
sval_t prepend_node( sval_t a, sval_t listVal )
{
	sval_t *list = (sval_t *)listVal;
	sval_t *cell;

	cell = ParseTree_Alloc( 2 );
	cell[0] = a;
	cell[1] = list[0];
	list[0] = (sval_t)cell;
	return listVal;
}

/* ---- append_node  0x0046F730 ---- */
sval_t append_node( sval_t listVal, sval_t a )
{
	sval_t *list = (sval_t *)listVal;
	sval_t *cell;

	cell = ParseTree_Alloc( 2 );
	cell[0] = a;
	cell[1] = 0;

	((sval_t *)list[1])[1] = (sval_t)cell;
	list[1] = (sval_t)cell;
	return listVal;
}

/* ---- append_list  0x0046F7A0 ---- VERIFIED */
sval_t append_list( sval_t aVal, sval_t bVal )
{
	sval_t *a = (sval_t *)aVal;
	sval_t *b = (sval_t *)bVal;

	((sval_t *)a[1])[1] = b[0];
	a[1] = b[1];
	return aVal;
}
