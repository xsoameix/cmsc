#ifndef __MSERV_CRYPTO_H
#define __MSERV_CRYPTO_H
#include "mserv_config.h"

struct _struct_MServ_Packet;

typedef struct _struct_MServ_Crypto{
	unsigned int	sendIV;
	unsigned int	recvIV;
}MServ_Crypto;

MServ_Crypto*	MServ_Crypto_New		();
#define			MSERV_CRYPTO_SHUFFLE_RECV	0
#define			MSERV_CRYPTO_SHUFFLE_SEND	1
void			MServ_Crypto_Shuffle	(MServ_Crypto* c,int shuffleIndex);
void			MServ_Crypto_Free		(MServ_Crypto* c);
struct _struct_MServ_Packet*		MServ_Crypto_DecryptPacket	(MServ_Crypto* c,struct _struct_MServ_Packet* p);

#endif
