#include "stdio.h"
#include "stdlib.h"
#include "sys/wait.h"
#include "time.h"
#include "unistd.h"

int rnd_in_range() {
  int max = 5, min = 0;
  srand(time(NULL));
  return rand() % (max - min + 1) + min;
}

int main() {
  int i;
  int res = 0;
  int fd[2]; // one pipe for child write -> parent read

  if (pipe(fd) == -1) {
    fprintf(stderr, "Pipe Failed");
    return 1;
  }

  if (fork() > 0) {
    read(fd[0], &res, sizeof(res));
  } else {
    res = rnd_in_range();
    printf("writing %d\n", res);
    write(fd[1], &res, sizeof(res));
    exit(1);
  }

  int pids[res];
  for (i = 0; i < res; i++) {
    if ((pids[i] = fork()) == 0) {
      printf("child %d\n", i);
      exit(1);
    }
  }

  for (i = 0; i < res; i++) {
    printf("waiting for %d\n", pids[i]);
    wait(&pids[i]);
  }
}
