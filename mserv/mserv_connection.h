#ifndef __MSERV_CONNECTION_H
#define __MSERV_CONNECTION_H
#include "mserv_config.h"

#include <stdarg.h>

#include <prio.h>

#include "mserv_packet.h"
#include "mserv_crypto.h"

typedef struct _struct_MServ_Connection{
	MServ_Packet*		packet;
	MServ_Crypto*		crypto;
	PRFileDesc*			socket;
}MServ_Connection;

MServ_Connection*		MServ_Connection_New	();
void					MServ_Connection_Free	(MServ_Connection* con);
unsigned int			MServ_Connection_Sendvv	(MServ_Connection* con,const char* format,...);

#endif