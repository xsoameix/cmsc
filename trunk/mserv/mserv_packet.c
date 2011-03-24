#include "mserv_packet.h"

#include <math.h>
#include <string.h>

#include <prerr.h>
#include <prerror.h>

byte*	_MServ_Packet_MFResize(MEMFILE* mf,int numberOfTimes){
	if(mf->capacity==0)
		mf->capacity=1;
	mf->capacity*=pow(2.0f,numberOfTimes);
	mf->buffer=realloc(mf->buffer,mf->capacity);
	return mf->buffer;
}

MServ_Packet*	MServ_Packet_New(){
	MServ_Packet* mf=zalloc(sizeof(MServ_Packet));
	mf->buf=mnew();
	mf->buf->resizeFxn=_MServ_Packet_MFResize;
	return mf;
}
void				MServ_Packet_Free	(MServ_Packet* p){
	mclose(p->buf);
	free(p);
	return;
}

unsigned int	MServ_Packet_Write(void* buf,size_t size,size_t count,MServ_Packet* p){
	return mwrite(buf,size,count,p->buf);
}

unsigned int	MServ_Packet_Writefvp(MServ_Packet* p,const char* format,...){
	unsigned int ret=0;
	va_list ap;
	va_start(ap,format);
	ret=MServ_Packet_vWritefvp(p,format,ap);
	va_end(ap);
	return ret;
}

unsigned int	MServ_Packet_Writefvv(MServ_Packet* p,const char* format,...){
	unsigned int ret=0;
	va_list ap;
	va_start(ap,format);
	ret=MServ_Packet_vWritefvv(p,format,ap);
	va_end(ap);
	return ret;
}

unsigned int	MServ_Packet_vWritefvp(MServ_Packet* p,const char* format,va_list ap){
	int flen=strlen(format);
	int i=0;
	unsigned int ret=0;
	for(i=0;i<flen;i++){
		if(format[i]=='%'){
			char type=format[++i];
			byte* data=va_arg(ap,byte*);
			int numBytes=0;
			switch(type){
				case 'c':
					numBytes=1;
					break;
				case 'h':
					numBytes=2;
					break;
				case 'd':
					numBytes=4;
					break;
				case 'b':
					numBytes=va_arg(ap,int);
					break;
			}
			MServ_Packet_Write(data,sizeof(byte),numBytes,p);
			ret++;
		}
	}
	return ret;
}

unsigned int	MServ_Packet_vWritefvv(MServ_Packet* p,const char* format,va_list ap){
	int flen=strlen(format);
	int i=0;
	unsigned int ret=0;
	byte* tempB=zalloc(10);
	for(i=0;i<flen;i++){
		if(format[i]=='%'){
			char type=format[++i];
			byte* data=NULL;
			int numBytes=0;
			switch(type){
				case 'c':
					numBytes=1;
					(*tempB)=(va_arg(ap,int));
					break;
				case 'h':
					numBytes=2;
					(*tempB)=(va_arg(ap,int));
					break;
				case 'd':
					numBytes=4;
					(*tempB)=(va_arg(ap,int));
					break;
				case 'b':
					data=(va_arg(ap,byte*));
					numBytes=va_arg(ap,int);
					break;
			}
			if(data==NULL)
				data=tempB;
			MServ_Packet_Write(data,sizeof(byte),numBytes,p);
			memset(tempB,'\0',10);
			ret++;
		}
	}
	return ret;
}

PRStatus	MServ_Packet_Send(MServ_Packet* p,PRFileDesc* socket){
	PRStatus st=PR_Write(socket,p->buf->buffer,p->buf->size);
	return st;
}

int		MServ_Packet_Recv(MServ_Packet* p,PRFileDesc* socket){
	PRStatus st=PR_SUCCESS;
	PRErrorCode err=0;
	int ret=0;
	do{
		ret=PR_Recv(socket,p->buf->buffer,1024,0,PR_MillisecondsToInterval(10));
	}while(ret<=0 && (err=PR_GetError())==PR_WOULD_BLOCK_ERROR);
	p->buf->size+=ret;
	return ret;
}

void MServ_Packet_Reset(MServ_Packet* p){
	p->buf->writeIndex=0;
	p->buf->size=0;
}

unsigned int		MServ_Packet_GetLength	(MServ_Packet* p){
	byte* buf=zalloc(4);
	int ret=0;
	mread(buf,sizeof(byte),4,p->buf);
	ret=((buf[0]+buf[1]*0x100)^(buf[2]+buf[3]*0x100));
	free(buf);
	return ret;
}