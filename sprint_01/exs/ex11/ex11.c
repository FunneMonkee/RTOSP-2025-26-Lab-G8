#include "stdio.h"
#include "stdlib.h"
#include "time.h"
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

int board[3][3];
int counter = 0;
pthread_mutex_t lock =
    PTHREAD_MUTEX_INITIALIZER; // lock to make threads execute in order

struct player {
  int number;
};

int checkWin(int player) {
  for (int i = 0; i < 3; i++) {
    if (board[i][0] == player && board[i][1] == player &&
        board[i][2] == player) {
      return 1;
    }
  }

  for (int i = 0; i < 3; i++) {
    if (board[0][i] == player && board[1][i] == player &&
        board[2][i] == player) {
      return 1;
    }
  }

  if (board[0][0] == player && board[1][1] == player && board[2][2] == player) {
    return 1;
  }
  if (board[0][2] == player && board[1][1] == player && board[2][0] == player) {
    return 1;
  }

  return 0;
}

void *play(void *arg) {
  struct player *player = arg;
  for (;;) {
    if (counter == 9 || checkWin(1) == 1 || checkWin(2) == 1)
      break;
    int played = 0;
    pthread_mutex_lock(&lock);
    while (played == 0) {
      int i = rand() % 3;
      int j = rand() % 3;
      if (board[i][j] == 0) {
        board[i][j] = player->number;
        played = 1;
        counter++;
        for (i = 0; i < 3; i++) {
          for (j = 0; j < 3; j++) {
            printf("|%d", board[i][j]);
          }
          printf("|\n");
        }
        printf("----------\n");
        pthread_mutex_unlock(&lock);
      }
    }
    sleep(1);
  }

  return NULL;
}

int main() {
  void *res;
  int s, i, j;
  pthread_t threads[2];
  pthread_mutex_init(&lock, NULL);
  srand(time(NULL));
  struct player player1;
  struct player player2;

  player1.number = 1;
  s = pthread_create(&threads[0], NULL, &play, &player1);
  if (s != 0)
    printf("error on thread 1 create\n");

  player2.number = 2;
  s = pthread_create(&threads[1], NULL, &play, &player2);
  if (s != 0)
    printf("error on thread 2 create\n");

  for (i = 0; i < 2; i++) {
    int s = pthread_join(threads[i], &res);
    if (s != 0)
      printf("error on thread join, thread %d\n", i);
  }

  for (i = 0; i < 3; i++) {
    for (j = 0; j < 3; j++) {
      printf("|%d", board[i][j]);
    }
    printf("|\n");
  }

  pthread_mutex_destroy(&lock);
}
