/*
 * @fidelity: verified
 */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winsock.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../qcommon/qcommon.h"

#ifndef NSPROTO_IPX
#define NSPROTO_IPX 1000
#endif

#define MAX_IPS 16

static SOCKET ip_socket;                    /* 0x008E4E54, alias `s` */
static SOCKET socks_socket;                 /* 0x008E3C58 */
static SOCKET ipx_socket;                   /* 0x008E3C70 */

static struct sockaddr socksRelayAddr;      /* 0x008E3C5C, alias `to` */
static qboolean usingSocks;                 /* 0x014073C0 */

static byte localIP[MAX_IPS][4];            /* 0x008E3C78 */
static int numIP;                           /* 0x008E3CB8 */

/* 0x008E3CC0. The SOCKS5 UDP request: RSV RSV FRAG ATYP DST.ADDR DST.PORT, then the payload from byte 10 on. */
static byte socksBuf[MAX_MSGLEN + 16];

static WSADATA winsockdata;                 /* 0x008E4CC0 */
static qboolean winsockInitialized;         /* 0x014073BC */

static cvar_t *net_noudp;                   /* 0x008E4E5C */
static cvar_t *net_noipx;                   /* 0x008E3C6C */
static cvar_t *net_socksEnabled;            /* 0x008E3C54 */
static cvar_t *net_socksServer;             /* 0x008E4E50 */
static cvar_t *net_socksPort;               /* 0x008E3CBC */
static cvar_t *net_socksUsername;           /* 0x008E3C74 */
static cvar_t *net_socksPassword;           /* 0x008E4E58 */

extern int networkingEnabled;

extern const char *NET_AdrToString( netadr_t a );

static qboolean Sys_StringToSockaddr( struct sockaddr *s, const char *str );
static void NET_OpenSocks( int port );
static void NET_GetLocalAddress( void );

/* ---- NET_ErrorString  0x00464280 ----  VERIFIED */
const char *NET_ErrorString( void ) {
	int code = WSAGetLastError();

	switch ( code ) {
	case WSAEINTR:              return "WSAEINTR";
	case WSAEBADF:              return "WSAEBADF";
	case WSAEACCES:             return "WSAEACCES";
	case WSAEFAULT:             return "WSAEFAULT";
	case WSAEINVAL:             return "WSAEINVAL";
	case WSAEMFILE:             return "WSAEMFILE";
	case WSAEWOULDBLOCK:        return "WSAEWOULDBLOCK";
	case WSAEINPROGRESS:        return "WSAEINPROGRESS";
	case WSAEALREADY:           return "WSAEALREADY";
	case WSAENOTSOCK:           return "WSAENOTSOCK";
	case WSAEDESTADDRREQ:       return "WSAEDESTADDRREQ";
	case WSAEMSGSIZE:           return "WSAEMSGSIZE";
	case WSAEPROTOTYPE:         return "WSAEPROTOTYPE";
	case WSAENOPROTOOPT:        return "WSAENOPROTOOPT";
	case WSAEPROTONOSUPPORT:    return "WSAEPROTONOSUPPORT";
	case WSAESOCKTNOSUPPORT:    return "WSAESOCKTNOSUPPORT";
	case WSAEOPNOTSUPP:         return "WSAEOPNOTSUPP";
	case WSAEPFNOSUPPORT:       return "WSAEPFNOSUPPORT";
	case WSAEAFNOSUPPORT:       return "WSAEAFNOSUPPORT";
	case WSAEADDRINUSE:         return "WSAEADDRINUSE";
	case WSAEADDRNOTAVAIL:      return "WSAEADDRNOTAVAIL";
	case WSAENETDOWN:           return "WSAENETDOWN";
	case WSAENETUNREACH:        return "WSAENETUNREACH";
	case WSAENETRESET:          return "WSAENETRESET";
	case WSAECONNABORTED:       return "WSWSAECONNABORTEDAEINTR";
	case WSAECONNRESET:         return "WSAECONNRESET";
	case WSAENOBUFS:            return "WSAENOBUFS";
	case WSAEISCONN:            return "WSAEISCONN";
	case WSAENOTCONN:           return "WSAENOTCONN";
	case WSAESHUTDOWN:          return "WSAESHUTDOWN";
	case WSAETOOMANYREFS:       return "WSAETOOMANYREFS";
	case WSAETIMEDOUT:          return "WSAETIMEDOUT";
	case WSAECONNREFUSED:       return "WSAECONNREFUSED";
	case WSAELOOP:              return "WSAELOOP";
	case WSAENAMETOOLONG:       return "WSAENAMETOOLONG";
	case WSAEHOSTDOWN:          return "WSAEHOSTDOWN";
	case WSASYSNOTREADY:        return "WSASYSNOTREADY";
	case WSAVERNOTSUPPORTED:    return "WSAVERNOTSUPPORTED";
	case WSANOTINITIALISED:     return "WSANOTINITIALISED";
	case WSAEDISCON:            return "WSAEDISCON";
	case WSAHOST_NOT_FOUND:     return "WSAHOST_NOT_FOUND";
	case WSATRY_AGAIN:          return "WSATRY_AGAIN";
	case WSANO_RECOVERY:        return "WSANO_RECOVERY";
	case WSANO_DATA:            return "WSANO_DATA";
	default:                    return "NO ERROR";
	}
}

