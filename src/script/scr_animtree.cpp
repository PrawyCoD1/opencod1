/*
 * script/scr_animtree.cpp
 *
 * Original translation unit:
 *   /Volumes/BigCheese/ Source/AspyrP4/CoD/Source/script/scr_animtree.cpp
 *
 * Retail range 0x00467180-0x00468817, 20 functions.
 *
 * @fidelity: verified
 * @fidelity-default: verified
 */

#include "../qcommon/qcommon.h"
#include "../qcommon/hexrays_shim.h"

#include <ctype.h>
#include <string.h>

#define scrAnimPub_treeCount        cod1_globals_scalar_decl_8E58D0
#define scrAnimPub_treeNames         cod1_globals_scalar_decl_8E58D8
#define scrAnimPub_trees        cod1_globals_scalar_decl_8E5AD8
#define scrAnimPub_parseStart        cod1_globals_scalar_decl_8E58C8
#define scrAnimPub_parseState        cod1_globals_scalar_decl_8E58CC
#define scrAnimPub_propertyNames          cod1_globals_scalar_decl_575818
#define FindNextSibling     cod1_globals_bogus_decl_FindNextSibling
#define FindObject          cod1_globals_bogus_decl_FindObject
#include "../qcommon/cod1_globals.h"
#undef scrAnimPub_treeCount
#undef scrAnimPub_treeNames
#undef scrAnimPub_trees
#undef scrAnimPub_parseStart
#undef scrAnimPub_parseState
#undef scrAnimPub_propertyNames
#undef FindNextSibling
#undef FindObject

#define SCR_ANIMTREE_POOL_COUNT     2       /* 0x00A9CC58 selects one */
#define SCR_ANIMTREE_SLOTS          128

int             scrAnimPub_treeCount[SCR_ANIMTREE_POOL_COUNT];
unsigned short  scrAnimPub_treeNames[SCR_ANIMTREE_POOL_COUNT * SCR_ANIMTREE_SLOTS];
int             scrAnimPub_trees[SCR_ANIMTREE_POOL_COUNT * SCR_ANIMTREE_SLOTS];

#define scrAnimPub_treeCount    scrAnimPub_treeCount
#define scrAnimPub_treeNames    scrAnimPub_treeNames
#define scrAnimPub_trees        scrAnimPub_trees
#define scrAnimPub_pool         xanim_activePoolSlot

#define SCR_ANIMTREE_SLOT_VALID( slot ) \
	( (unsigned int)(slot) < (unsigned int)SCR_ANIMTREE_SLOTS )

#define SCR_ANIMTREE_AT( slot ) \
	scrAnimPub_trees[ SCR_ANIMTREE_SLOTS * scrAnimPub_pool + (slot) ]
#define SCR_ANIMTREE_NAME_AT( slot ) \
	scrAnimPub_treeNames[ SCR_ANIMTREE_SLOTS * scrAnimPub_pool + (slot) ]

extern int AllocVariable__Fv();
extern int FindVariableIndexInternal__FUsUi();
extern int FreeValue__FP21VariableValueInternal();
extern int GetObject__FUs();
extern int GetVariableIndexInternal__FUsUi();
extern int Hunk_AllocAlignInternal();
extern int MakeVariableExternal__FP8VariableP21VariableValueInternal();
extern int RemoveRefToObject__FUs();
extern int RemoveVariable__FUsUi();
extern int SL_FreeString__FUsPCcUi();
extern int Scr_IsIdentifier__FPCc();
extern int SetVariableValue__FUsP13VariableValue();
extern void *XAnimPrecacheAnimTree_m( const char *animName,
                                      void *( __cdecl *allocFn )( unsigned int ) );
#define XAnimLoadFile XAnimPrecacheAnimTree_m

extern void __cdecl XAnimCheckSyncNodes_r( void *tree, int nodeIndex );

extern void QDECL CompileError( unsigned int sourcePos, const char *format, ... );
extern void QDECL CompileError2( const char *codePos, const char *format, ... );
extern qboolean   Scr_IsIdentifier( const char *name );
extern char      *Com_GetLastTokenPos( void );
extern char      *Com_ParseOnLine( char **data_p );
extern char      *Scr_AddSourceBuffer( const char *filename, void *a2, void *a3 );
extern int        Hunk_ClearTempMemoryHigh( void );

extern void      *FS_GetDataForFile( const char *dir, const char *name, const char *ext );

typedef struct scrXAnimEntry_s {
	unsigned short  childCount;      /* +0x00  0 == leaf */
	unsigned short  parentIndex;     /* +0x02 */
	union {
		void       *leafAsset;       /* +0x04  when childCount == 0 */
		struct {
			unsigned short  flags;           /* +0x04 */
			unsigned short  firstChildIndex; /* +0x06 */
		} parent;
	} u;
} scrXAnimEntry_t;

typedef struct scrXAnim_s {
	const char      *name;           /* +0x00 */
	int              nodeCount;      /* +0x04 */
	scrXAnimEntry_t  entries[1];     /* +0x08  [nodeCount] */
} scrXAnim_t;

extern void *__cdecl XAnimSetLeafNode( const char *animName, scrXAnim_t *tree,
									   unsigned short index );

