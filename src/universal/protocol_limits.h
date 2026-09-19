#ifndef PROTOCOL_LIMITS_H
#define PROTOCOL_LIMITS_H

/* First reconstructed build using the extended weapon and fragment format. */
#define OPENCOD_EXTENDED_BUILD 1

#define STOCK_MAX_WEAPONS 64
#define EXTENDED_MAX_WEAPONS 256

#define STOCK_MAX_GAMESTATE_CHARS 16000
#define EXTENDED_MAX_GAMESTATE_CHARS 131072
/* Preserve the stock limit for legacy layouts and code. Expanded storage must
 * explicitly opt in; network limits are selected per connection below. */
#define MAX_GAMESTATE_CHARS STOCK_MAX_GAMESTATE_CHARS

static int Protocol_GameStateLimit( int extended ) {
	return extended ? EXTENDED_MAX_GAMESTATE_CHARS : STOCK_MAX_GAMESTATE_CHARS;
}

#endif