/* ---- NetadrToSockadr  0x004644D0 ----  VERIFIED */
void NetadrToSockadr( struct sockaddr *s, const netadr_t *a ) {
	memset( s, 0, sizeof( struct sockaddr ) );

	switch ( a->type ) {
	case NA_BROADCAST:
		*(short *)&s->sa_family = AF_INET;
		*(unsigned short *)&s->sa_data[0] = (unsigned short)a->port;
		*(unsigned int *)&s->sa_data[2] = INADDR_BROADCAST;
		break;

	case NA_IP:
		*(short *)&s->sa_family = AF_INET;
		*(unsigned int *)&s->sa_data[2] = *(const unsigned int *)a->ip;
		*(unsigned short *)&s->sa_data[0] = (unsigned short)a->port;
		break;

	case NA_IPX:
		*(short *)&s->sa_family = AF_IPX;
		memcpy( &s->sa_data[0], a->ipx, 10 );
		*(unsigned short *)&s->sa_data[10] = (unsigned short)a->port;
		break;

	case NA_BROADCAST_IPX:
		*(short *)&s->sa_family = AF_IPX;
		*(unsigned int *)&s->sa_data[0] = 0;
		*(unsigned int *)&s->sa_data[4] = 0xFFFFFFFFu;
		*(unsigned short *)&s->sa_data[8] = 0xFFFFu;
		*(unsigned short *)&s->sa_data[10] = (unsigned short)a->port;
		break;

	default:
		break;
	}
}

/* ---- SockadrToNetadr  0x00464570 ----  VERIFIED */
void SockadrToNetadr( netadr_t *a, const struct sockaddr *s ) {
	if ( *(const short *)&s->sa_family == AF_INET ) {
		a->type = NA_IP;
		*(unsigned int *)a->ip = *(const unsigned int *)&s->sa_data[2];
		a->port = *(const unsigned short *)&s->sa_data[0];
	} else if ( *(const short *)&s->sa_family == AF_IPX ) {
		a->type = NA_IPX;
		memcpy( a->ipx, &s->sa_data[0], 10 );
		a->port = *(const unsigned short *)&s->sa_data[10];
	}
}