#define SCR_HANDLE( g )     ( *(unsigned short *)&(g) )

#define scrAnimPub_treeRoot         SCR_HANDLE( scrAnimPub_treeRoot )  /* 0x008E60C0 */
#define scrAnimPub_currentTreeRoot  SCR_HANDLE( scrAnimPub_currentTreeRoot )  /* 0x008E5EDC */
#define scrAnimPub_currentUsingTree SCR_HANDLE( dword_8E60B0 )  /* 0x008E60B0 */
#define scrAnimPub_checksum         ( *(unsigned int *)&scrCompilePub_checksum )
#define scrCompilePub_sourceBuf     scrCompilePub_sourceBuf               /* 0x01407370 */
#define scrCompilePub_scriptfilename scrCompilePub_scriptfilename              /* 0x01407374 */

static const char  *scrAnimPub_parseStart;
static char        *scrAnimPub_parseState;

#define SCR_ANIM_PROP_LOOPSYNC      0x01
#define SCR_ANIM_PROP_NONLOOPSYNC   0x02
#define SCR_ANIM_PROP_COMPLETE      0x08

static const char * const scrAnimPub_propertyNames[3] = {
	"loopsync", "nonloopsync", "complete"
};

#define SCR_ANIMTREE_CHILD_ANIMS    0
#define SCR_ANIMTREE_CHILD_TREE     1

#define SCR_ANIMTREE_NAME_TYPE      4

#include "scr_local.h"

/* ---- SetAnimCheck  0x00467180 ---- VERIFIED */
static int __cdecl SetAnimCheck(int result)
{
  com_animCheckCached = result;
  return result;
}

/* ---- AnimTreeCompileError  0x00467190 ---- VERIFIED */
void AnimTreeCompileError( const char *message )
{
	const char *errorPos = Com_GetLastTokenPos();

	Com_EndParseSession();
	CompileError( (unsigned int)( errorPos - scrAnimPub_parseStart ), "%s", message );
}

int AnimTreeCompileError__FPc( const char *message ) {
	AnimTreeCompileError( message );
	return 0;
}

/* ---- GetAnimTreeParseProperties  0x004671F0 ---- VERIFIED */
unsigned int GetAnimTreeParseProperties( void )
{
	unsigned int flags = 0;

	for ( ;; ) {
		const char     *token = Com_ParseOnLine( &scrAnimPub_parseState );
		unsigned int    i;

		if ( !token[0] ) {
			return flags;
		}

		for ( i = 0; i < 3; i++ ) {
			if ( !Q_stricmp( token, scrAnimPub_propertyNames[i] ) ) {
				break;
			}
		}

		switch ( i ) {
		case 0:  flags |= SCR_ANIM_PROP_LOOPSYNC;    break;
		case 1:  flags |= SCR_ANIM_PROP_NONLOOPSYNC; break;
		case 2:  flags |= SCR_ANIM_PROP_COMPLETE;    break;
		default: AnimTreeCompileError( "unknown anim property" ); break;
		}
	}
}

int GetAnimTreeParseProperties__Fv( void ) { return (int)GetAnimTreeParseProperties(); }

/* ---- Scr_EmitAnimationInternal  0x00467300 ---- VERIFIED */
void Scr_EmitAnimationInternal( char *animRef, unsigned short animName,
										   unsigned short tree, unsigned int sourcePos )
{
	unsigned short  handle;
	VariableValue  *stored;
	VariableValue   value;

	if ( developer_script_int ) {
		CompileError( sourcePos, "cannot reference animation from /# ... #/ comment" );
		return;
	}

	handle = FindVariable( tree, animName );
	if ( handle ) {
		stored = GetVariableValueAddress( handle );
		*(unsigned int *)animRef = (unsigned int)stored->u.pointerValue;
		stored->u.pointerValue   = animRef;
		return;
	}

	handle = GetVariable( tree, animName );
	*(unsigned int *)animRef = 0;
	value.u.pointerValue = animRef;
	value.type           = VAR_CODEPOS;
	SetVariableValue( handle, &value );
}

/* ---- Scr_EmitAnimation  0x004673A0 ---- VERIFIED */
void Scr_EmitAnimation( char *animRef, unsigned short animName, unsigned int sourcePos )
{
	if ( !scrAnimPub_currentUsingTree ) {
		CompileError( sourcePos, "#using_animtree was not specified" );
		return;
	}
	Scr_EmitAnimationInternal( animRef, animName, scrAnimPub_currentUsingTree, sourcePos );
}

int Scr_EmitAnimation__FPcUsUi( char *animRef, unsigned short animName,
								unsigned int sourcePos ) {
	Scr_EmitAnimation( animRef, animName, sourcePos );
	return 0;
}

/* AnimTree_TokenIsName  0x004674AC */
static qboolean AnimTree_TokenIsName( const char *token )
{
	const char *p = token;

	if ( !*p ) {
		return qtrue;
	}
	while ( isalnum( (unsigned char)*p ) || *p == '_' ) {
		if ( !*++p ) {
			return qtrue;
		}
	}
	return qfalse;
}

