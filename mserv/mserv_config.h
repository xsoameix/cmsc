#ifndef __MSERV_CONFIG_H
#define __MSERV_CONFIG_H

#include <stdlib.h>

typedef struct _struct_MServ_Config{
	int	loginPort;
}MServ_Config;

void*	zalloc(size_t size);

//extern MServ_Config config;

#endif
