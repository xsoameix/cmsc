#include "wzlibc.h"

#include <stdlib.h>
#include <string.h>

#include <stdio.h>
#include <stdarg.h>

#include "wzlibc_key.h"
#include "zlib.h"

#define WZPP(prop) WZLIB_PRIMITIVEPROPERTY(prop)

typedef struct _struct_LL_Entry{
	struct _struct_LL_Entry*		prev;
	struct _struct_LL_Entry*		next;
	void*		data;
}_LL_Entry;

_LL_Entry*		_LL_Entry_New			(void* data){
	_LL_Entry* ret=(_LL_Entry*)malloc(sizeof(_LL_Entry));
	memset(ret,0,sizeof(_LL_Entry));
	ret->data=data;
	return ret;
}
int			_LL_Entry_GetSize		(_LL_Entry* head){
	_LL_Entry* cur=head;
	int l=0;
	if(cur==NULL)
		return 0;
	while(cur!=NULL){
		l++;
		cur=cur->next;
	}
	return l;
}

/* attaches attach to LL that starts at head */
/* returns head for easy coding later lulz*/
_LL_Entry*		_LL_Entry_Attach		(_LL_Entry* head,_LL_Entry* attach){
	_LL_Entry* cur;
	if(head==NULL)
		head=attach; /* then attach is the first*/
	/*find end of list*/
	cur=head;
	while(cur->next!=NULL){
		cur=cur->next;
	}
	if(cur!=attach){
		cur->next=attach;
		attach->prev=cur;
	}
	return head;
}

/*Functions priv to this file*/
WZLib_Object*		_WZLib_Object_New		(const char* name,WZLib_ObjectType type);
WZLib_Directory* 	_WZLib_Directory_New		(const char* name);
void			_WZLib_Directory_Delete		(WZLib_Directory* dir);
ErrorCode		_WZLib_Directory_Parse		(WZLib_Directory* dir,FILE* stream,WZLib_Header* header);
WZLib_Image*		_WZLib_Image_New		(const char* name);
WZLib_Header*		_WZLib_Header_New		(const char* name);
ErrorCode		_WZLib_Header_Parse		(WZLib_Header* header,FILE* stream);
WZLib_Property* 	_WZLib_Property_New		(const char* name,WZLib_PropertyType propType);
WZLib_SubProperty*	_WZLib_SubProperty_New		(const char* name);
ErrorCode		_WZLib_SubProperty_Parse	(WZLib_SubProperty* sub,FILE* stream,unsigned int offset);
_LL_Entry* 		_WZLib_SubProperty_Add		(WZLib_SubProperty* sub,WZLib_Property* prop);
void			_WZLib_SubProperty_Delete	(WZLib_SubProperty* sub);
WZLib_PrimitiveProperty* _WZLib_PrimitiveProperty_New	(const char* name,WZLib_PropertyType propType);
WZLib_CanvasProperty*	_WZLib_CanvasProperty_New	(const char* name);
WZLib_PNGProperty*	_WZLib_PNGProperty_New		(const char* name);
ErrorCode		_WZLib_PNGProperty_Read		(WZLib_PNGProperty* png,FILE* stream);
WZLib_SoundProperty* 	_WZLib_SoundProperty_New	(const char* name);
void 			_WZLib_SoundProperty_Parse	(WZLib_SoundProperty* prop,FILE* stream);
void 			_WZLib_SoundProperty_Delete	(WZLib_SoundProperty* prop);
WZLib_Property*		_WZLib_ExtendedProperty_Parse	(FILE* stream,const char* name,int offset,int eob);
unsigned int		_WZLib_File_GetVersionHash	(int encver,int realver);
/*---------------------------*/

void _tokenize(const char* str,const char* delim,char*** outTokens,int* numTokens){
	const int guessSize=2;
	char** tokens=(char**)malloc(sizeof(char*)*guessSize);
	int index=0;
	int tokenStart=0;
	int tokenIndex=0;
	int tokensCap=guessSize;
	int len=strlen(str);
	int delimLen=strlen(delim);
	for(index=0;index<=len;index++){
		if(strncmp(str+sizeof(char)*index,delim,delimLen)==0){ /*match using strcmp*/
			int strlen=0;
			if(index==tokenStart){
				tokenStart++; /*in case it's two delims after the other*/
				continue;
			}
			/*do we need to expand tokens?*/
			if(tokenIndex+1>=tokensCap){
				tokens=(char**)realloc(tokens,sizeof(char*)*(tokensCap+guessSize));
				tokensCap+=guessSize;
			}
			strlen=index-tokenStart;
			tokens[tokenIndex]=(char*)malloc(sizeof(char*)*(strlen+1));
			memcpy(tokens[tokenIndex],str+(sizeof(char)*tokenStart),strlen+1);
			tokens[tokenIndex][strlen]=0; /*null terminate pl0x*/
			tokenIndex++;
			tokenStart=index+1;
		}
	}
	if(str[tokenStart]!=0){
		int strlen=0;
		if(tokenIndex+1>=tokensCap){
			tokens=(char**)realloc(tokens,sizeof(char*)*(tokensCap+guessSize));
			tokensCap+=guessSize;
		}
		strlen=index-tokenStart;
		tokens[tokenIndex]=(char*)malloc(sizeof(char*)*(strlen+1));
		memcpy(tokens[tokenIndex],str+(sizeof(char)*tokenStart),strlen+1);
		tokens[tokenIndex][strlen]=0; /*null terminate pl0x*/
		tokenIndex++;
		tokenStart=index+1;
	}
	if(tokenIndex<tokensCap)
		tokens=(char**)realloc(tokens,sizeof(char*)*tokenIndex);
	*outTokens=tokens;
	*numTokens=tokenIndex;
}
int _z_decompress(unsigned char* inBuffer,int inLen,unsigned char** outBufferPtr,int* outLen){
	const int inChunk=1024;
	const int outChunk=1024;
	const int guessSize=500000; /*allocate in chunks of this size*/
	unsigned char* outData=(unsigned char*)malloc(sizeof(unsigned char)*outChunk);
	unsigned char* outBuffer=(unsigned char*)malloc(sizeof(unsigned char)*guessSize);
	int outBufferIndex=0;
	int outBufferSize=guessSize;
	int inBufferIndex=0;
	int outDataLen=0;
	int err=Z_OK;
	z_stream strm;
	{
		strm.next_in=NULL;
		strm.avail_in=0;
		strm.opaque=NULL;
		strm.zfree=Z_NULL;
		strm.zalloc=Z_NULL;
		inflateInit(&strm);
		
		do{
			strm.next_in=inBuffer+sizeof(unsigned char)*inBufferIndex;
			strm.avail_in=(inLen-inBufferIndex > inChunk)?inChunk:inLen-inBufferIndex;
			if(strm.avail_in==0)
				break;
			inBufferIndex+=strm.avail_in;
			do{
				int have;
				if(strm.avail_in==0)
					break;
				strm.avail_out=outChunk;
				strm.next_out=outData;
				err=inflate(&strm,Z_NO_FLUSH);
				switch(err){
					case Z_NEED_DICT:
					case Z_DATA_ERROR:
					case Z_MEM_ERROR:
					case Z_BUF_ERROR:
						free(outData);
						inflateEnd(&strm);
						return err;
				}
				have=outChunk-strm.avail_out;
				if(outBufferIndex+have > outBufferSize){ /*need to make it bigger pl0x*/
					outBuffer=(unsigned char*)realloc(outBuffer,sizeof(unsigned char*)*(outBufferSize+guessSize));
					outBufferSize+=guessSize;
				}
				memcpy((void*)(outBuffer+(sizeof(unsigned char)*outBufferIndex)),(void*)outData,have);
				outBufferIndex+=have;
			}while(strm.avail_out==0);
		}while(err!=Z_STREAM_END);
		
		inflateEnd(&strm);
	}
	*outLen=strm.total_out;
	*outBufferPtr=outBuffer;
	if(strm.total_out<outBufferSize){ /*make smaller*/
		outBuffer=(unsigned char*)realloc(outBuffer,sizeof(unsigned char)*strm.total_out);
	}
	free(outData);
	return Z_OK;
}

