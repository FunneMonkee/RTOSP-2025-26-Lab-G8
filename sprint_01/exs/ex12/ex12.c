#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

typedef struct Node {
  int data;
  struct Node *next;
} Node;

Node *head = NULL;
Node *tail = NULL;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t not_empty = PTHREAD_COND_INITIALIZER;

void produce(int value) {
  Node *newNode = (Node *)malloc(sizeof(Node));
  newNode->data = value;
  newNode->next = NULL;

  pthread_mutex_lock(&mutex);

  if (tail == NULL) {
    head = tail = newNode;
  } else {
    tail->next = newNode;
    tail = newNode;
  }

  printf("Produced: %d\n", value);

  pthread_cond_signal(&not_empty); // single thread
  // all threads (only one so it can also be used here??)
  // pthread_cond_broadcast(&not_empty);
  pthread_mutex_unlock(&mutex);
}

int consume() {
  pthread_mutex_lock(&mutex);

  while (head == NULL) {
    pthread_cond_wait(&not_empty, &mutex);
  }

  Node *temp = head;
  int value = temp->data;

  head = head->next;
  if (head == NULL)
    tail = NULL;

  free(temp);

  pthread_mutex_unlock(&mutex);

  return value;
}

void *producer(void *arg) {
  for (int i = 1; i <= 10; i++) {
    produce(i);
    sleep(1);
  }
  return NULL;
}

void *consumer(void *arg) {
  for (int i = 1; i <= 10; i++) {
    int value = consume();
    printf("Consumed: %d\n", value);
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
