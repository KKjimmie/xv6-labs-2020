#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  int p1[2]; // p->c
  int p2[2]; // c->p
  char *ptc = "a"; // parent send to child
  char *ctp = "b"; // child send to parent
  pipe(p1);
  pipe(p2);
  if(fork() == 0){
    close(0);
    char pmsg; // receive message from parent
    if(read(p1[0], &pmsg, 1) != 1){
      fprintf(2, "child can't receive message from parent.\n");
      close(p1[0]);
      exit(1);
    }else {
      printf("child received : %c\n", pmsg);
      printf("%d: received ping\n", getpid());
    }
    if(write(p2[1], ctp, 1) != 1){
      fprintf(2, "child can't send to parent.\n");
      close(p2[1]);
      exit(1);
    } else {
      close(p2[1]);
      exit(0);
    }
  } else {
    close(0);
    if(write(p1[1], ptc, 1) != 1){
      fprintf(2, "parent can't send to child.\n");
      close(p1[1]);
      exit(1);
    } else {
      close(p1[1]);
      wait(0); // attention!
    }

    char cmsg; // receive message from child
    if(read(p2[0], &cmsg, 1) != 1){
      fprintf(2, "parent can't receive message from child.\n");
      close(p2[0]);
      exit(1);
    }else {
      printf("parent received : %c\n", cmsg);
      printf("%d: received pong\n", getpid());
      exit(0);
    }
  }
}