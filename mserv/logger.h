#ifndef __LOGGER_H
#define __LOGGER_H

#ifdef __cplusplus
extern "C"{
#endif

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ewfutils.h"

#ifndef _LOGGER_MAX_LOG_LENGTH
#define _LOGGER_MAX_LOG_LENGTH 2048
#endif

#ifndef _LOGGER_NO_SHORT_NAMES
#define lnew	_Logger_New
#define lfree	_Logger_Free
#define ladd	_Logger_AddDestination
#define llog	_Logger_Log
#define LL_WARN		_LOGGER_LEVEL_WARNING
#define LL_INFO		_LOGGER_LEVEL_INFO
#define LL_ERR		_LOGGER_LEVEL_ERROR
#define LL_CRIT		_LOGGER_LEVEL_CRIT
#endif

//levels
#define	_LOGGER_LEVEL_WARNING	"WARN"
#define _LOGGER_LEVEL_INFO	"INFO"
#define _LOGGER_LEVEL_ERROR	"ERR"
#define _LOGGER_LEVEL_CRIT	"CRIT"

typedef struct _struct_Logger{
	_LL_Entry*	destinations;	//linked list of FILE*'s
	char*		name;		//prefix for messages
}_Logger;


FILE*		_Logger_AddDestination(_Logger* log,FILE* dest){
	if(dest!=NULL){
		_LL_Entry* l=_LL_Entry_New((void*)dest);
		log->destinations=_LL_Entry_Attach(log->destinations,l);
	}
	return dest;
}
_Logger*	_Logger_New(const char* name){
	_Logger* ret=(_Logger*)memset(malloc(sizeof(_Logger)),'\0',sizeof(_Logger));
	ret->name=(char*)malloc(sizeof(char)*(strlen(name)+1));
	strncpy(ret->name,name,strlen(name)+1);
	#ifndef _LOGGER_NO_STDOUT
	_Logger_AddDestination(ret,stdout);
	#endif
	return ret;
}
void		_Logger_Free(_Logger* log){
	_LL_Entry* cur=log->destinations;
	free(log->name);
	log->name=NULL;
	while(cur!=NULL){
		_LL_Entry* temp=cur;
		cur=cur->next;
		free(temp);
	}
	free(log);
}
#define lmll _LOGGER_MAX_LOG_LENGTH
void		_Logger_Log(_Logger* log,const char* level,const char* format,...){
	char* msg=(char*)memset(malloc(lmll),'\0',lmll);
	va_list ap;
	_LL_Entry* cur=log->destinations;
	va_start(ap,format);
	vsnprintf(msg,lmll,format,ap);
	va_end(ap);
	while(cur!=NULL){
		FILE* dest=(FILE*)cur->data;
		fprintf(dest,"%s[%s]: %s\n",log->name,level,msg);
		cur=cur->next;
	}
	free(msg);
}
#undef lmll

#ifdef __cplusplus
};
#endif

#endif