/* ---- AnimTreeParseInternal  0x004673D0 ---- VERIFIED */
static qboolean AnimTreeParseInternal( unsigned short parentHandle,
									   unsigned short treeHandle,
									   qboolean addEmptyVoid,
									   qboolean addLoopingVoid,
									   qboolean forceLoadedChildren )
{
	unsigned short  animName   = 0;
	unsigned short  animNode   = 0;
	unsigned int    properties = 0;
	qboolean        missingFromTree = qfalse;
	qboolean        hitEndOfFile    = qtrue;
	const char     *token;

	for ( ;; ) {
		token = Com_ParseExt( &scrAnimPub_parseState, qtrue );
		if ( scrAnimPub_parseState == NULL ) {
			break;
		}

		if ( AnimTree_TokenIsName( token ) ) {
			if ( missingFromTree ) {
				RemoveVariable( parentHandle, animName );
			}

			animName = SL_GetLowercaseStringOfLen( token, SCR_STRING_USER_CANONICAL,
												   strlen( token ) + 1,
												   SCR_ANIMTREE_NAME_TYPE );
			if ( FindVariable( parentHandle, animName ) ) {
				AnimTreeCompileError( "duplicate animation" );
			}
			animNode = GetVariable( parentHandle, animName );

			missingFromTree = qfalse;
			if ( !forceLoadedChildren
			  && !FindVariable( treeHandle, animName )
			  && !com_animCheckCached ) {
				missingFromTree = qtrue;
			}

			properties = 0;
			token = Com_ParseOnLine( &scrAnimPub_parseState );
			if ( !token[0] ) {
				continue;
			}
			if ( AnimTree_TokenIsName( token ) ) {
				AnimTreeCompileError( "FIXME: aliases not yet implemented" );
			}
			if ( token[0] != ':' || token[1] ) {
				AnimTreeCompileError( "bad token" );
			}

			properties = GetAnimTreeParseProperties();
			token = Com_ParseExt( &scrAnimPub_parseState, qtrue );
			if ( token[0] != '{' || token[1] ) {
				AnimTreeCompileError(
					"properties cannot be applied to primitive animations" );
			}
		}

		if ( token[0] == '{' ) {
			unsigned short  childArray;
			qboolean        childForce;

			if ( token[1] ) {
				AnimTreeCompileError( "bad token" );
			}
			token = Com_ParseOnLine( &scrAnimPub_parseState );
			if ( token[0] ) {
				/* retail's literal at 0x0055BB60. */
				AnimTreeCompileError( "token not allowed after '{'" );
			}
			if ( !animNode ) {
				AnimTreeCompileError( "no animation specified for this block" );
			}

			childArray = GetArray( animNode );
			childForce = ( forceLoadedChildren
						|| ( ( properties & SCR_ANIM_PROP_COMPLETE ) && !missingFromTree ) )
						? qtrue : qfalse;

			if ( AnimTreeParseInternal( childArray, treeHandle,
										missingFromTree ? qfalse : qtrue,
										( properties & SCR_ANIM_PROP_LOOPSYNC ) ? qtrue : qfalse,
										childForce ) ) {
				AnimTreeCompileError( "unexpected end of file" );
			}

			if ( GetArraySize( childArray ) == 0 ) {
				RemoveVariable( parentHandle, animName );
			} else {
				VariableValue value;

				value.u.intValue = (int)properties;
				value.type       = VAR_INTEGER;
				SetVariableValue( GetArrayVariable( childArray, 0 ), &value );
			}

			animNode        = 0;
			missingFromTree = qfalse;
			continue;
		}

		if ( token[0] == '}' ) {
			if ( token[1] ) {
				AnimTreeCompileError( "bad token" );
			}
			token = Com_ParseOnLine( &scrAnimPub_parseState );
			if ( token[0] ) {
				/* retail's literal at 0x0055BB04. */
				AnimTreeCompileError( "token not allowed after '}'" );
			}
			hitEndOfFile = qfalse;
			break;
		}

		AnimTreeCompileError( "bad token" );
	}

	if ( missingFromTree ) {
		RemoveVariable( parentHandle, animName );
	}

	if ( addEmptyVoid && GetArraySize( parentHandle ) == 0 ) {
		const char     *voidName = addLoopingVoid ? "void_loop" : "void";
		unsigned short  handle;

		handle = SL_GetStringOfLen( voidName, 0, strlen( voidName ) + 1,
									SCR_ANIMTREE_NAME_TYPE );
		GetVariable( parentHandle, handle );
		SL_RemoveRefToString( handle );
	}

	return hitEndOfFile;
}

int AnimTreeParseInternal__FUsUsbbb( unsigned short parentHandle, unsigned short treeHandle,
									 int addEmptyVoid, int addLoopingVoid,
									 int forceLoadedChildren ) {
	return AnimTreeParseInternal( parentHandle, treeHandle,
								  addEmptyVoid ? qtrue : qfalse,
								  addLoopingVoid ? qtrue : qfalse,
								  forceLoadedChildren ? qtrue : qfalse );
}

