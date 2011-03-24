#include "mserv_login.h"

#include <stdio.h>

#include <prthread.h>
#include <prnetdb.h>
#include <prerror.h>

#include "mserv_connection.h"
#include "mserv_packet.h"

void	MServ_Login_NewClient(void* sock);

void	MServ_Login_Start(){
	printf("Starting Login Server Thread.\n");
	PR_CreateThread(PR_USER_THREAD,MServ_Login_Listen,NULL,PR_PRIORITY_NORMAL,PR_GLOBAL_THREAD,PR_JOINABLE_THREAD,0);
}

void	MServ_Login_Listen(void* data){
	PRFileDesc* socket=PR_NewTCPSocket();
	PRNetAddr addr;
	{
		PRSocketOptionData opt;
		opt.option=PR_SockOpt_Nonblocking;
		opt.value.non_blocking=(1==1);
		PR_SetSocketOption(socket,&opt);
	}
	{
		PR_InitializeNetAddr(PR_IpAddrAny,8484,&addr);
	}
	PR_Bind(socket,&addr);
	PR_Listen(socket,10);
	while(1==1){
		PRFileDesc* newSock=PR_Accept(socket,NULL,10);
		if(newSock!=NULL){
			void* ret=NULL;
			printf("Connection request, spinning off thread.\n");
			ret=PR_CreateThread(PR_USER_THREAD,MServ_Login_NewClient,(void*)newSock,PR_PRIORITY_NORMAL,PR_GLOBAL_THREAD,PR_JOINABLE_THREAD,0);
			printf("Thread out %d\n",ret);
		}
		PR_Sleep(PR_MillisecondsToInterval(1));
	}
	PR_Close(socket);
}

void	MServ_Login_NewClient(void* sock){
	PRFileDesc* s=(PRFileDesc*)sock;
	MServ_Connection* con=MServ_Connection_New();
	MServ_Packet* p=MServ_Packet_New();
	PRNetAddr peerAddr;
	PR_GetPeerName(s,&peerAddr);
	con->socket=s;
	printf("New connection from ");
	{
		char buf[256]="";
		PR_NetAddrToString(&peerAddr,buf,256);
		printf(buf);
	}
	printf("\n");
	{
		MServ_Connection_Sendvv(con,MSERV_PACKET_DEF_HELLO_FORMAT,MSERV_PACKET_DEF_HELLO_OPCODE,40,0,con->crypto->recvIV,con->crypto->sendIV,5);
	}
	PR_Close(s);
}
