#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

void primes(int *p){
	int first; // note the first read number
	int next; // note next numbers
	close(p[1]);
	if(read(p[0], (void*)&first, sizeof(first)) != 4){
		fprintf(2, "falied\n");
		close(p[0]);
		exit(1);
	} else {
		printf("prime %d\n", first);

    if(read(p[0], (void*)&next, sizeof(next))){
      int fd[2];
      pipe(fd);
      if(fork() == 0){
        primes(fd);
      } else {
        close(fd[0]);
        while(1){
          if(next % first != 0){
            write(fd[1], (void*)&next, sizeof(next));
          }
          if(read(p[0], (void*)&next, sizeof(next)) == 0){
            break;
          }
        }
        close(p[0]);
        close(fd[1]);
        wait(0);
      }
    }
	}
  exit(0);
}

int
main(int argc, char *argv[])
{
	int start = 2;
	int end = 35;

	int p[2];
	pipe(p);

	if(fork() == 0){
		primes(p);
	} else {
		close(p[0]);
		// loop to send
		for(int i = start; i <= end; i ++){
			if(write(p[1], (void*)&i, sizeof(i)) != 4){
				fprintf(2, "failed\n");
				close(p[1]);
				exit(1);
			}
		}
    close(p[1]);
    wait(0);
	}
  exit(0);
}