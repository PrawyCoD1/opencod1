/*
 * script/scr_stringlist.cpp
 *
 * Original translation unit:
 *   /Volumes/BigCheese/ Source/AspyrP4/CoD/Source/script/scr_stringlist.cpp
 *
 * Retail range 0x0046F7C0-0x004704B0, 28 functions.
 *
 * @fidelity: verified
 */

#include "scr_local.h"
#include <malloc.h>

scrStringHashSlot_t     scrStringHashSlots[SCR_STRING_HASH_SLOT_COUNT];

scrStringHashSlot_t    *scrString_freedHashSlot;

unsigned short         *scrString_canonicalMap;
unsigned short          scrString_canonicalCount;

/* ---- GetRefString  0x0046F7C0 ---- VERIFIED */
scrStringEntry_t *GetRefString( unsigned short handle ) {
	return (scrStringEntry_t *)&scrMemTree_blocks[handle];
}

/* ---- GetRefString  0x0046F7D0 ---- VERIFIED */
static scrStringEntry_t *GetRefStringByText( const char *text ) {
	return (scrStringEntry_t *)( text - SCR_STRING_ENTRY_HEADER_SIZE );
}

/* ---- SL_ConvertToString  0x0046F7E0 ---- VERIFIED */
const char *SL_ConvertToString( unsigned short handle ) {
	if ( handle == 0 ) {
		return NULL;
	}
	return GetRefString( handle )->text;
}

/* ---- SL_ConvertFromString  0x0046F800 ---- VERIFIED */
unsigned short SL_ConvertFromString( const char *text ) {
	scrStringEntry_t *entry = GetRefStringByText( text );

	return (unsigned short)( ( (const char *)entry - (const char *)scrMemTree_blocks )
							 / SCR_MT_BLOCK_SIZE );
}

/* ---- GetHashCode  0x0046F820 ---- VERIFIED */
static unsigned short GetHashCode( const char *text, unsigned int size ) {
	unsigned int hash;
	unsigned int i;

	if ( size >= SCR_STRING_HASH_SHORT_LIMIT ) {
		hash = size >> 2;
	} else {
		hash = 0;
		for ( i = 0; i < size; i++ ) {
			hash = hash * SCR_STRING_HASH_MULTIPLIER + (unsigned int)(int)(signed char)text[i];
		}
	}

	return (unsigned short)( hash % SCR_STRING_HASH_LINK_MASK + SCR_STRING_HASH_FIRST_SLOT );
}

/* ---- SL_Init  0x0046F860 ---- VERIFIED */
void SL_Init( void ) {
	unsigned short previous;
	unsigned short slot;

	MT_Init();

	scrStringHashSlots[0].linkAndFlags = 0;
	previous = 0;
	for ( slot = SCR_STRING_HASH_FIRST_SLOT;
		  slot < SCR_STRING_HASH_SLOT_COUNT;
		  slot++ ) {
		scrStringHashSlots[slot].linkAndFlags = SCR_STRING_HASH_EMPTY;
		scrStringHashSlots[previous].linkAndFlags =
			(unsigned short)( scrStringHashSlots[previous].linkAndFlags | slot );
		scrStringHashSlots[slot].stringHandle = previous;
		previous = slot;
	}
	scrStringHashSlots[0].stringHandle = previous;
}

static qboolean SL_EntryMatches( const scrStringHashSlot_t *slot,
								 const char *text, unsigned int size ) {
	scrStringEntry_t *entry = GetRefString( slot->stringHandle );

	if ( entry->sizeByte == (unsigned char)size
		 && memcmp( entry->text, text, size ) == 0 ) {
		return qtrue;
	}
	return qfalse;
}

static void SL_MoveSlotToHashHead( unsigned short hash,
								   unsigned short previousSlot,
								   unsigned short matchSlot ) {
	scrStringHashSlot_t *head     = &scrStringHashSlots[hash];
	scrStringHashSlot_t *previous = &scrStringHashSlots[previousSlot];
	scrStringHashSlot_t *match    = &scrStringHashSlots[matchSlot];
	unsigned short       movedHandle;

	previous->linkAndFlags =
		(unsigned short)( ( match->linkAndFlags & SCR_STRING_HASH_LINK_MASK )
						  | ( previous->linkAndFlags & SCR_STRING_HASH_FLAGS_MASK ) );
	match->linkAndFlags =
		(unsigned short)( ( head->linkAndFlags & SCR_STRING_HASH_LINK_MASK )
						  | ( match->linkAndFlags & SCR_STRING_HASH_FLAGS_MASK ) );
	head->linkAndFlags =
		(unsigned short)( matchSlot
						  | ( head->linkAndFlags & SCR_STRING_HASH_FLAGS_MASK ) );

	movedHandle = match->stringHandle;
	match->stringHandle = head->stringHandle;
	head->stringHandle = movedHandle;
}

