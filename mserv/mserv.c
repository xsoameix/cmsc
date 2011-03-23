#include <stdio.h>
#include <stdlib.h>

#include <prio.h>

#include "mserv_login.h"

int main(int argc,char* argv[]){
#	ifdef _PR_INET6
	printf("I haz inet6\n");
#	endif
	MServ_Login_Start();
	while(1==1);
	return 0;
}
