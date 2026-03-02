#include "stdio.h"
#include "sys/types.h"
#include "unistd.h"

int main(void) {
  int x = 0;
  pid_t p = fork(); /*pid_t: sys/types.h; fork(): unistd.h*/
  if (p == 0) {
    x = x + 2;
    printf("Step 1. x = %d\n", x);
  } else {
    x = x - 2;
    printf("Step 2. x = %d\n", x);
  }
  printf("Step 3. %d; x = %d\n", p, x);
}