/*File Reading Functions*/
#define F_Read_Primitive(stream,type,val)	fread(&val,sizeof(type),1,stream)
char* F_Read_NullString(FILE* stream){
	int guessSize=30;
	int actual=0;
	char* ret=(char*)malloc(sizeof(char)*guessSize);
	memset(ret,0,sizeof(char)*guessSize);
	do{
		char c;
		F_Read_Primitive(stream,char,c);
		if(actual>=guessSize){
			ret=(char*)realloc(ret,sizeof(char)*(actual+guessSize));
			guessSize+=actual;
		}
		ret[actual]=c;
		if(c==0)
			break;
		actual++;
	}while(1==1);
	if(actual<guessSize)
		ret=(char*)realloc(ret,sizeof(char)*(actual+1));
	return ret;
}
char* F_Read_Chars(FILE* stream,int count){
	char* ret=(char*)malloc(sizeof(char)*count);
	int i=0;
	for(i=0;i<count;i++){
		char c;
		F_Read_Primitive(stream,char,c);
		ret[i]=c;
	}
	return ret;
}
char* F_Read_EncryptedString(FILE* stream){
	char smallLength=0;
	char* ret=NULL;
	F_Read_Primitive(stream,signed char,smallLength);
	if(smallLength==0)
		return "";
	if(smallLength>0){
		unsigned short mask=0xAAAA;
		unsigned int length=0;
		unsigned int i=0;
		if(smallLength==0x7F)
			F_Read_Primitive(stream,signed int,length);
		else
			length=smallLength;
		if(length<=0)
			return "";
		ret=(char*)malloc(sizeof(char)*length+1); /*1 extra for null//*/
		for(i=0;i<length;i++){
			unsigned int encChar;
			F_Read_Primitive(stream,unsigned short,encChar);
			encChar^=mask;
			encChar^=(_WZLib_WZKey[i*2+1]<<8)+_WZLib_WZKey[i*2];
			mask++;
			ret[i]=(char)encChar;
		}
		ret[length]=0;
	}else{
		unsigned int mask=0xAA;
		unsigned int length=0;
		unsigned int i=0;
		if(smallLength==-128)
			F_Read_Primitive(stream,unsigned int,length);
		else
			length=-smallLength;
		if(length<=0)
			return "";
		ret=(char*)malloc(sizeof(char)*length+1);
		for(i=0;i<length;i++){
			unsigned char encChar;
			F_Read_Primitive(stream,unsigned char,encChar);
			encChar^=mask;
			encChar^=_WZLib_WZKey[i];
			ret[i]=encChar;
			mask++;
		}
		ret[length]=0;
	}
	return ret;
}
int F_Read_CompressedInt(FILE* stream){
	signed char chk;
	int val;
	F_Read_Primitive(stream,signed char,chk);
	if(chk!=-128)
		return (int)chk;
	F_Read_Primitive(stream,int,val);
	return val;
}
unsigned int F_Read_Offset(FILE* stream,WZLib_Header* header){ /*I'll need a header for this*/
	unsigned int offset=ftell(stream);
	unsigned int xor;
	offset=(offset-header->fileStart)^0xFFFFFFFF;
	offset*=header->versionHash;
	offset-=0x581C3F6D;
	offset=(offset<<(offset&0x1f))|(offset>>(32-offset&0x1F));
	F_Read_Primitive(stream,unsigned int,xor);
	offset^=xor;
	offset+=header->fileStart*2;
	return offset;
}
char* F_Read_StringBlock(FILE* stream,long offset){
	char* ret;
	unsigned char b;
	long currentOffset=0;
	int offinc;
	F_Read_Primitive(stream,unsigned char,b);
	switch(b){
		case 0:
		case 0x73:
			ret=F_Read_EncryptedString(stream);
			break;
		case 1:
		case 0x1B:
			F_Read_Primitive(stream,int,offinc);
			offset+=offinc;
			currentOffset=ftell(stream);
			fseek(stream,offset,SEEK_SET);
			ret=F_Read_EncryptedString(stream);
			fseek(stream,currentOffset,SEEK_SET);
			break;
		default:
			ret="";
			break;
	}
	return ret;
}
char* F_Read_StringAtOffset(FILE* stream,unsigned int offset){
	long cur=ftell(stream);
	char* ret=NULL;
	fseek(stream,offset,SEEK_SET);
	ret=F_Read_EncryptedString(stream);
	fseek(stream,cur,SEEK_SET);
	return ret;
}
/*----------------------*/

/*Key Fxns*/
/*#include "aes.h"
ErrorCode _WZLib_Key_Generate(){
	_WZLib_Key->keyData=NULL;
	_WZLib_Key->keyLength=0;
}//*/
/*--------*/
char*		WZLib_PropertyType_GetName(WZLib_PropertyType propType){
	switch(propType){
		case WZLib_Prop_Null:
			return "WZNullProperty";
		case WZLib_Prop_UnsignedShort:
			return "WZUnsignedShortProperty";
		case WZLib_Prop_CompressedInt:
			return "WZCompressedIntProperty";
		case WZLib_Prop_Vector:
			return "WZVectorProperty";
		case WZLib_Prop_Float:
			return "WZFloatProperty";
		case WZLib_Prop_Double:
			return "WZDoubleProperty";
		case WZLib_Prop_String:
			return "WZStringProperty";
		case WZLib_Prop_UOL:
			return "WZUOLProperty";
		case WZLib_Prop_Sub:
			return "WZSubProperty";
		case WZLib_Prop_Canvas:
			return "WZCanvasProperty";
		case WZLib_Prop_Convex:
			return "WZConvexProperty";
		case WZLib_Prop_PNG:
			return "WZPNGProperty";
		case WZLib_Prop_MP3:
			return "WZMP3Property";
	}
	return "WZUnknownProperty";
}
char*		WZLib_ObjectType_GetName(WZLib_ObjectType type){
	switch(type){
		case WZLib_ObjectType_File:
			return "WZFile";
		case WZLib_ObjectType_Directory:
			return "WZDirectory";
		case WZLib_ObjectType_Image:
			return "WZImage";
		case WZLib_ObjectType_Property:
			return "WZProperty";
		case WZLib_ObjectType_Header:
			return "WZFileHeader";
	}
	return "Unknown";
}

