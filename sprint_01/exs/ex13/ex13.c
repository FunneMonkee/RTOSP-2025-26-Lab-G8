#include <dispatch/dispatch.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

dispatch_semaphore_t read_sem, write_sem;
pthread_t tid;
pthread_t writers[3], readers[3];
int readercount = 0;

void *reader(void *param) {
  // macos
  dispatch_semaphore_wait(read_sem, DISPATCH_TIME_FOREVER);
  // sem_wait(&read_sem);

  readercount++;
  if (readercount == 1)
    dispatch_semaphore_wait(write_sem, DISPATCH_TIME_FOREVER);
  // sem_wait(&write_sem);

  // macos
  dispatch_semaphore_signal(read_sem);

  // sem_post(&read_sem);
  printf("%d reading\n", readercount);
  usleep(3);

  dispatch_semaphore_wait(read_sem, DISPATCH_TIME_FOREVER);
  readercount--;
  if (readercount == 0) {
    dispatch_semaphore_signal(write_sem);
    // sem_post(&write_sem);
  }
  dispatch_semaphore_signal(read_sem);
  // sem_post(&read_sem);
  printf("%d finished reading\n", readercount + 1);
  return NULL;
}

void *writer(void *param) {
  printf("trying to write\n");
  dispatch_semaphore_wait(write_sem, DISPATCH_TIME_FOREVER);
  // sem_wait(&write_sem);
  printf("writing\n");
  dispatch_semaphore_signal(write_sem);
  // sem_post(&write_sem);
  printf("finished writing\n");
  return NULL;
}

int main() {
  int i;
  // macos
  read_sem = dispatch_semaphore_create(1);
  write_sem = dispatch_semaphore_create(1);

  // sem_open(&read_sem, 0, 1);
  // sem_open(&write_sem, 0, 1);

  for (i = 0; i < 3; i++) {
    pthread_create(&writers[i], NULL, reader, NULL);
    pthread_create(&readers[i], NULL, writer, NULL);
  }
  for (i = 0; i < 3; i++) {
    pthread_join(writers[i], NULL);
    pthread_join(readers[i], NULL);
  }
}
