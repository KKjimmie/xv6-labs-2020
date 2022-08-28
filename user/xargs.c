#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"
#include "kernel/param.h"

int
main(int argc, char *argv[])
{
	char buf[512];
	char *args[MAXARG];
	char c;
	char *p;
	int n;

	if(argc < 2){
		fprintf(2, "require more paremeter.\\n");
		exit(1);
	}

	for(int i = 1; i < argc; i ++){
		args[i-1] = argv[i];
	}

	while(1){
		p = buf;
		// read line
		while((n = read(0, &c, 1)) && c != '\n'){
			*p = c;
			p ++;
		}

		// end
		*p = '\0';
		if(p != buf){
			args[argc -1] = buf;
			if (fork() == 0){
				exec(argv[1], args);
				exit(0);
			}
			wait(0);
		}
		if (n == 0) break;
		if (n < 0){
			fprintf(2, "read error.\\n");
			exit(1);
		}
	}
	exit(0);
}