WZLib_Object*	_WZLib_Object_New(const char* name,WZLib_ObjectType type){
	WZLib_Object* ret=(WZLib_Object*)malloc(sizeof(WZLib_Object));
	memset(ret,0,sizeof(WZLib_Object));
	ret->name=(char*)malloc(sizeof(char)*(strlen(name)+1));
	memcpy(ret->name,name,strlen(name)+1);
	ret->type=type;
	ret->parsed=0;
	return ret;
}

WZLib_Object*	WZLib_Object_Get_n(WZLib_Object* obj,const char* name){
	if(obj->type==WZLib_ObjectType_Image)
		return WZLib_Image_Get_n((WZLib_Image*)obj,name);
	if(obj->type==WZLib_ObjectType_Directory)
		return WZLib_Directory_Get_n((WZLib_Directory*)obj,name);
	if(obj->type==WZLib_ObjectType_Property)
		if(WZLIB_SPROPTYPE(obj)==WZLib_Prop_Sub)
			return WZLib_SubProperty_Get_n((WZLib_SubProperty*)obj,name);
	return NULL;
}
WZLib_Object*	WZLib_Object_Get_i(WZLib_Object* obj,int index){
	if(obj->type==WZLib_ObjectType_Image)
		return WZLib_Image_Get_i((WZLib_Image*)obj,index);
	if(obj->type==WZLib_ObjectType_Directory)
		return WZLib_Directory_Get_i((WZLib_Directory*)obj,index);
	if(obj->type==WZLib_ObjectType_Property)
		if(WZLIB_SPROPTYPE(obj)==WZLib_Prop_Sub)
			return WZLib_SubProperty_Get_i((WZLib_SubProperty*)obj,index);
	return NULL;
}
WZLib_Object*	WZLib_Object_Get_iv(WZLib_Object* obj, ...){
	va_list ap;
	WZLib_Object* currentObj=obj;
	int ci;
	va_start(ap,obj);
	ci=va_arg(ap,int);
	while(ci!=-1){
		currentObj=WZLib_Object_Get_i(currentObj,ci);
		if(currentObj==NULL)
			break;
		ci=va_arg(ap,int);
	}
	va_end(ap);
	return currentObj;
}
WZLib_Object*	WZLib_Object_Get_nv(WZLib_Object* obj, ...){
	va_list ap;
	WZLib_Object* currentObj=obj;
	char* ci;
	va_start(ap,obj);
	ci=va_arg(ap,char*);
	while(ci!=NULL && strcmp(ci,"")!=0){
		currentObj=WZLib_Object_Get_n(currentObj,ci);
		if(currentObj==NULL)
			break;
		ci=va_arg(ap,char*);
	}
	va_end(ap);
	return currentObj;
}
int		WZLib_Object_GetSize(WZLib_Object* obj){
	if(obj->type==WZLib_ObjectType_Image)
		return WZLib_Image_GetSize((WZLib_Image*)obj);
	if(obj->type==WZLib_ObjectType_Directory)
		return WZLib_Directory_GetSize((WZLib_Directory*)obj);
	if(obj->type==WZLib_ObjectType_Property)
		if(WZLIB_SPROPTYPE(obj)==WZLib_Prop_Sub)
			return WZLib_SubProperty_GetSize((WZLib_SubProperty*)obj);
	return -1;
}

WZLib_Image* _WZLib_Image_New(const char* name){
	WZLib_Image* ret=(WZLib_Image*)malloc(sizeof(WZLib_Image));
	memset(ret,0,sizeof(WZLib_Image));
	ret->inh=*_WZLib_Object_New(name,WZLib_ObjectType_Image);
	ret->_sub=_WZLib_SubProperty_New(name);
	((WZLib_Property*)ret->_sub)->parentImg=ret;
	WZLIB_OBJECT(ret->_sub)->parent=ret;
	return ret;
}

WZLib_Property* _WZLib_Property_New(const char* name,WZLib_PropertyType propType){
	WZLib_Property* ret=(WZLib_Property*)malloc(sizeof(WZLib_Property));
	memset(ret,0,sizeof(WZLib_Property));
	ret->inh=*_WZLib_Object_New(name,WZLib_ObjectType_Property);
	ret->propType=propType;
	return ret;
}

WZLib_PrimitiveProperty* _WZLib_PrimitiveProperty_New(const char* name,WZLib_PropertyType propType){
	WZLib_PrimitiveProperty* ret=(WZLib_PrimitiveProperty*)malloc(sizeof(WZLib_PrimitiveProperty));
	memset(ret,0,sizeof(WZLib_PrimitiveProperty));
	ret->inh=*_WZLib_Property_New(name,propType);
	return ret;
}

WZLib_PNGProperty* _WZLib_PNGProperty_New(const char* name){
	WZLib_PNGProperty* ret=(WZLib_PNGProperty*)malloc(sizeof(WZLib_PNGProperty));
	memset(ret,0,sizeof(WZLib_PNGProperty));
	ret->inh=*_WZLib_Property_New(name,WZLib_Prop_PNG);
	return ret;
}

void WZLib_PNGProperty_Unparse(WZLib_PNGProperty* png){
	if(png==NULL)
		return;
	if(!WZLIB_OBJECT(png)->parsed)
		return;
#	ifdef WZLIB_HAVE_SDL
	SDL_FreeSurface(png->png);
#	endif
	free(png->_compBytes);
	free(png->_decBytes);
	WZLIB_OBJECT(png)->parsed=0;
	png->pngParsed=0;
}

void WZLib_PNGProperty_DisplayFormatAlpha(WZLib_PNGProperty* pngProp){
	SDL_Surface* vSurface=SDL_GetVideoSurface();
	SDL_Surface* opng=pngProp->png;
	if(vSurface==NULL)
		return;
	pngProp->png=SDL_DisplayFormatAlpha(pngProp->png);
	SDL_FreeSurface(opng);
}

