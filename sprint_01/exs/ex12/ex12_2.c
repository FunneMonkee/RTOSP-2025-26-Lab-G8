#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define BUFFER_SIZE 5

int buffer[BUFFER_SIZE];
int count = 0;
int in = 0;
int out = 0;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t not_full = PTHREAD_COND_INITIALIZER;
pthread_cond_t not_empty = PTHREAD_COND_INITIALIZER;

void *producer(void *arg) {
  for (int i = 1; i <= 20; i++) {

    pthread_mutex_lock(&mutex);

    while (count == BUFFER_SIZE)
      pthread_cond_wait(&not_full, &mutex);

    buffer[in] = i;
    printf("Produced: %d\n", i);

    in = (in + 1) % BUFFER_SIZE;
    count++;

    pthread_cond_signal(&not_empty);
    pthread_mutex_unlock(&mutex);

    sleep(1);
  }
  return NULL;
}

void *consumer(void *arg) {
  int item;

  for (int i = 1; i <= 20; i++) {

    pthread_mutex_lock(&mutex);

    while (count == 0)
      pthread_cond_wait(&not_empty, &mutex);

    item = buffer[out];
    printf("Consumed: %d\n", item);

    out = (out + 1) % BUFFER_SIZE;
    count--;

    pthread_cond_signal(&not_full);
    pthread_mutex_unlock(&mutex);

    sleep(2);
  }

  return NULL;
}

int main() {
  pthread_t p, c;

  pthread_create(&p, NULL, producer, NULL);
  pthread_create(&c, NULL, consumer, NULL);

  pthread_join(p, NULL);
  pthread_join(c, NULL);

  return 0;
}