/* ---- Sys_StringToSockaddr  0x004645C0 ----  VERIFIED */
static qboolean Sys_StringToSockaddr( struct sockaddr *s, const char *str ) {
	struct hostent *h;
	char pair[3];
	unsigned int b;
	int i, src;

	memset( s, 0, sizeof( struct sockaddr ) );

	if ( strlen( str ) == 21 && str[8] == '.' ) {
		*(short *)&s->sa_family = AF_IPX;
		*(unsigned short *)&s->sa_data[10] = 0;

		pair[2] = 0;
		src = 0;
		for ( i = 0; i < 10; i++ ) {
			if ( i == 4 ) {
				src++;
			}
			pair[0] = str[src];
			pair[1] = str[src + 1];
			sscanf( pair, "%x", &b );
			s->sa_data[i] = (char)b;
			src += 2;
		}
		return qtrue;
	}

	*(short *)&s->sa_family = AF_INET;
	*(unsigned short *)&s->sa_data[0] = 0;

	if ( str[0] >= '0' && str[0] <= '9' ) {
		*(unsigned int *)&s->sa_data[2] = inet_addr( str );
	} else {
		h = gethostbyname( str );
		if ( !h ) {
			return qfalse;
		}
		*(unsigned int *)&s->sa_data[2] = *(unsigned int *)h->h_addr_list[0];
	}

	return qtrue;
}

/* ---- Sys_StringToAdr  0x00464830 ----  VERIFIED */
qboolean Sys_StringToAdr( const char *s, netadr_t *a ) {
	struct sockaddr sadr;

	if ( !Sys_StringToSockaddr( &sadr, s ) ) {
		return qfalse;
	}
	SockadrToNetadr( a, &sadr );
	return qtrue;
}

#define SOCKS_UDP_HEADER_BYTES 10

/* ---- Sys_GetPacket  0x00464890 ----  VERIFIED */
qboolean Sys_GetPacket( msg_t *net_message, netadr_t *net_from ) {
	struct sockaddr from;
	int fromlen;
	int ret, protocol, err;
	SOCKET net_socket;
	byte *buf;

	for ( protocol = 0; protocol < 2; protocol++ ) {
		net_socket = protocol ? ipx_socket : ip_socket;
		if ( !net_socket ) {
			continue;
		}

		fromlen = sizeof( from );
		ret = recvfrom( net_socket, (char *)net_message->data, net_message->maxsize,
						0, &from, &fromlen );
		if ( ret == SOCKET_ERROR ) {
			err = WSAGetLastError();
			if ( err != WSAEWOULDBLOCK && err != WSAECONNRESET ) {
				Com_Printf( "NET_GetPacket: %s\n", NET_ErrorString() );
			}
			continue;
		}

		if ( net_socket == ip_socket ) {
			*(unsigned int *)&from.sa_data[6] = 0;
			*(unsigned int *)&from.sa_data[10] = 0;
		}

		buf = net_message->data;

		if ( usingSocks && net_socket == ip_socket
			 && memcmp( &from, &socksRelayAddr, (size_t)fromlen ) == 0 ) {
			if ( ret < SOCKS_UDP_HEADER_BYTES ) {
				continue;
			}
			if ( buf[0] || buf[1] || buf[2] || buf[3] != 1 ) {
				continue;
			}
			net_from->type = NA_IP;
			net_from->ip[0] = buf[4];
			net_from->ip[1] = buf[5];
			net_from->ip[2] = buf[6];
			net_from->ip[3] = buf[7];
			net_from->port = *(unsigned short *)&buf[8];
			net_message->readcount = SOCKS_UDP_HEADER_BYTES;
		} else {
			SockadrToNetadr( net_from, &from );
			net_message->readcount = 0;
		}

		if ( ret == net_message->maxsize ) {
			Com_Printf( "Oversize packet from %s\n", NET_AdrToString( *net_from ) );
			continue;
		}

		net_message->cursize = ret;
		return qtrue;
	}

	return qfalse;
}