ErrorCode WZLib_PNGProperty_Parse(WZLib_PNGProperty* png){
	unsigned char* decBuf=NULL;
	int decLen=0;
	if(png->_compBytes[0]==0x78 && png->_compBytes[1]==0x9c)
		_z_decompress(png->_compBytes,png->_length,&decBuf,&decLen);
	else{
		int blocksize=0;
		int eop=png->_length;
		int loc=0;
		unsigned char* tempCB=(unsigned char*)malloc(sizeof(unsigned char)*png->_length);
		int tempCBLoc=0;
		memset(tempCB,0,sizeof(unsigned char)*png->_length);
		while(loc<(eop-1)){
			int i=0;
			blocksize=0;
			blocksize+=(png->_compBytes[loc]  <<  0);
			blocksize+=(png->_compBytes[loc+1]<<  8);
			blocksize+=(png->_compBytes[loc+2]<< 16);
			blocksize+=(png->_compBytes[loc+3]<< 24);
			loc+=4;
			for(i=0;i<blocksize;i++){
				tempCB[tempCBLoc]=png->_compBytes[loc]^_WZLib_WZKey[i];
				loc++;
				tempCBLoc++;
			}
		}
		_z_decompress(tempCB,tempCBLoc++,&decBuf,&decLen);
		free(tempCB);
	}
#	ifdef WZLIB_HAVE_SDL
	{
		int f=png->_format+png->_format2;
		switch(f){
			case 1:
				{
					int x,y,r,g,b,a,k;
					png->png=SDL_CreateRGBSurface(SDL_HWSURFACE|SDL_SRCALPHA,png->width,png->height,32,0,0,0,SDL_ALPHA_TRANSPARENT);
					WZLib_PNGProperty_DisplayFormatAlpha(png);
					if(SDL_MUSTLOCK(png->png))
						SDL_LockSurface(png->png);
					for(k=0,x=0,y=0;k<decLen;k+=2){
						if(x==png->width){
							x=0;
							y++;
							if(y==png->height)
								break;
						}
						b=decBuf[k] & 15;
						b|=b<<4;
						g=decBuf[k] & 240;
						g|=g>>4;
						r=decBuf[k+1] & 15;
						r|=r<<4;
						a=decBuf[k+1] & 240;
						a|=a>>4;
						*((Uint32*)png->png->pixels+y*png->png->pitch/4+x)=SDL_MapRGBA(png->png->format,r,g,b,a);
						x++;
					}
					if(SDL_MUSTLOCK(png->png))
						SDL_UnlockSurface(png->png);
				}
				break;
			case 2:
				{
					png->png=SDL_CreateRGBSurface(SDL_HWSURFACE|SDL_SRCALPHA,png->width,png->height,32,0,0,0,SDL_ALPHA_TRANSPARENT);
					WZLib_PNGProperty_DisplayFormatAlpha(png);
					if(SDL_MUSTLOCK(png->png))
						SDL_LockSurface(png->png);
					memcpy(png->png->pixels,decBuf,decLen);
					if(SDL_MUSTLOCK(png->png))
						SDL_UnlockSurface(png->png);
				}
				break;
			case 513:
				{
					png->png=SDL_CreateRGBSurface(SDL_HWSURFACE|SDL_SRCALPHA,png->width,png->height,16,0,0,0,SDL_ALPHA_TRANSPARENT);
					if(SDL_MUSTLOCK(png->png))
						SDL_LockSurface(png->png);
					memcpy(png->png->pixels,decBuf,decLen);
					if(SDL_MUSTLOCK(png->png))
						SDL_UnlockSurface(png->png);
					WZLib_PNGProperty_DisplayFormatAlpha(png);
				}
				break;
			case 517:
				{/*//Map.wz -> Back -> midForest -> back -> 0.png*/
					png->png=SDL_CreateRGBSurface(SDL_HWSURFACE|SDL_SRCALPHA,png->width,png->height,32,0,0,0,SDL_ALPHA_TRANSPARENT);
					if(SDL_MUSTLOCK(png->png))
						SDL_LockSurface(png->png);
					{
						int x=0;int y=0;int i=0;unsigned char j=0;int k=0;
						for(i=0;i<decLen;i++){
							for(j=0;j<8;j++){
								unsigned char iB=((decBuf[i] & (0x01 << (7 - j))) >> (7 - j)) * 0xFF;
								for(k=0;k<16;k++){
									if(x==png->width){
										x=0;y++;
									}
									*((Uint32*)png->png->pixels+y*png->png->pitch/4+x)=SDL_MapRGBA(png->png->format,iB,iB,iB,SDL_ALPHA_OPAQUE);
									x++;
								}
							}
						}
					}
					if(SDL_MUSTLOCK(png->png))
						SDL_UnlockSurface(png->png);
					WZLib_PNGProperty_DisplayFormatAlpha(png);
				}
				break;
			default:
				printf("unrecognized png format %d\n",f);
				break;
		}
	}
#	endif
	WZLIB_OBJECT(png)->parsed=1;
	png->pngParsed=1;
	png->_decBytes=decBuf;
	png->_decLen=decLen;
	return WZLib_Error_NoError;
}

ErrorCode _WZLib_PNGProperty_Read(WZLib_PNGProperty* png,FILE* stream){
	png->_stream=stream;
	WZLIB_OBJECT(png)->offset=-1;
	if(stream!=NULL){
		int length=0;
		png->width=F_Read_CompressedInt(stream);
		png->height=F_Read_CompressedInt(stream);
		png->_format=F_Read_CompressedInt(stream);
		F_Read_Primitive(stream,unsigned char,png->_format2);
		fseek(stream,4,SEEK_CUR);
		F_Read_Primitive(stream,int,length);
		png->_length=length;
		fseek(stream,1,SEEK_CUR);
		if(length>0)
			png->_compBytes=(unsigned char*)malloc(sizeof(unsigned char)*length);
		WZLIB_OBJECT(png)->offset=ftell(stream);
		if(length>0)
			fread(png->_compBytes,sizeof(unsigned char),length,stream);
		WZLIB_OBJECT(png)->parsed=1;
	}
	return WZLib_Error_NoError;
}

WZLib_CanvasProperty* _WZLib_CanvasProperty_New(const char* name){
	WZLib_CanvasProperty* ret=(WZLib_CanvasProperty*)malloc(sizeof(WZLib_CanvasProperty));
	memset(ret,0,sizeof(WZLib_CanvasProperty));
	ret->inh=*_WZLib_SubProperty_New(name);
	ret->inh.inh.propType=WZLib_Prop_Canvas;
	return ret;
}

WZLib_SubProperty* _WZLib_SubProperty_New(const char* name){
	WZLib_SubProperty* ret=(WZLib_SubProperty*)malloc(sizeof(WZLib_SubProperty));
	memset(ret,0,sizeof(WZLib_SubProperty));
	ret->inh=*_WZLib_Property_New(name,WZLib_Prop_Sub);
	return ret;
}

