#include "mserv_login.h"

#include <stdio.h>

#include <prthread.h>
#include <prnetdb.h>
#include <prerror.h>

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
	if(PR_Bind(socket,&addr)!=PR_SUCCESS)
		printf("holy ballz no socket\n");
	PR_Listen(socket,10);
	while(1==1){
		PRFileDesc* newSock=PR_Accept(socket,NULL,10);
		if(newSock!=NULL){
			void* ret=NULL;
			printf("Connection request, spinning off thread.\n");
			ret=PR_CreateThread(PR_USER_THREAD,MServ_Login_NewClient,(void*)newSock,PR_PRIORITY_NORMAL,PR_GLOBAL_THREAD,PR_JOINABLE_THREAD,0);
			printf("Thread out %d\n",ret);
		}
	}
	PR_Close(socket);
}

void	MServ_Login_NewClient(void* sock){
	PRFileDesc* s=(PRFileDesc*)sock;
	PRNetAddr peerAddr;
	PR_GetPeerName(s,&peerAddr);
	printf("New connection from ");
	{
		char buf[256]="";
		PR_NetAddrToString(&peerAddr,buf,256);
		printf(buf);
	}
	printf("\n");
	{//send connect?
		MServ_Packet* p=MServ_Packet_New();
		short opcode=0xd;
		short version=40;
		unsigned int recvIV=1;
		unsigned int sendIV=2;
		unsigned char locale=5;
		unsigned short strLen=0;
		MServ_Packet_Writefv(p,"%h%h%h%d%d%c",&opcode,&version,&strLen,&recvIV,&sendIV,&locale);
		MServ_Packet_Send(p,s);
		/*
		Opcode - 0xD
		Version - 40
		Recv, Send - unused but send anyway
		locale - 5
		*/
	}
	PR_Close(s);
}