/* ---- SL_FindStringOfLen  0x0046F8E0 ---- VERIFIED */
unsigned short SL_FindStringOfLen( const char *text, unsigned int size ) {
	unsigned short          hash;
	scrStringHashSlot_t    *head;
	scrStringHashSlot_t    *current;
	unsigned short          previousSlot;
	unsigned short          currentSlot;

	hash = GetHashCode( text, size );
	head = &scrStringHashSlots[hash];

	if ( ( head->linkAndFlags & SCR_STRING_HASH_FLAGS_MASK )
		 != SCR_STRING_HASH_OCCUPIED ) {
		return 0;
	}

	if ( SL_EntryMatches( head, text, size ) ) {
		return head->stringHandle;
	}

	previousSlot = hash;
	currentSlot = head->linkAndFlags;
	for ( ;; ) {
		currentSlot &= SCR_STRING_HASH_LINK_MASK;
		current = &scrStringHashSlots[currentSlot];
		if ( current == head ) {
			return 0;
		}
		if ( SL_EntryMatches( current, text, size ) ) {
			SL_MoveSlotToHashHead( hash, previousSlot, currentSlot );
			return head->stringHandle;
		}
		previousSlot = currentSlot;
		currentSlot = current->linkAndFlags;
	}
}

/* ---- SL_FindString  0x0046FA30 ---- VERIFIED */
unsigned short SL_FindString( const char *text ) {
	return SL_FindStringOfLen( text, (unsigned int)strlen( text ) + 1 );
}

/* ---- SL_FindLowercaseString  0x0046FA50 ---- VERIFIED */
unsigned short SL_FindLowercaseString( const char *text ) {
	unsigned int    size;
	char           *lowercase;
	int             i;

	size = (unsigned int)strlen( text ) + 1;
	lowercase = (char *)_alloca( size );

	for ( i = (int)size - 1; i >= 0; i-- ) {
		lowercase[i] = (char)tolower( (unsigned char)text[i] );
	}

	return SL_FindStringOfLen( lowercase, size );
}

static unsigned short SL_PopFreeHashSlot( void ) {
	unsigned short freeSlot;

	freeSlot = scrStringHashSlots[0].linkAndFlags;
	if ( freeSlot == 0 ) {
		Scr_DumpScriptThreads();
		Com_Error( ERR_DROP, "\x15" "exceeded maximum number of script strings\n" );
	}

	scrStringHashSlots[0].linkAndFlags =
		(unsigned short)( scrStringHashSlots[freeSlot].linkAndFlags
						  & SCR_STRING_HASH_LINK_MASK );
	scrStringHashSlots[scrStringHashSlots[0].linkAndFlags].stringHandle = 0;

	return freeSlot;
}

static void SL_InsertIntoOccupiedHead( unsigned short hash, unsigned short freeSlot ) {
	scrStringHashSlot_t *head = &scrStringHashSlots[hash];
	scrStringHashSlot_t *slot = &scrStringHashSlots[freeSlot];

	slot->linkAndFlags =
		(unsigned short)( ( head->linkAndFlags & SCR_STRING_HASH_LINK_MASK )
						  | SCR_STRING_HASH_CHAINED );
	head->linkAndFlags =
		(unsigned short)( ( head->linkAndFlags & SCR_STRING_HASH_FLAGS_MASK )
						  | freeSlot );
	slot->stringHandle = head->stringHandle;
}

static void SL_UnlinkFreeSlot( unsigned short slot ) {
	unsigned short next;
	unsigned short previous;

	next = (unsigned short)( scrStringHashSlots[slot].linkAndFlags
							 & SCR_STRING_HASH_LINK_MASK );
	previous = scrStringHashSlots[slot].stringHandle;

	scrStringHashSlots[previous].linkAndFlags =
		(unsigned short)( next
						  | ( scrStringHashSlots[previous].linkAndFlags
							  & SCR_STRING_HASH_FLAGS_MASK ) );
	scrStringHashSlots[next].stringHandle = previous;
}