/* ---- Sys_SendPacket  0x00464AA0 ----  VERIFIED */
void Sys_SendPacket( const void *data, int length, netadr_t to ) {
	struct sockaddr addr;
	SOCKET net_socket;
	int ret;

	switch ( to.type ) {
	case NA_BROADCAST:
	case NA_IP:
		net_socket = ip_socket;
		break;
	case NA_IPX:
	case NA_BROADCAST_IPX:
		net_socket = ipx_socket;
		break;
	default:
		Com_Error( ERR_FATAL, "\x15" "Sys_SendPacket: bad address type" );
		return;
	}

	if ( !net_socket ) {
		return;
	}

	NetadrToSockadr( &addr, &to );

	if ( usingSocks && to.type == NA_IP ) {
		socksBuf[0] = 0;
		socksBuf[1] = 0;
		socksBuf[2] = 0;
		socksBuf[3] = 1;
		*(unsigned int *)&socksBuf[4] = *(unsigned int *)&addr.sa_data[2];
		*(unsigned short *)&socksBuf[8] = *(unsigned short *)&addr.sa_data[0];
		memcpy( &socksBuf[SOCKS_UDP_HEADER_BYTES], data, (size_t)(unsigned int)length );
		ret = sendto( net_socket, (const char *)socksBuf,
					  length + SOCKS_UDP_HEADER_BYTES, 0,
					  &socksRelayAddr, sizeof( socksRelayAddr ) );
	} else {
		ret = sendto( net_socket, (const char *)data, length, 0,
					  &addr, sizeof( addr ) );
	}

	if ( ret == SOCKET_ERROR ) {
		int err = WSAGetLastError();

		if ( err == WSAEWOULDBLOCK ) {
			return;
		}
		if ( err == WSAEADDRNOTAVAIL
			 && ( to.type == NA_BROADCAST || to.type == NA_BROADCAST_IPX ) ) {
			return;
		}
		Com_Printf( "Sys_SendPacket: %s\n", NET_ErrorString() );
	}
}

/* ---- Sys_IsLANAddress  0x00464BE0 ----  VERIFIED */
qboolean Sys_IsLANAddress( netadr_t adr ) {
	int i;

	if ( adr.type == NA_LOOPBACK || adr.type == NA_BOT || adr.type == NA_IPX ) {
		return qtrue;
	}
	if ( adr.type != NA_IP ) {
		return qfalse;
	}

	if ( adr.ip[0] == 127 && adr.ip[1] == 0 && adr.ip[2] == 0 && adr.ip[3] == 1 ) {
		return qtrue;
	}

	if ( adr.ip[0] < 128 ) {
		for ( i = 0; i < numIP; i++ ) {
			if ( adr.ip[0] == localIP[i][0] ) {
				return qtrue;
			}
		}
		return qfalse;
	}

	if ( ( adr.ip[0] & 0xC0 ) == 0x80 ) {
		for ( i = 0; i < numIP; i++ ) {
			if ( adr.ip[0] == localIP[i][0] && adr.ip[1] == localIP[i][1] ) {
				return qtrue;
			}
			if ( adr.ip[0] == 0xAC && localIP[i][0] == 0xAC
				 && ( adr.ip[1] & 0xF0 ) == 0x10
				 && ( localIP[i][1] & 0xF0 ) == 0x10 ) {
				return qtrue;
			}
		}
		return qfalse;
	}

	for ( i = 0; i < numIP; i++ ) {
		if ( adr.ip[0] == localIP[i][0] && adr.ip[1] == localIP[i][1]
			 && adr.ip[2] == localIP[i][2] ) {
			return qtrue;
		}
		if ( adr.ip[0] == 0xC0 && localIP[i][0] == 0xC0
			 && adr.ip[1] == 0xA8 && localIP[i][1] == 0xA8 ) {
			return qtrue;
		}
	}
	return qfalse;
}

/* ---- Sys_ShowIP  0x00464D10 ----  VERIFIED */
void Sys_ShowIP( void ) {
	int i;

	for ( i = 0; i < numIP; i++ ) {
		Com_Printf( "IP: %i.%i.%i.%i\n",
					localIP[i][0], localIP[i][1], localIP[i][2], localIP[i][3] );
	}
}

