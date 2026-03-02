#include "stdio.h"
#include "stdlib.h"
#include "sys/wait.h"
#include "time.h"
#include "unistd.h"

int max_in_range(int *start) {
  int res = start[0];

  for (int i = 0; i < 51; i++) {
    if (res < start[i])
      res = start[i];
  }
  return res;
}

int main() {
  int i;
  int n[500];
  int pids[5];
  int maxes[5];
  int min = 0;
  int max = 255;
  int res = 0;
  int fd[5][2]; // one pipe for child write -> parent read

  srand(time(NULL));
  for (i = 0; i < 500; i++) {
    n[i] = rand() % (max - min + 1) + min;
  }

  for (i = 0; i < 5; i++) {
    if (pipe(fd[i]) == -1) {
      fprintf(stderr, "Pipe Failed");
      return 1;
    }

    if ((pids[i] = fork()) == 0) {
      int max = max_in_range(&n[i * 51]);
      write(fd[i][1], &max, sizeof(max));
      exit(1);
    }
  }

  for (i = 0; i < 5; i++) {
    printf("waiting for %d\n", pids[i]);
    wait(&pids[i]);
    read(fd[i][0], &maxes[i], sizeof(maxes[i]));
  }

  for (i = 0; i < 5; i++) {
    res = res + maxes[i];
  }

  printf("res = %d", res);
}
