#include "mserv_packet.h"

#include <math.h>
#include <string.h>

unsigned char*	_MServ_Packet_MFResize(MEMFILE* mf,int numberOfTimes){
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

unsigned int	MServ_Packet_Write(void* buf,size_t size,size_t count,MServ_Packet* p){
	return mwrite(buf,size,count,p->buf);
}

unsigned int	MServ_Packet_Writefv(MServ_Packet* p,const char* format,...){
	va_list ap;
	int flen=strlen(format);
	int i=0;
	va_start(ap,format);
	for(i=0;i<flen;i++){
		if(format[i]=='%'){
			char type=format[++i];
			unsigned char* data=va_arg(ap,unsigned char*);
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
			MServ_Packet_Write(data,sizeof(unsigned char),numBytes,p);
		}
	}
	va_end(ap);
}

PRStatus	MServ_Packet_Send(MServ_Packet* p,PRFileDesc* socket){
	PRStatus st=PR_Write(socket,p->buf->buffer,p->buf->size);
	return st;
}