/* ---- NET_IPSocket  0x00464D60 ----  VERIFIED */
static SOCKET NET_IPSocket( const char *net_interface, int port ) {
	SOCKET newsocket;
	struct sockaddr address;
	u_long _true = 1;
	int i = 1;

	if ( net_interface ) {
		Com_Printf( "Opening IP socket: %s:%i\n", net_interface, port );
	} else {
		Com_Printf( "Opening IP socket: localhost:%i\n", port );
	}

	newsocket = socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP );
	if ( newsocket == INVALID_SOCKET ) {
		if ( WSAGetLastError() != WSAEAFNOSUPPORT ) {
			Com_Printf( "WARNING: UDP_OpenSocket: socket: %s\n", NET_ErrorString() );
		}
		return 0;
	}

	if ( ioctlsocket( newsocket, FIONBIO, &_true ) == SOCKET_ERROR ) {
		Com_Printf( "WARNING: UDP_OpenSocket: ioctl FIONBIO: %s\n", NET_ErrorString() );
		return 0;
	}
	if ( setsockopt( newsocket, SOL_SOCKET, SO_BROADCAST, (char *)&i, sizeof( i ) )
		 == SOCKET_ERROR ) {
		Com_Printf( "WARNING: UDP_OpenSocket: setsockopt SO_BROADCAST: %s\n",
					NET_ErrorString() );
		return 0;
	}

	if ( net_interface && net_interface[0] && Q_stricmp( "localhost", net_interface ) ) {
		Sys_StringToSockaddr( &address, net_interface );
	} else {
		*(unsigned int *)&address.sa_data[2] = INADDR_ANY;
	}

	if ( port == PORT_ANY ) {
		*(unsigned short *)&address.sa_data[0] = 0;
	} else {
		*(unsigned short *)&address.sa_data[0] = htons( (u_short)port );
	}
	*(short *)&address.sa_family = AF_INET;

	if ( bind( newsocket, &address, sizeof( address ) ) == SOCKET_ERROR ) {
		Com_Printf( "WARNING: UDP_OpenSocket: bind: %s\n", NET_ErrorString() );
		closesocket( newsocket );
		return 0;
	}

	return newsocket;
}

/* ---- NET_IPXSocket  0x00465560 ----  VERIFIED */
static SOCKET NET_IPXSocket( int port ) {
	SOCKET newsocket;
	struct sockaddr address;
	u_long _true = 1;

	newsocket = socket( AF_IPX, SOCK_DGRAM, NSPROTO_IPX );
	if ( newsocket == INVALID_SOCKET ) {
		if ( WSAGetLastError() != WSAEAFNOSUPPORT ) {
			Com_Printf( "WARNING: IPX_Socket: socket: %s\n", NET_ErrorString() );
		}
		return 0;
	}

	if ( ioctlsocket( newsocket, FIONBIO, &_true ) == SOCKET_ERROR ) {
		Com_Printf( "WARNING: IPX_Socket: ioctl FIONBIO: %s\n", NET_ErrorString() );
		return 0;
	}
	if ( setsockopt( newsocket, SOL_SOCKET, SO_BROADCAST, (char *)&_true,
					 sizeof( _true ) ) == SOCKET_ERROR ) {
		Com_Printf( "WARNING: IPX_Socket: setsockopt SO_BROADCAST: %s\n",
					NET_ErrorString() );
		return 0;
	}

	*(short *)&address.sa_family = AF_IPX;
	memset( address.sa_data, 0, 10 );
	*(unsigned short *)&address.sa_data[10] =
		( port == PORT_ANY ) ? 0 : htons( (u_short)port );

	if ( bind( newsocket, &address, 14 ) == SOCKET_ERROR ) {
		Com_Printf( "WARNING: IPX_Socket: bind: %s\n", NET_ErrorString() );
		closesocket( newsocket );
		return 0;
	}

	return newsocket;
}

