/*
 * 
 *  Copyright (c) 2012
 *  name : mhogo mchungu 
 *  email: mhogomchungu@gmail.com
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 2 of the License, or
 *  (at your option) any later version.
 * 
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 * 
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */ 

#include "socket.h"

struct Socket_t
{
	int type ;
	int protocol ;
	int fread ;
	int fwrite ;
	int cmax ;
	int domain ;
	socklen_t size ;
	int fd ;
	struct sockaddr_un * local ;
	struct sockaddr_in * net ;
};

struct SocketPair_t
{
	socket_t first ;
	socket_t second ;
};

socketPair_t SocketPair( void )
{
	socketPair_t sp = ( socketPair_t ) malloc( sizeof( struct SocketPair_t ) ) ;
	return sp ;	
}

void SocketPairSet( socketPair_t sp,socket_t s,size_t index )
{
	switch( index ){
		case 0 : sp->first  = s ; break ;
		case 1 : sp->second = s ; break ;
	}
}

socket_t SocketPairFirst( socketPair_t sp ) 
{
	return sp->first ;
}

socket_t SocketPairSecond( socketPair_t sp )
{
	return sp->second ;
}

void SocketPairDelete( socketPair_t * sp )
{
	socketPair_t s = *sp ;
	*sp = socketPairVoid ;
	
	SocketDelete( &s->first )  ;
	SocketDelete( &s->second ) ;
	
	free( s ) ;
}

socket_t SocketLocal( const char * address )
{
	socket_t s = Socket( "local" ) ;
	SocketSetHostAddress( s,address ) ;
	return s ;
}

socket_t Socket( const char * domain ) 
{
	socket_t s = ( socket_t ) malloc( sizeof( struct Socket_t ) ) ;
	
	if( strcmp( domain,"local" ) == 0 ){
		s->domain = AF_UNIX ;		
		s->size = sizeof( struct sockaddr_un ) ;		
		s->local = ( struct sockaddr_un * ) malloc( s->size ) ;
		memset( s->local,'\0',s->size ) ;			
		s->local->sun_family = AF_UNIX ;
	}else{
		s->domain = AF_INET ;
		s->size = sizeof( struct sockaddr_in ) ;
		s->net = ( struct sockaddr_in  * ) malloc( s->size ) ;
		memset( s->net,'\0',s->size ) ;		
		s->net->sin_family = AF_INET ;		
	}
	
	s->type = SOCK_STREAM ;
	s->protocol = 0 ;
	s->cmax = 1 ;
	s->fread = MSG_WAITALL ;
	s->fwrite = 0 ;
	
	s->fd = socket( s->domain,s->type,s->protocol ) ;
	return s ;	
}

void SocketSetOptionType( socket_t s,int option ) 
{
	s->type = option ;
}

void SocketSetProtocolType( socket_t s,int protocol ) 
{
	s->protocol = protocol ;
}

void SocketDelete( socket_t * x )
{
	socket_t s = *x ;
	*x = NULL ;
	
	if( s->domain == AF_UNIX )
		free( s->local ) ;
	else
		free( s->net ) ;
	
	free( s ) ;
}

void SocketSetPortNumber( socket_t s,int port ) 
{
	if( s->domain == AF_INET )
		s->net->sin_port = htons( port );
}

#ifdef __STDC__
struct addrinfo {
	int     ai_flags;
	int     ai_family;
	int     ai_socktype;
	int     ai_protocol;
	size_t  ai_addrlen;
	struct  sockaddr *ai_addr;
	char    *ai_canonname;     
	struct  addrinfo *ai_next;
};

int getaddrinfo( const char *,const char *,const struct addrinfo *,struct addrinfo ** );
void freeaddrinfo( struct addrinfo * );
#endif

void SocketSetHostAddress( socket_t s,const char * address ) 
{
	struct addrinfo * addr ;
	struct sockaddr_in * addr_in ;
	
	if( s->domain == AF_UNIX )
		strcpy( s->local->sun_path,address ) ;
	else{
		if( getaddrinfo( address,NULL,NULL,&addr ) == 0 ){
			addr_in = ( struct sockaddr_in * ) addr->ai_addr ;		
			s->net->sin_addr.s_addr = addr_in->sin_addr.s_addr ;		
			freeaddrinfo( addr ) ;		
		}
	}
}

const char * SocketAddress( socket_t s )
{	
	if( s->domain == AF_UNIX )
		return s->local->sun_path ;
	else
		return inet_ntoa( s->net->sin_addr ) ;	
}

void SocketSetHostIPAddress( socket_t s,const char * address ) 
{
	if( s->domain == AF_INET )
		s->net->sin_addr.s_addr = inet_addr( address ) ;
}

int SocketBind( socket_t s )
{
	if( s->domain == AF_UNIX ){
		unlink( s->local->sun_path ) ;
		return bind( s->fd,( struct sockaddr * )s->local,s->size ) ;
	}else{
		return bind( s->fd,( struct sockaddr * )s->net,s->size ) ;
	}
}

socket_t SocketAccept( socket_t s ) 
{
	socket_t x = ( socket_t ) malloc( sizeof( struct Socket_t ) ) ;
	
	if( s->domain == AF_UNIX ){
		x->local = ( struct sockaddr_un * ) malloc( s->size ) ;
		x->fd = accept( s->fd,( struct sockaddr * )x->local,&x->size ) ;
	}else{
		x->net = ( struct sockaddr_in * ) malloc( s->size )  ;
		x->fd = accept( s->fd,( struct sockaddr * )x->net,&x->size ) ;
	}
	
	return x ;
}

void SocketSetListenMaximum( socket_t s,int m ) 
{
	s->cmax = m ;
}

int SocketConnect( socket_t s ) 
{
	if( s->domain == AF_UNIX )
		return connect( s->fd,( struct sockaddr * )s->local,s->size ) ;
	else
		return connect( s->fd,( struct sockaddr * )s->net,s->size ) ;	
}

int SocketListen( socket_t s ) 
{
	return listen( s->fd,s->cmax ) ;
}

#define BUFFSIZE 64

size_t SocketGetData( socket_t s,char ** buffer,size_t len ) 
{
	size_t i ;
	size_t buffCount = BUFFSIZE ;
	
	char * c = ( char * ) malloc( sizeof( char ) * BUFFSIZE ) ;
	char * d = NULL ;
	
	if( c == NULL )
		return -1 ;
	
	for( i = 0 ; i < len ; i++ ){
	
		if( i == buffCount ){
			
			buffCount += BUFFSIZE ;
			d = realloc( c,buffCount ) ;
			
			if( d == NULL ){
				free( c ) ;
				return -1 ;
			}else{
				c = d ;
			}
		}
		
		if( recv( s->fd,c + i,1,s->fread ) <= 0 )
			break ;
	}	
	
	if( i > 0 ){
		c = realloc( c,i + 1 ) ;
		*( c + i ) = '\0' ;
		*buffer = c ;
	}else{
		free( c ) ;
	}
	
	return i ;
}

ssize_t SocketSendData( socket_t s,const char * buffer,size_t len ) 
{	
	size_t sent = 0 ;
	size_t remain = len ;
	do{
		sent = sent + send( s->fd,buffer + sent,remain,s->fwrite ) ;		
		remain = remain - sent ;
	}while( sent != len );
	
	return sent ;
}

void SockectSetReadOption( socket_t s,int option ) 
{
	s->fread = option ;
}

void SocketSetWriteOption( socket_t s,int option ) 
{
	s->fwrite = option ;
}

int SocketClose( socket_t s ) 
{
	return close( s->fd ) ;
}