WZLib_Property* _WZLib_ExtendedProperty_Parse(FILE* stream,const char* name,int offset,int eob){
	unsigned char b=0;
	char* iname;
	WZLib_Property* ret=NULL;
	F_Read_Primitive(stream,unsigned char,b);
	if(b==0x1B){
		int inc;
		int readPos=offset;
		long currentPos=0;
		F_Read_Primitive(stream,int,inc);
		readPos+=inc;
		currentPos=ftell(stream);
		fseek(stream,readPos,SEEK_SET);
		iname=F_Read_EncryptedString(stream);
		fseek(stream,currentPos,SEEK_SET);
	}else
		iname=F_Read_EncryptedString(stream);
	if(strcmp(iname,"Property")==0){
		ret=_WZLib_SubProperty_New(name);
		fseek(stream,2,SEEK_CUR);
		_WZLib_SubProperty_Parse((WZLib_SubProperty*)ret,stream,offset);
	}
	if(strcmp(iname,"Canvas")==0){
		unsigned char b;
		ret=_WZLib_CanvasProperty_New(name);
		fseek(stream,1,SEEK_CUR);
		F_Read_Primitive(stream,unsigned char,b);
		if(b==1){
			fseek(stream,2,SEEK_CUR);
			_WZLib_SubProperty_Parse((WZLib_SubProperty*)ret,stream,offset);
		}
		((WZLib_CanvasProperty*)ret)->png=_WZLib_PNGProperty_New(name);
		_WZLib_PNGProperty_Read(((WZLib_CanvasProperty*)ret)->png,stream);
	}
	if(strcmp(iname,"Shape2D#Vector2D")==0){
		ret=_WZLib_PrimitiveProperty_New(name,WZLib_Prop_Vector);
		WZPP(ret)->val.vecVal[0]=F_Read_CompressedInt(stream);
		WZPP(ret)->val.vecVal[1]=F_Read_CompressedInt(stream);
	}
	if(strcmp(iname,"Shape2D#Convex2D")==0){
		int ec=F_Read_CompressedInt(stream);
		int i=0;
		ret=_WZLib_SubProperty_New(name);
		for(i=0;i<ec;i++){
			WZLib_Property* nP=_WZLib_ExtendedProperty_Parse(stream,name,offset,eob);
			_WZLib_SubProperty_Add((WZLib_SubProperty*)ret,nP);
			WZLIB_OBJECT(nP)->parent=ret;
		}
	}
	if(strcmp(iname,"Sound_DX8")==0){
		ret=_WZLib_SoundProperty_New(name);
		_WZLib_SoundProperty_Parse((WZLib_SoundProperty*)ret,stream);
	}
	if(strcmp(iname,"UOL")==0){
		unsigned char b;
		unsigned int offInc;
		ret=_WZLib_PrimitiveProperty_New(name,WZLib_Prop_UOL);
		fseek(stream,1,SEEK_CUR);
		F_Read_Primitive(stream,unsigned char,b);
		switch(b){
			case 0:
				WZPP(ret)->val.strVal=F_Read_EncryptedString(stream);
				break;
			case 1:
				F_Read_Primitive(stream,unsigned int,offInc);
				WZPP(ret)->val.strVal=F_Read_StringAtOffset(stream,offset+offInc);
				break;
		}
	}
	return ret;
}

WZLib_Property* WZLib_SubProperty_Get_n(WZLib_SubProperty* sub,const char* name){
	_LL_Entry* cur=sub->_firstChild;
	if(cur==NULL)
		return NULL;
	do{
		if(strcmp(WZLIB_OBJECT(cur->data)->name,name)==0)
			return (WZLib_Property*)cur->data;
		cur=cur->next;
	}while(cur!=NULL);
	return NULL;
}
WZLib_Property* WZLib_SubProperty_Get_i(WZLib_SubProperty* sub,int index){
	_LL_Entry* cur=sub->_firstChild;
	int curIndex=0;
	if(cur==NULL)
		return NULL;
	do{
		if(curIndex==index)
			return (WZLib_Property*)cur->data;
		curIndex++;
		cur=cur->next;
	}while(cur!=NULL);
	return NULL;
}
int WZLib_SubProperty_GetSize(WZLib_SubProperty* sub){
	return _LL_Entry_GetSize(sub->_firstChild);
}

_LL_Entry* _WZLib_SubProperty_Add(WZLib_SubProperty* sub,WZLib_Property* prop){
	_LL_Entry* newE=_LL_Entry_New((void*)prop);
	sub->_firstChild=_LL_Entry_Attach(sub->_firstChild,newE);
	return newE;
}

void _WZLib_SubProperty_Delete(WZLib_SubProperty* sub){
	_LL_Entry* cur=sub->_firstChild;
	while(cur!=NULL){
		WZLib_Property* prop=WZLIB_PROPERTY(cur->data);
		if(prop->propType==WZLib_Prop_Canvas)
			WZLib_PNGProperty_Unparse(WZLIB_CANVASPROPERTY(prop)->png);
		if(WZLIB_SPROPTYPE(prop)==WZLib_Prop_Sub)
			_WZLib_SubProperty_Delete(WZLIB_SUBPROPERTY(prop));
		if(prop->propType==WZLib_Prop_MP3)
			_WZLib_SoundProperty_Delete((WZLib_SoundProperty*)prop);
		/* now just kill it */
		free(cur->prev);
		cur=cur->next;
		//free(prop);
	}
}

ErrorCode _WZLib_SubProperty_Parse(WZLib_SubProperty* sub,FILE* stream,unsigned int offset){
	ErrorCode ret=WZLib_Error_NoError;
	int entryCount=F_Read_CompressedInt(stream);
	int i=0;
	for(i=0;i<entryCount;i++){
		char* name;
		unsigned char b;
		WZLib_Property* prop=NULL;
		name=F_Read_StringBlock(stream,offset);
		F_Read_Primitive(stream,unsigned char,b);
		switch(b){
			case 0x00:
				{
					prop=_WZLib_PrimitiveProperty_New(name,WZLib_Prop_Null);
				}
				break;
			case 0x0B:
			case 0x02:
				{
					prop=_WZLib_PrimitiveProperty_New(name,WZLib_Prop_UnsignedShort);
					F_Read_Primitive(stream,unsigned int,WZPP(prop)->val.intVal);
				}
				break;
			case 0x03:
				{
					prop=_WZLib_PrimitiveProperty_New(name,WZLib_Prop_CompressedInt);
					WZPP(prop)->val.intVal=F_Read_CompressedInt(stream);
				}
				break;
			case 0x04:
				{
					unsigned char type=0;
					F_Read_Primitive(stream,unsigned char,type);
					prop=_WZLib_PrimitiveProperty_New(name,WZLib_Prop_Float);
					if(type==0x80)
						F_Read_Primitive(stream,float,WZPP(prop)->val.dblVal);
					else
						WZPP(prop)->val.dblVal=0.0;
				}
				break;
			case 0x05:
				{
					prop=_WZLib_PrimitiveProperty_New(name,WZLib_Prop_Double);
					F_Read_Primitive(stream,double,WZPP(prop)->val.dblVal);
				}
				break;
			case 0x08:
				{
					prop=_WZLib_PrimitiveProperty_New(name,WZLib_Prop_String);
					WZPP(prop)->val.strVal=F_Read_StringBlock(stream,offset);
				}
				break;
			case 0x09:
				{
					unsigned int temp=0;
					int eob=0;
					F_Read_Primitive(stream,unsigned int,temp);
					eob=(int)(temp+ftell(stream));
					prop=_WZLib_ExtendedProperty_Parse(stream,name,offset,eob);
					if(ftell(stream)!=eob)
						fseek(stream,eob,SEEK_SET);
				}
				break;
		}
		prop->parentImg=((WZLib_Property*)sub)->parentImg;
		WZLIB_OBJECT(prop)->parent=sub;
		_WZLib_SubProperty_Add(sub,prop);
		free(name);
	}
	return ret;
}

