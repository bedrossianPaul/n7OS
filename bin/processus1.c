#include <stdio.h>
#include <unistd.h>

void processus1() {
  printf("Hello, world from P1\n");
  sleep(1000); // Sleep pendant 1000 ms (1 seconde)
  printf("PID: %d\n", getpid());
  printf("P1 is exiting...\n");
  exit();
  for (;;);
}
