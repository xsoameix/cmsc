#include "mserv_config.h"

#include <stdlib.h>
#include <string.h>

void*	zalloc(size_t size){
	void* ret=malloc(size);
	memset(ret,0,size);
	return ret;
}