void WZLib_File_Close(WZLib_File* file){
	_WZLib_Directory_Delete(file->_wzDir);
	free(file->_wzDir);
	fclose(file->_stream);
	free(file->_header);
	free(file);
}

WZLib_File*	WZLib_File_Open(const char* name){
	WZLib_File* ret=(WZLib_File*)malloc(sizeof(WZLib_File));
	FILE* testf=NULL;
	memset(ret,0,sizeof(WZLib_File));
	ret->_fileVersion=-1;
	ret->inh=*_WZLib_Object_New(name,WZLib_ObjectType_File);
	ret->_fileName=(char*)malloc(sizeof(char)*(strlen(name)+1));
	memcpy(ret->_fileName,name,strlen(name)+1);
	/*test to see if name is valid*/
	testf=fopen(name,"rb");
	if(testf==NULL){
		free(ret);
		return NULL;
	}
	fclose(testf);
	ret->_header=_WZLib_Header_New(name);
	ret->_header->inh.parent=ret;
	return ret;
}

WZLib_Directory* _WZLib_Directory_New(const char* name){
	WZLib_Directory* ret=(WZLib_Directory*)malloc(sizeof(WZLib_Directory));
	memset(ret,0,sizeof(WZLib_Directory));
	ret->inh=*_WZLib_Object_New(name,WZLib_ObjectType_Directory);
	return ret;
}

void _WZLib_Directory_Delete(WZLib_Directory* dir){
	_LL_Entry* cur=dir->_firstChild;
	while(cur!=NULL){
		WZLib_Object* obj=WZLIB_OBJECT(cur->data);
		if(obj->type==WZLib_ObjectType_Directory)
			_WZLib_Directory_Delete((WZLib_Directory*)cur->data);
		else
			WZLib_Image_Unparse((WZLib_Image*)cur->data);
		/* and kill it */
		free(obj);
		free(cur->prev);
		cur=cur->next;
	}
}

ErrorCode		_WZLib_Directory_Parse(WZLib_Directory* dir,FILE* stream,WZLib_Header* header){
	int count=F_Read_CompressedInt(stream);
	int i=0;
	for(i=0;i<count;i++){
		unsigned char type;
		char* name;
		int fsize;
		int checksum;
		unsigned int offset;
		F_Read_Primitive(stream,unsigned char,type);
		if(type==2){
			int stringOffset;
			long pos;
			F_Read_Primitive(stream,int,stringOffset);
			pos=ftell(stream);
			fseek(stream,header->fileStart+stringOffset,SEEK_SET);
			F_Read_Primitive(stream,unsigned char,type);
			name=F_Read_EncryptedString(stream);
			fseek(stream,pos,SEEK_SET);
		}else if(type==3 || type==4)
			name=F_Read_EncryptedString(stream);
		else
			continue;
		fsize=F_Read_CompressedInt(stream);
		checksum=F_Read_CompressedInt(stream);
		offset=F_Read_Offset(stream,header);
		if(type==3){
			WZLib_Directory* newDir=_WZLib_Directory_New(name);
			_LL_Entry* newEntry=_LL_Entry_New((void*)newDir);
			long co=0;
			dir->_firstChild=_LL_Entry_Attach(dir->_firstChild,newEntry);
			newDir->inh.parent=(WZLib_Object*)dir;
			newDir->inh.blockSize=fsize;
			newDir->inh.checksum=checksum;
			newDir->inh.offset=offset;
			co=ftell(stream);
			fseek(stream,offset,SEEK_SET);
			newDir->parentFile=dir->parentFile;
			_WZLib_Directory_Parse(newDir,stream,header);
			fseek(stream,co,SEEK_SET);
		}else{
			WZLib_Image* newImg=_WZLib_Image_New(name);
			_LL_Entry* newEntry=_LL_Entry_New((void*)newImg);
			dir->_firstChild=_LL_Entry_Attach(dir->_firstChild,newEntry);
			newImg->inh.parent=(WZLib_Object*)dir;
			newImg->inh.blockSize=fsize;
			newImg->inh.checksum=checksum;
			newImg->inh.offset=offset;
			newImg->_stream=stream;
			newImg->parentFile=dir->parentFile;
		}
		free(name);
	}
	dir->inh.parsed=1;
	return WZLib_Error_NoError;
}

WZLib_Header*		_WZLib_Header_New(const char* name){
	WZLib_Header* ret=(WZLib_Header*)malloc(sizeof(WZLib_Header));
	memset(ret,0,sizeof(WZLib_Header));
	ret->inh=*_WZLib_Object_New(name,WZLib_ObjectType_Header);
	return ret;
}

ErrorCode		_WZLib_Header_Parse(WZLib_Header* header,FILE* stream){
	ErrorCode ret=WZLib_Error_NoError;
	long p=0;
	header->ident=F_Read_Chars(stream,4);
	F_Read_Primitive(stream,unsigned long long,header->fileSize);
	F_Read_Primitive(stream,unsigned int,header->fileStart);
	header->copyright=F_Read_NullString(stream);
	p=ftell(stream);
	fseek(stream,header->fileStart,SEEK_SET);
	F_Read_Primitive(stream,unsigned short,header->version);
	fseek(stream,p,SEEK_SET);
	header->inh.parsed=1;
	return ret;
}