/* ---- Scr_AnimTreeParse  0x00467B40 ---- VERIFIED */
void Scr_AnimTreeParse( const char *source, unsigned short parentHandle,
						unsigned short treeHandle )
{
	Com_BeginParseSession( "Scr_AnimTreeParse" );
	scrAnimPub_parseState = (char *)source;
	scrAnimPub_parseStart = source;

	if ( !AnimTreeParseInternal( parentHandle, treeHandle, qtrue, qfalse, qfalse ) ) {
		AnimTreeCompileError( "bad token" );
	}
	Com_EndParseSession();
}

int Scr_AnimTreeParse__FPCcUsUs( const char *source, unsigned short parentHandle,
								 unsigned short treeHandle ) {
	Scr_AnimTreeParse( source, parentHandle, treeHandle );
	return 0;
}

/* ---- Hunk_AllocXAnimPrecache  0x00467BC0 ---- VERIFIED */
void * __cdecl Hunk_AllocXAnimPrecache( unsigned int size )
{
  return (void *)Hunk_AllocAlignInternal( size, 4 );
}

/* ---- Scr_GetAnimTreeSize  0x00467BD0 ---- VERIFIED */
int Scr_GetAnimTreeSize( unsigned short handle )
{
	unsigned short  child;
	int             nodeCount = 0;

	for ( child = FindNextSibling( handle ); child != 0; child = FindNextSibling( child ) ) {
		if ( GetVariableName( child ) >= VAR_NAME_OBJECT_BASE ) {
			continue;
		}
		if ( GetVarType( child ) == VAR_OBJECT ) {
			nodeCount += Scr_GetAnimTreeSize( FindObject( child ) );
		} else {
			nodeCount++;
		}
	}

	if ( nodeCount ) {
		nodeCount++;
	}
	return nodeCount;
}

int Scr_GetAnimTreeSize__FUs( unsigned short handle ) { return Scr_GetAnimTreeSize( handle ); }

/* ---- ConnectScriptToAnim  0x00467C80 ---- VERIFIED */
void ConnectScriptToAnim( unsigned short unresolvedRoot, int nodeIndex,
						  unsigned short treeName, unsigned short animName,
						  int treeIndex )
{
	unsigned short  handle;
	VariableValue  *stored;
	unsigned int    packed;
	char           *ref;

	handle = FindVariable( unresolvedRoot, animName );
	if ( !handle ) {
		return;
	}

	stored = GetVariableValueAddress( handle );
	if ( stored->u.pointerValue == NULL ) {
		Com_Error( ERR_DROP, "\x15" "duplicate animation '%s' in 'animtrees/%s.atr'",
				   SL_ConvertToString( animName ), SL_ConvertToString( treeName ) );
	}

	packed = ( (unsigned int)treeIndex << 16 ) | (unsigned short)nodeIndex;
	ref    = (char *)stored->u.pointerValue;
	while ( ref != NULL ) {
		char *next = (char *)( *(unsigned int *)ref );

		*(unsigned int *)ref = packed;
		ref = next;
	}
	stored->u.pointerValue = NULL;
}

int ConnectScriptToAnim__FUsiUsUsi( unsigned short unresolvedRoot, int nodeIndex,
									unsigned short treeName, unsigned short animName,
									int treeIndex ) {
	ConnectScriptToAnim( unresolvedRoot, nodeIndex, treeName, animName, treeIndex );
	return 0;
}

/* ---- Scr_GetAnimsIndex  0x00467D30 ---- VERIFIED */
int __cdecl Scr_GetAnimsIndex( void *anims )
{
	int slot;

	for ( slot = scrAnimPub_treeCount[ scrAnimPub_pool ]; slot != 0; slot-- ) {
		if ( SCR_ANIMTREE_SLOT_VALID( slot )
		  && SCR_ANIMTREE_AT( slot ) == (int)anims ) {
			break;
		}
	}
	return slot;
}

/* Scr_GetAnimTreeCount  HIGH */
int __cdecl Scr_GetAnimTreeCount( void )
{
	return scrAnimPub_treeCount[ scrAnimPub_pool ];
}

/* ---- Scr_GetAnims  0x00467D60 ---- VERIFIED */
int __cdecl Scr_GetAnims( int slot )
{
	if ( !SCR_ANIMTREE_SLOT_VALID( slot ) ) {
		return 0;
	}
	return SCR_ANIMTREE_AT( slot );
}

