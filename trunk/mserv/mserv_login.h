#ifndef __MSERV_LOGIN_H
#define __MSERV_LOGIN_H
#include "mserv_config.h"

#include <prio.h>

typedef struct _struct_LoginServer_Data{
	PRFileDesc*		socket;
}LoginServer_Data;

void	MServ_Login_Start	();
void	MServ_Login_Listen	(void* data);

#endif
