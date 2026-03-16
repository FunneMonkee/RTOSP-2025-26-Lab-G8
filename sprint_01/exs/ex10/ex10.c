#include "stdio.h"
#include "stdlib.h"
#include "time.h"
#include <pthread.h>
#include <stdio.h>
#include <time.h>

struct client {
  int number;
  char name;
  int balance;
};
struct client clients[100];
int negatives[100];
float avg = 0;
pthread_mutex_t lock =
    PTHREAD_MUTEX_INITIALIZER; // lock to make threads execute in order

void *check_balance(void *arg) {
  pthread_mutex_lock(&lock);

  for (int i = 0; i < 100; i++) {
    if (clients[i].balance < 0) {
      negatives[i] = i;
    } else {
      negatives[i] = -1;
    }
  }

  printf("Balance checked\n");

  free(arg);
  pthread_mutex_unlock(&lock);
  return NULL;
}
void *show_balance(void *arg) {
  pthread_mutex_lock(&lock);

  for (int i = 0; i < 100; i++) {
    if (negatives[i] != -1) {
      struct client client = clients[negatives[i]];
      printf("Client %c as negative balance %d\n", client.name, client.balance);
    }
  }
  free(arg);
  pthread_mutex_unlock(&lock);

  return NULL;
}

void *calc_avg(void *arg) {
  pthread_mutex_lock(&lock);

  for (int i = 0; i < 100; i++) {
    avg += clients[i].balance;
  }

  avg = avg / 100;

  printf("AVG is %f\n", avg);

  free(arg);
  pthread_mutex_unlock(&lock);
  return NULL;
}

void *remove_neg(void *arg) {
  pthread_mutex_lock(&lock);

  for (int i = 0; i < 100; i++) {
    if (negatives[i] != -1) {
      struct client client = clients[negatives[i]];
      client.balance -= client.balance;
      printf("Client %c as been removed\n", client.name);
    }
  }

  avg = avg / 100;

  free(arg);
  pthread_mutex_unlock(&lock);
  return NULL;
}

int main() {
  void *res;
  int s, i;
  pthread_t threads[3];
  pthread_mutex_init(&lock, NULL);

  srand(time(NULL));
  for (i = 0; i < 100; i++) {
    clients[i].balance = rand() % 200 - 101;
    clients[i].name = '0' + i;
    clients[i].number = i;
  }

  s = pthread_create(&threads[0], NULL, &check_balance, NULL);
  if (s != 0)
    printf("error on thread 1 create\n");

  s = pthread_create(&threads[1], NULL, &show_balance, NULL);
  if (s != 0)
    printf("error on thread 2 create\n");

  s = pthread_create(&threads[2], NULL, &calc_avg, NULL);
  if (s != 0)
    printf("error on thread 3 create\n");

  for (i = 0; i < 3; i++) {
    int s = pthread_join(threads[i], &res);
    if (s != 0)
      printf("error on thread join, thread %d\n", i);
  }

  if (avg < 0) {
    s = pthread_create(&threads[0], NULL, &remove_neg, NULL);
    if (s != 0)
      printf("error on thread 4 create\n");

    s = pthread_join(threads[0], &res);
    if (s != 0)
      printf("error on thread 4 join\n");
  }

  free(res);
  pthread_mutex_destroy(&lock);
}