/* ---- Scr_CreateAnimationTree  0x00467D80 ---- VERIFIED */
static unsigned int Scr_CreateAnimationTree( unsigned short sourceHandle,
											 unsigned short unresolvedRoot,
											 scrXAnim_t *tree,
											 unsigned int firstChildIndex,
											 const char *nodeName,
											 unsigned int nodeIndex,
											 unsigned short treeName,
											 int treeIndex )
{
	unsigned short   child;
	unsigned short   childCount = 0;
	unsigned short   properties = 0;
	unsigned short   propertyHandle;
	unsigned int     childNodeIndex;
	unsigned int     nextFreeNodeIndex;
	unsigned int     i;
	scrXAnimEntry_t *entry;

	for ( child = FindNextSibling( sourceHandle ); child != 0; child = FindNextSibling( child ) ) {
		if ( GetVariableName( child ) < VAR_NAME_OBJECT_BASE ) {
			childCount++;
		}
	}

	propertyHandle = FindArrayVariable( sourceHandle, 0 );
	if ( propertyHandle ) {
		properties = (unsigned short)GetVariableValueAddress( propertyHandle )->u.intValue;
	}

	scrAnimPub_checksum = properties + 31u * ( childCount + 31u *
						  ( firstChildIndex + 31u * ( nodeIndex + 31u * scrAnimPub_checksum ) ) );

	/* --- XAnimSetParentNode, inlined as retail does --- */
	entry = &tree->entries[nodeIndex];
	entry->childCount             = childCount;
	entry->u.parent.flags         = properties;
	entry->u.parent.firstChildIndex = (unsigned short)firstChildIndex;
	for ( i = 0; i < childCount; i++ ) {
		tree->entries[ entry->u.parent.firstChildIndex + i ].parentIndex =
			(unsigned short)nodeIndex;
	}
	(void)nodeName;

	childNodeIndex    = firstChildIndex;
	nextFreeNodeIndex = firstChildIndex + childCount;

	for ( child = FindNextSibling( sourceHandle ); child != 0; child = FindNextSibling( child ) ) {
		unsigned int    name = GetVariableName( child );
		unsigned short  animName;

		if ( name >= VAR_NAME_OBJECT_BASE ) {
			continue;
		}
		animName = (unsigned short)name;

		ConnectScriptToAnim( unresolvedRoot, (int)childNodeIndex, treeName, animName,
							 treeIndex );

		if ( GetVarType( child ) == VAR_OBJECT ) {
			nextFreeNodeIndex = Scr_CreateAnimationTree(
									FindObject( child ), unresolvedRoot, tree,
									nextFreeNodeIndex, SL_ConvertToString( animName ),
									childNodeIndex, treeName, treeIndex );
		} else {
			scrAnimPub_checksum = childNodeIndex + 31u * scrAnimPub_checksum;
			XAnimSetLeafNode( SL_ConvertToString( animName ), tree,
							  (unsigned short)childNodeIndex );
		}
		childNodeIndex++;
	}

	return nextFreeNodeIndex;
}

int Scr_CreateAnimationTree__FUsUsP7XAnim_sUiPCcUiUsi(
		unsigned short sourceHandle, unsigned short unresolvedRoot, void *tree,
		unsigned int firstChildIndex, const char *nodeName, unsigned int nodeIndex,
		unsigned short treeName, int treeIndex ) {
	return (int)Scr_CreateAnimationTree( sourceHandle, unresolvedRoot,
										 (scrXAnim_t *)tree, firstChildIndex,
										 nodeName, nodeIndex, treeName, treeIndex );
}

/* ---- Scr_CheckAnimsDefined  0x00468000 ---- VERIFIED */
void Scr_CheckAnimsDefined( unsigned short unresolvedRoot, unsigned short treeName )
{
	unsigned short child;

	for ( child = FindNextSibling( unresolvedRoot ); child != 0;
		  child = FindNextSibling( child ) ) {
		VariableValue  *stored = GetVariableValueAddress( child );
		unsigned short  animName;
		unsigned int    codePos;
		char           *message;

		if ( stored->u.pointerValue == NULL ) {
			continue;
		}

		animName = (unsigned short)GetVariableName( child );
		message  = va( "animation '%s' not defined in anim tree '%s'",
					   SL_ConvertToString( animName ),
					   SL_ConvertToString( treeName ) );

		codePos = stored->u.uintValue;
		if ( ( codePos < (unsigned int)scrVarPub_programBuffer
			|| codePos >= (unsigned int)scrVarPub_programBuffer + (unsigned int)scrCompilePub_programLen )
		  && ( codePos < (unsigned int)scrCompileGlob_devOpBuffer
			|| codePos >= (unsigned int)developerCodeStart ) ) {
			Com_Error( ERR_DROP, "\x15%s", message );
		}
		CompileError2( (const char *)codePos, "%s", message );
	}
}

int Scr_CheckAnimsDefined__FUsUs( unsigned short unresolvedRoot, unsigned short treeName ) {
	Scr_CheckAnimsDefined( unresolvedRoot, treeName );
	return 0;
}

/* ---- Scr_PrecacheAnimationTree  0x00468110 ---- VERIFIED */
void __cdecl Scr_PrecacheAnimationTree( unsigned short animsId )
{
	unsigned short  id;
	unsigned int    name;

	for ( id = FindNextSibling( animsId ); id != 0; id = FindNextSibling( id ) ) {
		name = scrVarNodes[id].status >> VAR_STATUS_NAME_SHIFT;
		if ( name >= VAR_NAME_OBJECT_BASE ) {
			continue;
		}

		if ( ( scrVarNodes[id].status & VAR_STATUS_TYPE_MASK ) == VAR_OBJECT ) {
			Scr_PrecacheAnimationTree( scrVarNodes[id].u.halfword[0] );
		} else {
			XAnimLoadFile( SL_ConvertToString( (unsigned short)name ),
						   Hunk_AllocXAnimPrecache );
		}
	}
}