static void SL_InsertIntoLinkedHash( unsigned short hash, unsigned short freeSlot ) {
	unsigned short previousSlot;
	unsigned short evictedLink;

	evictedLink = (unsigned short)( scrStringHashSlots[hash].linkAndFlags
									& SCR_STRING_HASH_LINK_MASK );

	previousSlot = evictedLink;
	while ( ( scrStringHashSlots[previousSlot].linkAndFlags
			  & SCR_STRING_HASH_LINK_MASK ) != hash ) {
		previousSlot = (unsigned short)( scrStringHashSlots[previousSlot].linkAndFlags
										 & SCR_STRING_HASH_LINK_MASK );
	}

	scrStringHashSlots[previousSlot].linkAndFlags =
		(unsigned short)( ( scrStringHashSlots[previousSlot].linkAndFlags
							& SCR_STRING_HASH_FLAGS_MASK )
						  | freeSlot );
	scrStringHashSlots[freeSlot].linkAndFlags =
		(unsigned short)( evictedLink | SCR_STRING_HASH_CHAINED );
	scrStringHashSlots[freeSlot].stringHandle = scrStringHashSlots[hash].stringHandle;
}

/* SL_AddUser  0x0046FD8B */
static void SL_AddUser( scrStringHashSlot_t *head, unsigned char user ) {
	scrStringEntry_t *entry = GetRefString( head->stringHandle );

	if ( ( entry->flags & user ) == 0 ) {
		entry->flags = (unsigned char)( entry->flags | user );
		entry->refCount++;
	}
}

/* ---- SL_GetStringOfLen  0x0046FAC0 ---- VERIFIED */
unsigned short SL_GetStringOfLen( const char *text, unsigned char user,
								  unsigned int size, int type ) {
	unsigned short          hash;
	scrStringHashSlot_t    *head;
	scrStringHashSlot_t    *current;
	unsigned short          previousSlot;
	unsigned short          currentSlot;
	scrStringEntry_t       *entry;

	hash = GetHashCode( text, size );
	head = &scrStringHashSlots[hash];

	if ( ( head->linkAndFlags & SCR_STRING_HASH_FLAGS_MASK )
		 == SCR_STRING_HASH_OCCUPIED ) {
		if ( SL_EntryMatches( head, text, size ) ) {
			SL_AddUser( head, user );
			return head->stringHandle;
		}

		previousSlot = hash;
		currentSlot = head->linkAndFlags;
		for ( ;; ) {
			currentSlot &= SCR_STRING_HASH_LINK_MASK;
			current = &scrStringHashSlots[currentSlot];
			if ( current == head ) {
				break;
			}
			if ( SL_EntryMatches( current, text, size ) ) {
				SL_MoveSlotToHashHead( hash, previousSlot, currentSlot );
				SL_AddUser( head, user );
				return head->stringHandle;
			}
			previousSlot = currentSlot;
			currentSlot = current->linkAndFlags;
		}

		SL_InsertIntoOccupiedHead( hash, SL_PopFreeHashSlot() );
	} else {
		if ( ( head->linkAndFlags & SCR_STRING_HASH_FLAGS_MASK )
			 == SCR_STRING_HASH_EMPTY ) {
			SL_UnlinkFreeSlot( hash );
		} else {
			SL_InsertIntoLinkedHash( hash, SL_PopFreeHashSlot() );
		}
		head->linkAndFlags = (unsigned short)( hash | SCR_STRING_HASH_OCCUPIED );
	}

	head->stringHandle =
		MT_AllocIndex( (int)( size + SCR_STRING_ENTRY_HEADER_SIZE ), type );
	entry = GetRefString( head->stringHandle );
	memcpy( entry->text, text, size );
	entry->flags = user;
	entry->refCount = 0;
	entry->sizeByte = (unsigned char)size;

	return head->stringHandle;
}

/* ---- SL_GetString_  0x0046FE00 ---- VERIFIED */
unsigned short SL_GetString_( const char *text, unsigned char user, int type ) {
	return SL_GetStringOfLen( text, user, (unsigned int)strlen( text ) + 1, type );
}

/* ---- SL_GetString  0x0046FE30 ---- VERIFIED */
unsigned short SL_GetString( const char *text, unsigned char user ) {
	return SL_GetString_( text, user, 6 );
}

/* ---- SL_GetLowercaseStringOfLen  0x0046FE60 ---- VERIFIED */
unsigned short SL_GetLowercaseStringOfLen( const char *text, unsigned char user,
										   unsigned int size, int type ) {
	char   *lowercase;
	int     i;

	lowercase = (char *)_alloca( size );
	for ( i = (int)size - 1; i >= 0; i-- ) {
		lowercase[i] = (char)tolower( (unsigned char)text[i] );
	}

	return SL_GetStringOfLen( lowercase, user, size, type );
}

/* ---- SL_GetLowercaseString_  0x0046FED0 ---- VERIFIED */
unsigned short SL_GetLowercaseString_( const char *text, unsigned char user, int type ) {
	return SL_GetLowercaseStringOfLen( text, user,
									   (unsigned int)strlen( text ) + 1, type );
}

