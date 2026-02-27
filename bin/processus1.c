#include <stdio.h>
#include <unistd.h>

void processus1() {
  printf("Hello, world from P1\n");
  sleep(2000); // Sleep pendant 1000 ms (1 seconde)
  printf("PID: %d\n", getpid());
  char buff[10];
  printf("Enter some input: ");
  read(buff, 2); // Lire l'entrée de l'utilisateur dans le buffer
  printf("\nYou entered: %s\n", buff);
  printf("P1 is idling...\n");
  // exit();
  for (;;);
}