int Scr_PrecacheAnimationTree__FUs( unsigned short animsId ) {
	Scr_PrecacheAnimationTree( animsId );
	return 0;
}
#if 0
int __cdecl Scr_PrecacheAnimationTree(unsigned __int16 a1)
{
  unsigned __int16 v1;
  int result;
  int v3;
  unsigned int v4;
  unsigned int v5;
  int *v6;

  v1 = FindNextSibling[2 * HIWORD(firstFreeVariable[3 * a1])];
  result = FindObject[3 * v1] & 0x1F;
  if ( (unsigned __int8)result < 0xCu )
  {
    if ( v1 )
    {
      do
      {
        v3 = 3 * v1;
        v4 = FindObject[v3];
        v5 = v4 >> 8;
        if ( v4 >> 8 < 0x10000 )
        {
          if ( (v4 & 0x1F) == 7 )
          {
            Scr_PrecacheAnimationTree(dword_9B6478[v3]);
          }
          else
          {
            if ( (_WORD)v5 )
              v6 = (int *)(GetRefString_var + 8 * (unsigned __int16)v5 + 4);
            else
              v6 = 0;
            XAnimLoadFile(v6, (int (__cdecl *)(int))Hunk_AllocXAnimPrecache);
          }
        }
        v1 = FindNextSibling[2 * *(unsigned __int16 *)(v3 * 4 + 10183810)];
        result = FindObject[3 * v1] & 0x1F;
      }
      while ( (unsigned __int8)result < 0xCu && v1 );
    }
  }
  return result;
}
#endif

/* ---- Scr_UsingTreeInternal  0x004681E0 ---- VERIFIED */
unsigned short __cdecl Scr_UsingTreeInternal( const char *name, int *slot )
{
	unsigned short  canonical;
	unsigned short  entryNode;
	unsigned short  object;
	unsigned short  animsHandle;
	unsigned short  animsNode;
	int             count;
	int             i;

	canonical = Scr_CreateCanonicalFilename( name );

	entryNode = scrVarIndirections[
		FindVariableIndexInternal( (unsigned short)scrAnimPub_treeRoot, canonical ) ].id;

	if ( entryNode != 0 ) {
		object  = scrVarNodes[entryNode].u.halfword[0];
		*slot   = 0;
		count   = scrAnimPub_treeCount[ scrAnimPub_pool ];
		for ( i = 1; i <= count; i++ ) {
			if ( !SCR_ANIMTREE_SLOT_VALID( i ) ) {
				break;
			}
			if ( SCR_ANIMTREE_NAME_AT( i ) == entryNode ) {
				*slot = i;
				break;
			}
		}
	} else {
		entryNode = scrVarIndirections[
			GetVariableIndexInternal( (unsigned short)scrAnimPub_treeRoot, canonical ) ].id;
		object = GetObject( entryNode );

		count = scrAnimPub_treeCount[ scrAnimPub_pool ] + 1;
		scrAnimPub_treeCount[ scrAnimPub_pool ] = count;
		if ( SCR_ANIMTREE_SLOT_VALID( count ) ) {
			SCR_ANIMTREE_NAME_AT( count ) = entryNode;
		} else {
			Com_Printf( "^3too many #using_animtree names (max %d)\n",
						SCR_ANIMTREE_SLOTS - 1 );
		}
		*slot = count;
	}

	animsHandle = GetVariableIndexInternal( object, 0 );
	animsNode   = scrVarIndirections[animsHandle].id;

	if ( ( scrVarNodes[animsNode].status & VAR_STATUS_TYPE_MASK ) == VAR_UNDEFINED ) {
		scrVarNodes[animsNode].status |= VAR_OBJECT;
		scrVarNodes[animsNode].u.halfword[0] = Scr_AllocArray();
	}

	animsHandle = scrVarNodes[animsNode].u.halfword[0];
	SL_RemoveRefToString( canonical );
	return animsHandle;
}