unsigned int		_WZLib_File_GetVersionHash(int encver,int realver){
	char versionNumberStr[5];
	int versionHash=0;
	int a,b,c,d;
	unsigned int ret=0;
	int i=0,l=0;
	l=sprintf(versionNumberStr,"%d",realver);
	if(l<=0)
		return -1;
	for(i=0;i<l;i++)
		versionHash=(32*versionHash)+(int)versionNumberStr[i]+1;
	a=(versionHash>>24)&0xFF;
	b=(versionHash>>16)&0xFF;
	c=(versionHash>> 8)&0xFF;
	d=(versionHash    )&0xFF;
	ret=0xFF^a^b^c^d;
	if(ret==encver)
		return versionHash;
	return 0;
}

ErrorCode		WZLib_File_Parse(WZLib_File* file){
	ErrorCode ret=WZLib_Error_NoError;
	unsigned short dump;
	file->_wzDir=_WZLib_Directory_New(file->_fileName);
	file->_wzDir->parentFile=file;
	file->_stream=fopen(file->_fileName,"rb");
	if(file->_stream==NULL)
		return WZLib_Error_File_Parse_BadFileName;
	/*now parse header*/
	_WZLib_Header_Parse(file->_header,file->_stream);
	F_Read_Primitive(file->_stream,unsigned short,dump);
	/*now determine version*/
	if(file->_fileVersion==-1){
		int j=1;
		for(j=1;j<0x7F;j++){
			file->_fileVersion=j;
			file->_header->versionHash=_WZLib_File_GetVersionHash(file->_header->version,file->_fileVersion);
			if(file->_header->versionHash!=0)
				break;
		}
	}
	ret=_WZLib_Directory_Parse(file->_wzDir,file->_stream,file->_header);
	if(ret==WZLib_Error_NoError)
		file->inh.parsed=1;
	return ret;
}

WZLib_Object* WZLib_File_Get_n(WZLib_File* file,const char* name){
	return WZLib_Directory_Get_n(file->_wzDir,name);
}
WZLib_Object* WZLib_File_Get_i(WZLib_File* file,int index){
	return WZLib_Directory_Get_i(file->_wzDir,index);
}
int WZLib_File_GetSize(WZLib_File* file){
	return WZLib_Directory_GetSize(file->_wzDir);
}

WZLib_Object* WZLib_Directory_Get_i(WZLib_Directory* dir,int index){
	int curIndex=0;
	_LL_Entry* cur=dir->_firstChild;
	do{
		if(curIndex==index)
			return (WZLib_Object*)cur->data;
		cur=cur->next;
		curIndex++;
	}while(cur!=NULL);
	return NULL;
}
WZLib_Object* WZLib_Directory_Get_n(WZLib_Directory* dir,const char* name){
	_LL_Entry* cur=dir->_firstChild;
	do{
		if(strcmp(WZLIB_OBJECT(cur->data)->name,name)==0)
			return WZLIB_OBJECT(cur->data);
		cur=cur->next;
	}while(cur!=NULL);
	return NULL;
}
int WZLib_Directory_GetSize(WZLib_Directory* dir){
	return _LL_Entry_GetSize(dir->_firstChild);
}

void WZLib_Image_Unparse(WZLib_Image* image){
	if(!WZLIB_OBJECT(image)->parsed)
		return;
	_WZLib_SubProperty_Delete(image->_sub);
	WZLIB_OBJECT(image)->parsed=0;
}

ErrorCode	WZLib_Image_Parse(WZLib_Image* image){
	ErrorCode ret=WZLib_Error_NoError;
	long originalPos;
	unsigned char b;
	char* str;
	unsigned short c;
	if(image->inh.parsed)
		return WZLib_Error_NoError;
	originalPos=ftell(image->_stream);
	fseek(image->_stream,image->inh.offset,SEEK_SET);
	F_Read_Primitive(image->_stream,unsigned char,b);
	if(b!=0x73)
		return WZLib_Error_Image_Parse_Error1;
	str=F_Read_EncryptedString(image->_stream);
	if(strcmp(str,"Property")!=0){
		free(str);
		return WZLib_Error_Image_Parse_Error1;
	}
	free(str);
	F_Read_Primitive(image->_stream,unsigned short,c);
	if(c!=0)
		return WZLib_Error_Image_Parse_Error1;
	image->_sub->_firstChild=NULL;
	image->_sub->inh.parentImg=image;
	ret=_WZLib_SubProperty_Parse(image->_sub,image->_stream,image->inh.offset);
	fseek(image->_stream,originalPos,SEEK_SET);
	if(ret==WZLib_Error_NoError)
		image->inh.parsed=1;
	return ret;
}

WZLib_Property* WZLib_Image_Get_n(WZLib_Image* image,const char* name){
	return WZLib_SubProperty_Get_n(image->_sub,name);
}

WZLib_Property* WZLib_Image_Get_i(WZLib_Image* image,int index){
	return WZLib_SubProperty_Get_i(image->_sub,index);
}

int WZLib_Image_GetSize(WZLib_Image* image){
	return WZLib_SubProperty_GetSize(image->_sub);
}

WZLib_Property* WZLib_UOLProperty_Resolve(WZLib_PrimitiveProperty* uolProp){
	char** tokens;
	int numTokens;
	int i=0;
	WZLib_Property* curProp=WZLIB_PROPERTY(WZLIB_OBJECT(uolProp)->parent);
	if(WZLIB_PROPERTY(uolProp)->propType!=WZLib_Prop_UOL)
		return NULL;
	_tokenize(uolProp->val.strVal,"/",&tokens,&numTokens);
	for(i=0;i<numTokens;i++){
		if(strcmp(tokens[i],"..")==0){
			WZLib_Object* obj=WZLIB_OBJECT(curProp)->parent;
			if(obj==NULL){ /*oh teh noez*/
				curProp=NULL;
				break;
			}
			if(obj->type!=WZLib_ObjectType_Property){ /*oh teh noez*/
				curProp=NULL;
				break;
			}
			curProp=(WZLib_Property*)obj;
			continue;
		}
		/*go down*/
		{
			WZLib_Object* obj=WZLib_Object_Get_n(WZLIB_OBJECT(curProp),tokens[i]);
			if(obj==NULL){ /*oh teh noez*/
				curProp=NULL;
				break;
			}
			curProp=(WZLib_Property*)obj;
		}
	}
	for(i=0;i<numTokens;i++)
		free(tokens[i]);
	free(tokens);
	return curProp;
}

void WZLib_Image_ResolveUOLs(WZLib_Image* image){
	WZLib_SubProperty_ResolveUOLs(image->_sub);
}

