#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  if(argc > 1){
    fprintf(2, "too much parameters!\n");
    exit(1);
  }
  printf("uptime : %d\n", uptime());
  exit(0);
}