#if 0
__int16 __cdecl Scr_UsingTreeInternal(char *a1, int *a2)
{
  unsigned __int16 CanonicalFilename;
  unsigned __int16 v4;
  unsigned int v5;
  unsigned __int16 v6;
  unsigned __int16 v7;
  unsigned __int16 Object;
  int v9;
  int v10;
  int v11;
  int v12;
  int v13;
  int v14;
  char *v15;
  unsigned __int16 VariableIndexInternal;
  int v17;
  int v18;
  int v19;
  int v20;
  __int16 v21;
  __int16 v22;
  unsigned __int16 v24;

  CanonicalFilename = Scr_CreateCanonicalFilename(a1);
  v4 = scrAnimPub_treeRoot;
  v5 = CanonicalFilename;
  v24 = CanonicalFilename;
  v6 = FindNextSibling[2 * (unsigned __int16)FindVariableIndexInternal(scrAnimPub_treeRoot, CanonicalFilename)];
  if ( v6 )
  {
    v11 = xanim_activePoolSlot;
    Object = dword_9B6478[3 * v6];
    v12 = xanim_activePoolSlot << 8;
    *a2 = 0;
    v13 = scrAnimPub_treeCount[v11];
    v14 = 1;
    v15 = (char *)scrAnimPub_treeNames + v12;
    if ( v13 >= 1 )
    {
      while ( *(_WORD *)&v15[2 * v14] != v6 )
      {
        if ( ++v14 > v13 )
          goto LABEL_8;
      }
      *a2 = v14;
    }
  }
  else
  {
    v7 = FindNextSibling[2 * GetVariableIndexInternal(v4, v5)];
    Object = GetObject(v7);
    v9 = xanim_activePoolSlot;
    v10 = scrAnimPub_treeCount[xanim_activePoolSlot] + 1;
    scrAnimPub_treeCount[xanim_activePoolSlot] = v10;
    scrAnimPub_treeNames[128 * v9 + v10] = v7;
    *a2 = v10;
  }
LABEL_8:
  VariableIndexInternal = GetVariableIndexInternal(Object, 0);
  v17 = 3 * (unsigned __int16)FindNextSibling[2 * VariableIndexInternal];
  v18 = FindObject[3 * (unsigned __int16)FindNextSibling[2 * VariableIndexInternal]];
  v19 = 4 * v17 + 10183800;
  if ( (v18 & 0x1F) == 0 )
  {
    *(_DWORD *)(v19 + 4) = v18 | 7;
    v20 = AllocVariable();
    *(_DWORD *)(v20 + 4) = 111;
    *(_WORD *)v20 = 0;
    *(_WORD *)(v20 + 2) = 0;
    *(_WORD *)v19 = (v20 - (int)dword_9B6478) / 12;
  }
  v21 = *(_WORD *)v19;
  v22 = *(_WORD *)(GetRefString_var + 8 * v5);
  if ( v22 )
    *(_WORD *)(GetRefString_var + 8 * v5) = v22 - 1;
  else
    SL_FreeString(
      (char *)(GetRefString_var + 8 * v5 + 4),
      v24,
      GetRefString_var
    + 8 * v5
    + 4
    + strlen((const char *)(GetRefString_var + 8 * v5 + 4))
    + 1
    - (GetRefString_var
     + 8 * v5
     + 5)
    + 1);
  return v21;
}
#endif

/* ---- Scr_UsingTree__FPCcUi  0x00468350 ---- VERIFIED */
__int16 __cdecl Scr_UsingTree(char *a1, int a2)
{
  __int16 result;

  if ( !Scr_IsIdentifier(a1) )
    CompileError(a2, "bad anim tree name");
  result = Scr_UsingTreeInternal(a1, &scrCompileGlob_animTreeValue);
  LOWORD(dword_8E60B0) = result;
  return result;
}

/* ---- Scr_LoadAnimTreeInternal  0x00468390 ---- VERIFIED */
static qboolean Scr_LoadAnimTreeInternal( const char *animTreeName,
										  unsigned short parentHandle,
										  unsigned short treeHandle )
{
	char        filename[64];
	const char *savedSourcePos;
	const char *savedFilename;
	char       *source;
	qboolean    parsedSomething;

	Com_sprintf( filename, sizeof( filename ), "animtrees/%s.atr", animTreeName );

	savedSourcePos = (const char *)scrCompilePub_sourceBuf;

	source = (char *)Scr_AddSourceBuffer( filename, 0, 0 );
	if ( !source ) {
		return qfalse;
	}

	savedFilename = (const char *)scrCompilePub_scriptfilename;
	scrCompilePub_scriptfilename = (int)filename;

	Scr_AnimTreeParse( source, parentHandle, treeHandle );

	Hunk_ClearTempMemoryHigh();
	parsedSomething = GetArraySize( parentHandle ) != 0 ? qtrue : qfalse;

	scrCompilePub_scriptfilename = (int)savedFilename;
	scrCompilePub_sourceBuf      = (int)savedSourcePos;

	return parsedSomething;
}

int Scr_LoadAnimTreeInternal__FPCcUsUs( const char *animTreeName,
										unsigned short parentHandle,
										unsigned short treeHandle ) {
	return Scr_LoadAnimTreeInternal( animTreeName, parentHandle, treeHandle );
}

