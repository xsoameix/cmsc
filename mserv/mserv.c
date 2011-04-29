#include <stdio.h>
#include <stdlib.h>

#include <prio.h>

#include "mserv_login.h"

#include "logger.h"

int main(int argc,char* argv[]){
	_Logger* l=lnew("test");
	llog(l,LL_INFO,"argc: %d, argv: %#08x, l: %#08x",argc,argv,l);
	lfree(l);
	return 0;
#	ifdef _PR_INET6
	printf("I haz inet6\n");
#	endif
	MServ_Login_Start();
	printf("Press enter to die...\n");
	getc(stdin);
	PR_Close(__loginData.socket);
	return 0;
}
