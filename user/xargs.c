#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"
#include "kernel/param.h"

void
copy(char **p1, char *p2)
{
	*p1 = malloc(strlen(p2)+1);
	strcpy(*p1, p2);
}

// read arg from stdin
// i is the beginning index
int
readLine(char **args, int i)
{
	char buf[1024];
	int j = 0;
	// read a line
	while(read(0, buf + j, 1)){
		if(buf[j] == '\n'){
			buf[j] = 0;
			break;
		}
		j ++;

		if(j == 1024){
			fprintf(2, "Parameters are too long!\n");
			exit(1);
		}
	}

	// have nothing to read
	if(j == 0){
		return -1;
	}

	// Divided by spaces
	int k = 0;
	while(k < j){
		if(i > MAXARG){
			fprintf(2, "too much parameters!\n");
			exit(1);
		}
		while((k < j) && (buf[k] ==' ')){
			k ++;
		}

		// starting index
		int l = k;
		while((k < j) && (buf[k] != ' ')){
			k ++;
		}
		// ????
		buf[k++] = 0;
		copy(&args[i], buf + l);
		i ++;
	}
	return i;
}

int
main(int argc, char *argv[])
{
	if(argc < 2){
		fprintf(2, "require more args");
		exit(1);
	} else {
		char *args[MAXARG];
		for(int i = 1; i < argc; i ++){
			copy(&args[i-1], argv[i]);
		}

		int end;
		while((end = readLine(args, argc - 1)!= -1)){
			argv[end] = 0;
			if(fork() == 0){
				exec(args[0], args);
				exit(1);
			} else {
				wait(0);
			}
		}
		exit(0);
	}

}