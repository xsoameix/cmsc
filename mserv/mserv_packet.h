#ifndef __MSERV_PACKET_H
#define __MSERV_PACKET_H
#include "mserv_config.h"

#include <stdarg.h>

#include <prio.h>

#include "ewfutils.h"

#include "mserv_crypto.h"

typedef struct _struct_MServ_Packet{
	MEMFILE*		buf;
}MServ_Packet; //I guess this is more a packet sending/receiving class :-P

MServ_Packet*		MServ_Packet_New	();
void				MServ_Packet_Free	(MServ_Packet* p);
unsigned int		MServ_Packet_Write	(void* buf,size_t size,size_t count,MServ_Packet* p);
/*
*	Format specification:
*	%c	-	one byte
*	%h	-	two bytes
*	%d	-	four bytes
*	%s	-	null-term string (not implemented yet)
*	%b	-	byte array; requires two args, the array, and the size
*/
unsigned int		MServ_Packet_vWritefvv	(MServ_Packet* p,const char* format,va_list ap);
unsigned int		MServ_Packet_vWritefvp	(MServ_Packet* p,const char* format,va_list ap);
unsigned int		MServ_Packet_Writefvv	(MServ_Packet* p,const char* format,...);
unsigned int		MServ_Packet_Writefvp	(MServ_Packet* p,const char* format,...);
PRStatus			MServ_Packet_Send		(MServ_Packet* p,PRFileDesc* socket);
//blocks
int					MServ_Packet_Recv		(MServ_Packet* p,PRFileDesc* socket);
void				MServ_Packet_Reset		(MServ_Packet* p);
unsigned int		MServ_Packet_GetLength	(MServ_Packet* p);

/*---Packet Definitions---*/
#define				MSERV_PACKET_DEF_HELLO_FORMAT	"opcode:%h version:%h patchLocationStr:%h recvIV:%d sendIV:%d locale:%c"
#define				MSERV_PACKET_DEF_HELLO_OPCODE	0xD

#endif