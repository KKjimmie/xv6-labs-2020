#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
	if(argc != 2){
		fprintf(2, "please enter a number!\n");
		exit(1);
	} else {
		int n;
		n = atoi(argv[1]);
		sleep(n);
		exit(0);
	}
}