/* ---- NET_OpenSocks  0x00464F10 ----  VERIFIED */
static void NET_OpenSocks( int port ) {
	struct sockaddr address;
	struct hostent *h;
	int len, rc;
	byte buf[64];
	qboolean rfc1929;
	int ulen, plen;

	usingSocks = qfalse;

	Com_Printf( "Opening connection to SOCKS server.\n" );

	socks_socket = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );
	if ( socks_socket == INVALID_SOCKET ) {
		Com_Printf( "WARNING: NET_OpenSocks: socket: %s\n", NET_ErrorString() );
		return;
	}

	h = gethostbyname( net_socksServer->string );
	if ( h == NULL ) {
		Com_Printf( "WARNING: NET_OpenSocks: gethostbyname: %s\n", NET_ErrorString() );
		return;
	}
	if ( h->h_addrtype != AF_INET ) {
		Com_Printf( "WARNING: NET_OpenSocks: gethostbyname: address type was not AF_INET\n" );
		return;
	}

	*(short *)&address.sa_family = AF_INET;
	*(unsigned int *)&address.sa_data[2] = *(unsigned int *)h->h_addr_list[0];
	*(unsigned short *)&address.sa_data[0] = htons( (u_short)net_socksPort->integer );

	if ( connect( socks_socket, &address, sizeof( address ) ) == SOCKET_ERROR ) {
		Com_Printf( "NET_OpenSocks: connect: %s\n", NET_ErrorString() );
		return;
	}

	rfc1929 = (qboolean)( net_socksUsername->string[0] || net_socksPassword->string[0] );

	buf[0] = 5;
	if ( rfc1929 ) {
		buf[1] = 2;
		len = 4;
	} else {
		buf[1] = 1;
		len = 3;
	}
	buf[2] = 0;
	if ( rfc1929 ) {
		buf[3] = 2;
	}

	if ( send( socks_socket, (char *)buf, len, 0 ) == SOCKET_ERROR ) {
		Com_Printf( "NET_OpenSocks: send: %s\n", NET_ErrorString() );
		return;
	}

	rc = recv( socks_socket, (char *)buf, sizeof( buf ), 0 );
	if ( rc == SOCKET_ERROR ) {
		Com_Printf( "NET_OpenSocks: recv: %s\n", NET_ErrorString() );
		return;
	}
	if ( rc != 2 || buf[0] != 5 ) {
		Com_Printf( "NET_OpenSocks: bad response\n" );
		return;
	}
	if ( buf[1] != 0 && buf[1] != 2 ) {
		Com_Printf( "NET_OpenSocks: request denied\n" );
		return;
	}

	if ( buf[1] == 2 ) {
		ulen = strlen( net_socksUsername->string );
		plen = strlen( net_socksPassword->string );

		buf[0] = 1;
		buf[1] = (byte)ulen;
		if ( ulen ) {
			memcpy( &buf[2], net_socksUsername->string, (size_t)ulen );
		}
		buf[2 + ulen] = (byte)plen;
		if ( plen ) {
			memcpy( &buf[3 + ulen], net_socksPassword->string, (size_t)plen );
		}

		if ( send( socks_socket, (char *)buf, 3 + ulen + plen, 0 ) == SOCKET_ERROR ) {
			Com_Printf( "NET_OpenSocks: send: %s\n", NET_ErrorString() );
			return;
		}

		rc = recv( socks_socket, (char *)buf, sizeof( buf ), 0 );
		if ( rc == SOCKET_ERROR ) {
			Com_Printf( "NET_OpenSocks: recv: %s\n", NET_ErrorString() );
			return;
		}
		if ( rc != 2 || buf[0] != 1 ) {
			Com_Printf( "NET_OpenSocks: bad response\n" );
			return;
		}
		if ( buf[1] != 0 ) {
			Com_Printf( "NET_OpenSocks: authentication failed\n" );
			return;
		}
	}

	buf[0] = 5;
	buf[1] = 3;
	buf[2] = 0;
	buf[3] = 1;
	*(unsigned int *)&buf[4] = 0;
	*(unsigned short *)&buf[8] = htons( (u_short)port );

	if ( send( socks_socket, (char *)buf, 10, 0 ) == SOCKET_ERROR ) {
		Com_Printf( "NET_OpenSocks: send: %s\n", NET_ErrorString() );
	}

	rc = recv( socks_socket, (char *)buf, sizeof( buf ), 0 );
	if ( rc == SOCKET_ERROR ) {
		Com_Printf( "NET_OpenSocks: recv: %s\n", NET_ErrorString() );
		return;
	}
	if ( rc < 2 || buf[0] != 5 ) {
		Com_Printf( "NET_OpenSocks: bad response\n" );
		return;
	}
	if ( buf[1] != 0 ) {
		Com_Printf( "NET_OpenSocks: request denied: %i\n", buf[1] );
		return;
	}
	if ( buf[3] != 1 ) {
		Com_Printf( "NET_OpenSocks: relay address is not IPV4: %i\n", buf[3] );
		return;
	}

	*(short *)&socksRelayAddr.sa_family = AF_INET;
	*(unsigned short *)&socksRelayAddr.sa_data[0] = *(unsigned short *)&buf[8];
	*(unsigned int *)&socksRelayAddr.sa_data[2] = *(unsigned int *)&buf[4];
	*(unsigned int *)&socksRelayAddr.sa_data[6] = 0;
	*(unsigned int *)&socksRelayAddr.sa_data[10] = 0;

	usingSocks = qtrue;
}