/* ---- SL_GetLowercaseString  0x0046FF00 ---- VERIFIED */
unsigned short SL_GetLowercaseString( const char *text, unsigned char user ) {
	return SL_GetLowercaseString_( text, user, 6 );
}

/* ---- SL_TransferRefToString  0x0046FF30 ---- VERIFIED */
void SL_TransferRefToString( unsigned short handle, unsigned char usage ) {
	scrStringEntry_t *entry = GetRefString( handle );

	if ( ( entry->flags & usage ) == 0 ) {
		entry->flags = (unsigned char)( entry->flags | usage );
		return;
	}
	entry->refCount--;
}

/* ---- SL_AddRefToString  0x0046FF50 ---- VERIFIED */
void SL_AddRefToString( unsigned short handle ) {
	GetRefString( handle )->refCount++;
}

/* ---- SL_FreeString  0x0046FF70 ---- VERIFIED */
void SL_FreeString( unsigned short handle, const char *text, unsigned int size ) {
	unsigned short          hash;
	scrStringHashSlot_t    *head;
	scrStringHashSlot_t    *freeSlot;
	scrStringHashSlot_t    *replacement;
	unsigned short          freeSlotIndex;
	unsigned short          replacementSlot;
	unsigned short          previousSlot;
	unsigned short          oldFreeHead;

	hash = GetHashCode( text, size );
	head = &scrStringHashSlots[hash];
	freeSlot = head;
	freeSlotIndex = hash;

	MT_FreeIndex( handle, size + SCR_STRING_ENTRY_HEADER_SIZE );

	replacementSlot = (unsigned short)( head->linkAndFlags & SCR_STRING_HASH_LINK_MASK );
	replacement = &scrStringHashSlots[replacementSlot];

	if ( head->stringHandle == handle ) {
		if ( replacement != head ) {
			head->linkAndFlags =
				(unsigned short)( ( replacement->linkAndFlags
									& SCR_STRING_HASH_LINK_MASK )
								  | SCR_STRING_HASH_OCCUPIED );
			head->stringHandle = replacement->stringHandle;
			scrString_freedHashSlot = head;
			freeSlot = replacement;
			freeSlotIndex = replacementSlot;
		}
	} else {
		previousSlot = hash;
		{ int guard = 0;
		while ( replacement->stringHandle != handle ) {
			if ( ++guard > SCR_STRING_HASH_SLOT_COUNT ) {
				extern void Com_Printf( const char *fmt, ... );
				Com_Printf( "[[SLF]] SL_FreeString runaway handle=%u size=%u hash=%u ra=0x%08X\n",
							handle, size, hash, ( (unsigned int *)&handle )[-1] );
				return;
			}
			previousSlot = replacementSlot;
			replacementSlot = (unsigned short)( replacement->linkAndFlags
												& SCR_STRING_HASH_LINK_MASK );
			replacement = &scrStringHashSlots[replacementSlot];
		}
		}
		scrStringHashSlots[previousSlot].linkAndFlags =
			(unsigned short)( ( replacement->linkAndFlags & SCR_STRING_HASH_LINK_MASK )
							  | ( scrStringHashSlots[previousSlot].linkAndFlags
								  & SCR_STRING_HASH_FLAGS_MASK ) );
		freeSlot = replacement;
		freeSlotIndex = replacementSlot;
	}

	oldFreeHead = scrStringHashSlots[0].linkAndFlags;
	freeSlot->linkAndFlags = oldFreeHead;
	freeSlot->stringHandle = 0;
	scrStringHashSlots[oldFreeHead].stringHandle = freeSlotIndex;
	scrStringHashSlots[0].linkAndFlags = freeSlotIndex;
}

/* ---- SL_RemoveRefToString  0x00470050 ---- VERIFIED */
void SL_RemoveRefToString( unsigned short handle ) {
	scrStringEntry_t *entry = GetRefString( handle );

	if ( entry->refCount == 0 ) {
		SL_FreeString( handle, entry->text, (unsigned int)strlen( entry->text ) + 1 );
		return;
	}
	entry->refCount--;
}

/* ---- SL_RemoveRefToStringOfLen  0x00470090 ---- VERIFIED */
void SL_RemoveRefToStringOfLen( unsigned short handle, unsigned int size ) {
	scrStringEntry_t *entry = GetRefString( handle );

	if ( entry->refCount == 0 ) {
		SL_FreeString( handle, entry->text, size );
		return;
	}
	entry->refCount--;
}

