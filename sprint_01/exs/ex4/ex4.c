#include "stdio.h"
#include "stdlib.h"
#include "sys/wait.h"
#include "unistd.h"
#include <stdio.h>

void main() {
  int i;
  int pids[4];

  for (i = 0; i < 4; i++) {
    if ((pids[i] = fork()) > 0) {
      printf("Inside parent\n");
    } else {
      switch (i) {
      case 0:
        sleep(1);
        printf("In process 1\n");
        break;
      case 1:
        sleep(1);
        printf("In process 2\n");
        break;
      case 2:
        sleep(1);
        printf("In process 3\n");
        break;
      case 3:
        sleep(1);
        printf("In process 4\n");
        break;
      }
      exit(1);
    }
  }

  for (i = 0; i < 4; i++) {
    if (pids[i] % 2 != 0) {
      printf("waiting for %d\n", pids[i]);
      wait(&pids[i]);
    }
  }

  printf("End of execution!\n");
}