/* ---- Scr_LoadAnimTreeAtIndex  0x00468450 ---- VERIFIED */
void Scr_LoadAnimTreeAtIndex( int treeIndex, void *(__cdecl *allocFn)( int ) )
{
	unsigned short   treeRecord;
	unsigned short   treeName;
	unsigned short   treeData;
	unsigned short   sourceHandle;
	unsigned short   unresolvedRoot;
	unsigned short   rootName;
	int              nodeCount;
	void            *asset;
	const char      *name;
	scrXAnim_t      *tree;
	VariableValue    value;

	/* retail 0x00468453-0x00468466 indexes the pool with no range check */
	treeRecord = SCR_ANIMTREE_NAME_AT( treeIndex );
	treeName   = (unsigned short)GetVariableName( treeRecord );
	treeData   = FindObject( treeRecord );

	if ( FindVariable( treeData, SCR_ANIMTREE_CHILD_TREE ) ) {
		return;
	}

	sourceHandle = FindVariable( treeData, SCR_ANIMTREE_CHILD_ANIMS );
	if ( !sourceHandle ) {
		SCR_ANIMTREE_AT( treeIndex ) = 0;
		return;
	}

	unresolvedRoot = FindObject( sourceHandle );

	scrAnimPub_currentTreeRoot = Scr_AllocArray();

	if ( !Scr_LoadAnimTreeInternal( SL_ConvertToString( treeName ),
									scrAnimPub_currentTreeRoot, unresolvedRoot ) ) {
		RemoveVariable( scrAnimPub_treeRoot, treeName );
		RemoveRefToObject( scrAnimPub_currentTreeRoot );
		scrAnimPub_currentTreeRoot = 0;
		SCR_ANIMTREE_AT( treeIndex ) = 0;
		return;
	}

	nodeCount = Scr_GetAnimTreeSize( scrAnimPub_currentTreeRoot );

	asset = FS_GetDataForFile( "animtrees", SL_ConvertToString( treeName ), ".atr" );
	name  = asset ? *(const char **)asset : "(savegame)";

	/* --- XAnimAllocTree (0x00484000), inlined as retail does --- */
	tree = (scrXAnim_t *)allocFn( 8 * nodeCount + 8 );
	tree->name      = name;
	tree->nodeCount = nodeCount;

	rootName = SL_GetStringOfLen( "root", 0, strlen( "root" ) + 1,
								  SCR_ANIMTREE_NAME_TYPE );
	ConnectScriptToAnim( unresolvedRoot, SCR_ANIMTREE_CHILD_ANIMS, treeName, rootName,
						 treeIndex );
	SL_RemoveRefToString( rootName );

	Scr_CreateAnimationTree( scrAnimPub_currentTreeRoot, unresolvedRoot, tree,
							 SCR_ANIMTREE_CHILD_TREE, "root", SCR_ANIMTREE_CHILD_ANIMS,
							 treeName, treeIndex );
	Scr_CheckAnimsDefined( unresolvedRoot, treeName );
	Scr_PrecacheAnimationTree( scrAnimPub_currentTreeRoot );

	RemoveVariable( treeData, SCR_ANIMTREE_CHILD_ANIMS );
	RemoveRefToObject( scrAnimPub_currentTreeRoot );
	scrAnimPub_currentTreeRoot = 0;

	value.u.pointerValue = tree;
	value.type           = VAR_CODEPOS;
	SetVariableValue( GetVariable( treeData, SCR_ANIMTREE_CHILD_TREE ), &value );

	XAnimCheckSyncNodes_r( tree, 0 );

	SCR_ANIMTREE_AT( treeIndex ) = (int)tree;
}

int Scr_LoadAnimTreeAtIndex__FiPFi_Pv( int treeIndex, void *(__cdecl *allocFn)( int ) ) {
	Scr_LoadAnimTreeAtIndex( treeIndex, allocFn );
	return 0;
}

/* ---- Scr_FindAnimTree  0x004686E0 ---- VERIFIED */
void * __cdecl Scr_FindAnimTree( const char *name )
{
	unsigned short  canonical;
	unsigned short  entryNode;
	unsigned short  treeNode;

	canonical = Scr_CreateCanonicalFilename( name );
	entryNode = scrVarIndirections[
		FindVariableIndexInternal( (unsigned short)scrAnimPub_treeRoot, canonical ) ].id;
	SL_RemoveRefToString( canonical );

	if ( entryNode == 0 ) {
		return NULL;
	}

	treeNode = scrVarIndirections[
		FindVariableIndexInternal( scrVarNodes[entryNode].u.halfword[0], 1 ) ].id;
	if ( treeNode == 0 ) {
		return NULL;
	}
	return scrVarNodes[treeNode].u.pointerValue;
}
#if 0
int __cdecl Scr_FindAnimTree(char *a1)
{
  unsigned __int16 CanonicalFilename;
  unsigned __int16 v2;
  __int16 v3;
  unsigned __int16 v4;

  CanonicalFilename = Scr_CreateCanonicalFilename(a1);
  v2 = FindNextSibling[2 * (unsigned __int16)FindVariableIndexInternal(scrAnimPub_treeRoot, CanonicalFilename)];
  v3 = *(_WORD *)(GetRefString_var + 8 * CanonicalFilename);
  if ( v3 )
    *(_WORD *)(GetRefString_var + 8 * CanonicalFilename) = v3 - 1;
  else
    SL_FreeString(
      (char *)(GetRefString_var + 8 * CanonicalFilename + 4),
      CanonicalFilename,
      GetRefString_var
    + 8 * CanonicalFilename
    + 4
    + strlen((const char *)(GetRefString_var + 8 * CanonicalFilename + 4))
    + 1
    - (GetRefString_var
     + 8 * CanonicalFilename
     + 5)
    + 1);
  if ( v2 && (v4 = FindNextSibling[2 * (unsigned __int16)FindVariableIndexInternal(dword_9B6478[3 * v2], 1)]) != 0 )
    return dword_9B6478[3 * v4];
  else
    return 0;
}
#endif

/* ---- Scr_FindAnim  0x00468790 ---- VERIFIED */
void Scr_FindAnim( const char *treeName, const char *animName, void *animRef )
{
	unsigned short  animHandle;
	unsigned short  treeHandle;
	int             treeIndex;

	animHandle = SL_GetLowercaseStringOfLen( animName, 0, strlen( animName ) + 1,
											 SCR_ANIMTREE_NAME_TYPE );
	treeHandle = Scr_UsingTreeInternal( treeName, &treeIndex );
	Scr_EmitAnimationInternal( (char *)animRef, animHandle, treeHandle, 0 );
	SL_RemoveRefToString( animHandle );
}