/* ---- NET_GetLocalAddress  0x00465360 ----  VERIFIED */
static void NET_GetLocalAddress( void ) {
	char hostname[256];
	struct hostent *hostInfo;
	char *p;
	int i, n;
	unsigned long ip;

	if ( gethostname( hostname, sizeof( hostname ) ) == SOCKET_ERROR ) {
		return;
	}

	hostInfo = gethostbyname( hostname );
	if ( !hostInfo ) {
		return;
	}

	Com_Printf( "Hostname: %s\n", hostInfo->h_name );

	n = 0;
	while ( hostInfo->h_aliases[n] ) {
		Com_Printf( "Alias: %s\n", hostInfo->h_aliases[n] );
		n++;
	}

	if ( hostInfo->h_addrtype != AF_INET ) {
		return;
	}

	numIP = 0;
	i = 0;
	while ( ( p = hostInfo->h_addr_list[i] ) != NULL && numIP < MAX_IPS ) {
		ip = ntohl( *(unsigned int *)p );
		localIP[numIP][0] = p[0];
		localIP[numIP][1] = p[1];
		localIP[numIP][2] = p[2];
		localIP[numIP][3] = p[3];
		Com_Printf( "IP: %i.%i.%i.%i\n",
					(int)( ( ip >> 24 ) & 0xFF ), (int)( ( ip >> 16 ) & 0xFF ),
					(int)( ( ip >> 8 ) & 0xFF ), (int)( ip & 0xFF ) );
		numIP++;
		i = numIP;
	}
}

/* ---- NET_OpenIP  0x004654B0 ----  VERIFIED */
static void NET_OpenIP( void ) {
	cvar_t *ip;
	int port;
	int i;

	ip = Cvar_Get( "net_ip", "localhost", CVAR_LATCH );
	port = Cvar_Get( "net_port", va( "%i", PORT_SERVER ), CVAR_LATCH )->integer;

	for ( i = 0; i < 10; i++ ) {
		ip_socket = NET_IPSocket( ip->string, port + i );
		if ( ip_socket ) {
			Cvar_SetValue( "net_port", (float)( port + i ) );
			if ( net_socksEnabled->integer ) {
				NET_OpenSocks( port + i );
			}
			NET_GetLocalAddress();
			return;
		}
	}

	Com_Printf( "WARNING: Couldn't allocate IP port\n" );
}

