#ifndef __MSERV_PACKET_H
#define __MSERV_PACKET_H
#include "mserv_config.h"

#include <stdarg.h>

#include <prio.h>

#include "ewfutils.h"

typedef struct _struct_MServ_Packet{
	MEMFILE*	buf;
}MServ_Packet;

MServ_Packet*		MServ_Packet_New	();
unsigned int		MServ_Packet_Write	(void* buf,size_t size,size_t count,MServ_Packet* p);
/*
*	Format specification:
*	%c	-	one byte
*	%h	-	two bytes
*	%d	-	four bytes
*	%s	-	null-term string (not implemented yet)
*	%b	-	byte array; requires two args, the array, and the size
*/
unsigned int		MServ_Packet_Writefv(MServ_Packet* p,const char* format,...);
PRStatus			MServ_Packet_Send	(MServ_Packet* p,PRFileDesc* socket);

#endif