/* ---- Scr_SetString  0x004700C0 ---- VERIFIED */
void Scr_SetString( unsigned short *slot, unsigned short value ) {
	if ( *slot ) {
		SL_RemoveRefToString( *slot );
	}
	if ( value ) {
		SL_AddRefToString( value );
	}
	*slot = value;
}

/* ---- Scr_AllocString  0x00470130 ---- VERIFIED */
unsigned short Scr_AllocString( const char *text ) {
	return SL_GetString( text, SCR_STRING_USER_SCRIPT );
}

/* ---- SL_GetStringForFloat  0x00470160 ---- VERIFIED */
unsigned short SL_GetStringForFloat( float value ) {
	char text[SCR_STRING_FORMAT_BUFFER_SIZE];

	sprintf( text, "%g", (double)value );
	return SL_GetString_( text, 0, 14 );
}

/* ---- SL_GetStringForInt  0x004701E0 ---- */
unsigned short SL_GetStringForInt( int value ) {
	char text[SCR_STRING_FORMAT_BUFFER_SIZE];

	sprintf( text, "%i", value );
	return SL_GetString_( text, 0, 14 );
}

/* ---- SL_GetStringForVector  0x00470260 ---- */
unsigned short SL_GetStringForVector( const float *v ) {
	char text[SCR_STRING_FORMAT_BUFFER_SIZE];

	sprintf( text, "(%.2f, %.2f, %.2f)", (double)v[0], (double)v[1], (double)v[2] );
	return SL_GetString_( text, 0, 14 );
}

/* ---- SL_ShutdownSystem  0x004702F0 ---- VERIFIED */
void SL_ShutdownSystem( unsigned char usage ) {
	unsigned short          slot;
	scrStringHashSlot_t    *hashSlot;
	scrStringEntry_t       *entry;

	for ( slot = SCR_STRING_HASH_FIRST_SLOT;
		  slot < SCR_STRING_HASH_SLOT_COUNT;
		  slot++ ) {
		for ( ;; ) {
			hashSlot = &scrStringHashSlots[slot];
			if ( ( hashSlot->linkAndFlags & SCR_STRING_HASH_FLAGS_MASK )
				 == SCR_STRING_HASH_EMPTY ) {
				break;
			}

			entry = GetRefString( hashSlot->stringHandle );
			if ( ( entry->flags & usage ) == 0 ) {
				break;
			}
			entry->flags = (unsigned char)( entry->flags & ~usage );

			scrString_freedHashSlot = NULL;
			SL_RemoveRefToString( hashSlot->stringHandle );
			if ( scrString_freedHashSlot == NULL ) {
				break;
			}
		}
	}
}

/* ---- CreateCanonicalFilename  0x00470390 ---- VERIFIED */
void CreateCanonicalFilename( char *dest, const char *source, int maxLength ) {
	unsigned char ch;

	for ( ;; ) {
		do {
			do {
				ch = (unsigned char)*source++;
			} while ( ch == '\\' );
		} while ( ch == '/' );

		while ( ch > 0x1F ) {
			*dest++ = (char)tolower( ch );
			maxLength--;
			if ( maxLength == 0 ) {
				Com_Error( ERR_DROP,
						   "\x15" "Filename '%s' exceeds maximum length of %d",
						   source, maxLength );
			}
			if ( ch == '\\' ) {
				break;
			}

			ch = (unsigned char)*source++;
			if ( ch == '/' ) {
				ch = '\\';
			}
		}

		if ( ch == '\0' ) {
			*dest = '\0';
			return;
		}
	}
}

/* ---- Scr_CreateCanonicalFilename  0x00470400 ---- VERIFIED */
unsigned short Scr_CreateCanonicalFilename( const char *filename ) {
	char canonical[SCR_STRING_CANONICAL_LIMIT];

	CreateCanonicalFilename( canonical, filename, SCR_STRING_CANONICAL_LIMIT );
	return SL_GetString_( canonical, 0, 7 );
}

void SL_FreeString__FUsPCcUi( unsigned short handle, const char *text,
							  unsigned int size ) {
	SL_FreeString( handle, text, size );
}

unsigned short GetHashCode_SL__FPCcUi( const char *text, unsigned int size ) {
	return GetHashCode( text, size );
}

scrStringEntry_t *GetRefString__FUs( unsigned short handle ) {
	return GetRefString( handle );
}

scrStringEntry_t *GetRefString__FPCc( const char *text ) {
	return GetRefStringByText( text );
}

void CreateCanonicalFilename__FPcPCci( char *dest, const char *source, int maxLength ) {
	CreateCanonicalFilename( dest, source, maxLength );
}
