#include "stdio.h"
#include "stdlib.h"
#include "sys/wait.h"
#include "time.h"
#include "unistd.h"
#include <pthread.h>
#include <stdio.h>

struct thread_info {
  int *start_pos;
  int *result;
};

void *max_in_range(void *arg) {
  struct thread_info *tinfo = arg;
  int *start = tinfo->start_pos;
  int *result = tinfo->result;

  for (int i = 0; i < 100; i++) {
    if (*result < start[i])
      *result = start[i];
  }

  printf("max in thread =%d\n", *result);

  return EXIT_SUCCESS;
}

int main() {
  void *res;
  pthread_t threads[10];
  int n[1000];
  int sum_arr[10];
  int i;
  int min = 0;
  int max = 255;
  int sum = 0;

  srand(time(NULL));
  for (i = 0; i < 1000; i++) {
    n[i] = rand() % (max - min + 1) + min;
  }
  for (i = 0; i < 10; i++) {
    struct thread_info tinfo;
    tinfo.start_pos = &n[i * 10];
    tinfo.result = &sum_arr[i];

    int s = pthread_create(&threads[i], NULL, &max_in_range, &tinfo);
    if (s != 0)
      printf("error on thread create\n");
  }

  for (i = 0; i < 10; i++) {
    int s = pthread_join(threads[i], res);
    if (s != 0)
      printf("error on thread join\n");
    free(res);
  }

  max = sum_arr[0];
  for (int i = 1; i < 10; i++) {
    if (max < sum_arr[i])
      max = sum_arr[i];
  }
  printf("max = %d\n", max);
}
