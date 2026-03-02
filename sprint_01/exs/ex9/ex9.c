#include "stdio.h"
#include "stdlib.h"
#include "time.h"
#include <pthread.h>
#include <stdio.h>
#include <time.h>

struct fill_info {
  int *start_pos;
};

struct mult_info {
  int *start_pos_1;
  int *start_pos_2;
  int *start_pos_res;
};

void *fill(void *arg) {
  struct fill_info *tinfo = arg;
  int *start = tinfo->start_pos;

  // here so both matrixes are the same
  // makes the checking the result matrix easier
  srand(time(NULL));
  for (int i = 0; i < 16; i++) {
    for (int j = 0; j < 16; j++) {
      // small range to test
      *((start + i * 16) + j) = rand() % (10 - 2) + 1;
    }
  }

  return NULL;
}

void *multiply(void *arg) {
  struct mult_info *tinfo = arg;
  int *start_1 = tinfo->start_pos_1;
  int *start_2 = tinfo->start_pos_2;
  int *start_res = tinfo->start_pos_res;

  for (int i = 0; i < 2; i++) {
    for (int j = 0; j < 16; j++) {
      for (int k = 0; k < 16; k++) {
        *((start_res + i * 16) + j) +=
            *(start_1 + i * 16 + k) * *(start_2 + k * 16 + j);
      }
    }
  }

  free(arg);
  return NULL;
}

int main() {
  void *res;
  int s;
  int i;
  pthread_t threads[8];
  int matrix_1[16][16];
  int matrix_2[16][16];
  int result[16][16];

  struct fill_info tinfo;
  tinfo.start_pos = &matrix_1[0][0];
  struct fill_info tinfo2;
  tinfo2.start_pos = &matrix_2[0][0];

  s = pthread_create(&threads[0], NULL, &fill, &tinfo);
  if (s != 0)
    printf("error on thread create\n");

  s = pthread_create(&threads[1], NULL, &fill, &tinfo2);
  if (s != 0)
    printf("error on thread 2 create\n");

  // wait before multiplication
  for (i = 0; i < 2; i++) {
    int s = pthread_join(threads[i], res);
    if (s != 0)
      printf("error on thread join\n");
    free(res);
  }
  for (i = 0; i < 8; i++) {
    int index = i * 2;
    // NOT this struct mult_info tinfo;
    //  malloc so no address is being reused
    struct mult_info *tinfo = malloc(sizeof(struct mult_info));
    tinfo->start_pos_1 = &matrix_1[index][0];
    tinfo->start_pos_2 = &matrix_2[0][0];
    tinfo->start_pos_res = &result[index][0];

    s = pthread_create(&threads[i], NULL, &multiply, tinfo);
    if (s != 0)
      printf("error on thread create\n");
  }

  for (i = 0; i < 8; i++) {
    int s = pthread_join(threads[i], res);
    if (s != 0)
      printf("error on thread join, thread %d\n", i);
    free(res);
  }
  printf("matrix_1\n");
  for (int i = 0; i < 16; i++) {
    for (int j = 0; j < 16; j++) {
      printf("%d |", matrix_1[i][j]);
    }
    printf("\n");
  }
  printf("matrix_2\n");
  for (int i = 0; i < 16; i++) {
    for (int j = 0; j < 16; j++) {
      printf("%d |", matrix_2[i][j]);
    }
    printf("\n");
  }

  printf("result\n");
  for (int i = 0; i < 16; i++) {
    for (int j = 0; j < 16; j++) {
      printf("%d |", result[i][j]);
    }
    printf("\n");
  }
}
