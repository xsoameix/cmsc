#include "mserv_connection.h"

#include <stdio.h>

MServ_Connection*		MServ_Connection_New	(){
	MServ_Connection* ret=zalloc(sizeof(MServ_Connection));
	ret->crypto=MServ_Crypto_New();
	ret->crypto->recvIV=4;
	ret->crypto->sendIV=5;
	ret->packet=MServ_Packet_New();
	return ret;
}
void					MServ_Connection_Free	(MServ_Connection* con){
	MServ_Crypto_Free(con->crypto);
	MServ_Packet_Free(con->packet);
	free(con);
}
unsigned int			MServ_Connection_Sendvv	(MServ_Connection* con,const char* format,...){
	unsigned int ret=0;
	va_list ap;
	va_start(ap,format);
	ret=MServ_Packet_vWritefvv(con->packet,format,ap);
	va_end(ap);
	MServ_Packet_Send(con->packet,con->socket);
	MServ_Packet_Reset(con->packet);
	MServ_Crypto_Shuffle(con->crypto,MSERV_CRYPTO_SHUFFLE_SEND);
	return ret;
}
unsigned int			MServ_Connection_Recv	(MServ_Connection* con){
	unsigned int ret=0;
	ret=MServ_Packet_Recv(con->packet,con->socket);
	MServ_Crypto_DecryptPacket(con->crypto,con->packet);
	MServ_Crypto_Shuffle(con->crypto,MSERV_CRYPTO_SHUFFLE_RECV);
	return ret;
}
