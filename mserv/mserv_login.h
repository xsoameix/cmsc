#ifndef __MSERV_LOGIN_H
#define __MSERV_LOGIN_H
#include "mserv_config.h"

#include <prio.h>

struct _struct_LoginServer_Data{
	PRFileDesc*		socket;
}__loginData;

void	MServ_Login_Start	();
void	MServ_Login_Listen	(void* data);

#endif