void WZLib_SubProperty_ResolveUOLs(WZLib_SubProperty* subProp){
	_LL_Entry* cur=subProp->_firstChild;
	while(cur!=NULL){
		WZLib_Property* prop=WZLIB_PROPERTY(cur->data);
		if(WZLIB_SPROPTYPE(prop)==WZLib_Prop_Sub)
			WZLib_SubProperty_ResolveUOLs(WZLIB_SUBPROPERTY(prop));
		else if(prop->propType==WZLib_Prop_UOL){
			WZLib_Property* newProp=WZLib_UOLProperty_Resolve(WZPP(prop));
			if(newProp!=NULL){
				newProp=WZLib_Property_Copy(newProp);
				cur->data=(void*)newProp; /*replace w/ ref to other prop*/
				/*reparent*/
				((WZLib_Object*)newProp)->parent=(WZLib_Object*)subProp;
				strcpy(WZLIB_OBJECT(newProp)->name,WZLIB_OBJECT(prop)->name);
				free(prop); /*kill old UOL*/
			}
		}
		cur=cur->next;
	}
}

WZLib_SubProperty* _WZLib_SubProperty_Copy(WZLib_SubProperty* subProp){
	WZLib_SubProperty* ret=_WZLib_SubProperty_New(WZLIB_OBJECT(subProp)->name);
	_LL_Entry* cur=subProp->_firstChild;
	memcpy(ret,subProp,sizeof(WZLib_SubProperty));
	ret->_firstChild=NULL;
	while(cur!=NULL){
		WZLib_Property* copyProp=WZLib_Property_Copy(WZLIB_PROPERTY(cur->data));
		_LL_Entry* newE=_LL_Entry_New((void*)copyProp);
		ret->_firstChild=_LL_Entry_Attach(ret->_firstChild,newE);
		cur=cur->next;
	}
	return ret;
}

WZLib_PNGProperty* _WZLib_PNGProperty_Copy(WZLib_PNGProperty* pngProp){
	WZLib_PNGProperty* ret=_WZLib_PNGProperty_New(WZLIB_OBJECT(pngProp)->name);
	memcpy(ret,pngProp,sizeof(WZLib_PNGProperty));
	if(pngProp->_compBytes!=NULL){
		ret->_compBytes=(unsigned char*)malloc(sizeof(unsigned char)*ret->_length);
		memcpy(ret->_compBytes,pngProp->_compBytes,ret->_length);
	}
	if(pngProp->_decBytes!=NULL){
		ret->_decBytes=(unsigned char*)malloc(sizeof(unsigned char*)*ret->_decLen);
		memcpy(ret->_decBytes,pngProp->_decBytes,ret->_decLen);
	}
#	ifdef WZLIB_HAVE_SDL
	ret->png=NULL;
	if(pngProp->png!=NULL && pngProp->pngParsed && SDL_GetVideoSurface()!=NULL){
		SDL_Surface* cur=pngProp->png;
		SDL_Surface* temp=SDL_CreateRGBSurface(SDL_HWSURFACE,cur->w,cur->h,cur->pitch,0,0,0,SDL_ALPHA_TRANSPARENT);
		SDL_BlitSurface(cur,NULL,temp,NULL);
		if(SDL_GetVideoSurface()!=NULL)
			ret->png=SDL_DisplayFormatAlpha(temp);
		SDL_FreeSurface(temp);
	}
#	endif
	return ret;
}

WZLib_Property* WZLib_Property_Copy(WZLib_Property* prop){
	WZLib_Property* ret=NULL;
	/*primitive, sub, png, mp3 */
	switch(((WZLib_PropertyType)(((int)(WZLIB_PROPERTY(prop)->propType))/100*100))){
	case WZLib_Prop_Primitive:
		ret=WZLIB_PROPERTY(_WZLib_PrimitiveProperty_New(WZLIB_OBJECT(prop)->name,prop->propType));
		memcpy(WZPP(ret),WZPP(prop),sizeof(WZLib_PrimitiveProperty));
		switch(prop->propType){
		case WZLib_Prop_String:
		case WZLib_Prop_UOL:
			memcpy(WZPP(ret)->val.strVal,WZPP(prop)->val.strVal,strlen(WZPP(prop)->val.strVal));
			break;
		case WZLib_Prop_Vector:
			memcpy(WZPP(ret)->val.vecVal,WZPP(ret)->val.vecVal,2*sizeof(int));
			break;
		}
		break;
	case WZLib_Prop_Sub:
		ret=WZLIB_PROPERTY(_WZLib_SubProperty_Copy(WZLIB_SUBPROPERTY(prop)));
		switch(prop->propType){
		case WZLib_Prop_Canvas:
			((WZLib_CanvasProperty*)ret)->png=_WZLib_PNGProperty_Copy(((WZLib_PNGProperty*)WZLIB_CANVASPROPERTY(prop)->png));
			break;
		}
		break;
	}
	/* after that's all done, copy name */
	if(ret!=NULL){
		WZLIB_OBJECT(ret)->name=(char*)malloc(sizeof(char)*100);
		memcpy(WZLIB_OBJECT(ret)->name,WZLIB_OBJECT(prop)->name,strlen(WZLIB_OBJECT(prop)->name));
	}
	return ret;
}

WZLib_SoundProperty* _WZLib_SoundProperty_New(const char* name){
	WZLib_SoundProperty* ret=(WZLib_SoundProperty*)malloc(sizeof(WZLib_SoundProperty));
	memset(ret,0,sizeof(WZLib_SoundProperty));
	ret->inh=*_WZLib_Property_New(name,WZLib_Prop_MP3);
	return ret;
}

void _WZLib_SoundProperty_Parse(WZLib_SoundProperty* prop,FILE* stream){
	fseek(stream,1,SEEK_CUR);
	prop->_lenData=F_Read_CompressedInt(stream);
	prop->lenMS=F_Read_CompressedInt(stream);
	prop->_data=(unsigned char*)malloc(2+sizeof(unsigned char*)*prop->_lenData);
	fread(prop->_data+2,sizeof(unsigned char),prop->_lenData,stream);
	prop->_data[0]=0xff;
	prop->_data[1]=0xf0;
	prop->_lenData+=2;
	WZLIB_OBJECT(prop)->parsed=1;
	return;
}

void WZLib_SoundProperty_Play(WZLib_SoundProperty* prop){
	if(prop->_data==NULL)
		return;
#	ifdef WZLIB_HAVE_SDL
	if(prop->_mus==NULL){
		SDL_RWops* rw=SDL_RWFromConstMem((const void*)prop->_data,prop->_lenData);
		prop->_mus=Mix_LoadMUS_RW(rw);
	}
	Mix_PlayMusic(prop->_mus,1);
	Mix_VolumeMusic(MIX_MAX_VOLUME);
#	endif
}

void _WZLib_SoundProperty_Delete(WZLib_SoundProperty* prop){
	if(!WZLIB_OBJECT(prop)->parsed)
		return;
	if(prop->_data!=NULL){
		free(prop->_data);
		prop->_data=NULL;
	}
#	ifdef WZLIB_HAVE_SDL
	if(prop->_mus!=NULL){
		Mix_FreeMusic(prop->_mus);
		prop->_mus=NULL;
	}
#	endif
	WZLIB_OBJECT(prop)->parsed=0;
}