/* ---- NET_GetCvars  0x00465700 ----  VERIFIED */
static qboolean NET_GetCvars( void ) {
	qboolean modified = qfalse;

	if ( net_noudp && net_noudp->modified ) {
		modified = qtrue;
	}
	net_noudp = Cvar_Get( "net_noudp", "0", CVAR_LATCH | CVAR_ARCHIVE );

	if ( net_noipx && net_noipx->modified ) {
		modified = qtrue;
	}
	net_noipx = Cvar_Get( "net_noipx", "0", CVAR_LATCH | CVAR_ARCHIVE );

	if ( net_socksEnabled && net_socksEnabled->modified ) {
		modified = qtrue;
	}
	net_socksEnabled = Cvar_Get( "net_socksEnabled", "0", CVAR_LATCH | CVAR_ARCHIVE );

	if ( net_socksServer && net_socksServer->modified ) {
		modified = qtrue;
	}
	net_socksServer = Cvar_Get( "net_socksServer", "", CVAR_LATCH | CVAR_ARCHIVE );

	if ( net_socksPort && net_socksPort->modified ) {
		modified = qtrue;
	}
	net_socksPort = Cvar_Get( "net_socksPort", "1080", CVAR_LATCH | CVAR_ARCHIVE );

	if ( net_socksUsername && net_socksUsername->modified ) {
		modified = qtrue;
	}
	net_socksUsername = Cvar_Get( "net_socksUsername", "", CVAR_LATCH | CVAR_ARCHIVE );

	if ( net_socksPassword && net_socksPassword->modified ) {
		modified = qtrue;
	}
	net_socksPassword = Cvar_Get( "net_socksPassword", "", CVAR_LATCH | CVAR_ARCHIVE );

	return modified;
}

/* ---- NET_Config  0x00465850 ----  VERIFIED */
void NET_Config( qboolean enableNetworking ) {
	qboolean modified;
	qboolean stop, start;

	modified = NET_GetCvars();

	if ( net_noudp->integer && net_noipx->integer ) {
		enableNetworking = qfalse;
	}

	if ( enableNetworking == (qboolean)networkingEnabled ) {
		if ( !modified || !enableNetworking ) {
			return;
		}
		stop = qtrue;
		start = qtrue;
	} else {
		if ( enableNetworking ) {
			stop = qfalse;
			start = qtrue;
		} else {
			stop = qtrue;
			start = qfalse;
		}
		networkingEnabled = enableNetworking;
	}

	if ( stop ) {
		if ( ip_socket && ip_socket != INVALID_SOCKET ) {
			closesocket( ip_socket );
			ip_socket = 0;
		}
		if ( socks_socket && socks_socket != INVALID_SOCKET ) {
			closesocket( socks_socket );
			socks_socket = 0;
		}
		if ( ipx_socket && ipx_socket != INVALID_SOCKET ) {
			closesocket( ipx_socket );
			ipx_socket = 0;
		}
	}

	if ( start ) {
		if ( !net_noudp->integer ) {
			NET_OpenIP();
		}
		if ( !net_noipx->integer ) {
			ipx_socket = NET_IPXSocket(
				Cvar_Get( "net_port", va( "%i", PORT_SERVER ), CVAR_LATCH )->integer );
		}
	}
}

/* ---- NET_Init  0x00465940 ----  VERIFIED */
void NET_Init( void ) {
	int r;

	r = WSAStartup( MAKEWORD( 1, 1 ), &winsockdata );
	if ( r ) {
		Com_Printf( "WARNING: Winsock initialization failed, returned %d\n", r );
		return;
	}

	winsockInitialized = qtrue;
	Com_Printf( "Winsock Initialized\n" );

	NET_GetCvars();
	NET_Config( qtrue );
}

/* ---- NET_Shutdown  0x00465990 ----  VERIFIED */
void NET_Shutdown( void ) {
	if ( !winsockInitialized ) {
		return;
	}
	NET_Config( qfalse );
	WSACleanup();
	winsockInitialized = qfalse;
}

/* ---- NET_Sleep  0x004659B0 ----  VERIFIED */
void NET_Sleep( int msec ) {
	Sleep( (DWORD)msec );
}

/* ---- NET_Restart  0x004659C0 ----  VERIFIED */
void NET_Restart( void ) {
	NET_Config( (qboolean)networkingEnabled );
}
