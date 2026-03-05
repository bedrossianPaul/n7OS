#include <stdio.h>
#include <unistd.h>
#include "terminal.h"

void processus1() {
  printf("\n");
  printf("=====================================\n");
  printf("Processus 1\n");
  printf("======================================\n");
  printf("PID: %d\n\n", getpid());
  printf("Sleeping test...\n\n");
  sleep(1000); // Sleep pendant 1000 ms (1 seconde)
  printf(" Test sleeping : OK\n\n\n");
  printf("Press enter to lauch the console...\n");
  char c = getchar(); // Attendre que l'utilisateur appuie sur la touche "Entrée"
  while( c != '\n' && c != '\r'){
    c = getchar();
  }; // Attendre que l'utilisateur appuie sur la touche "Entrée"
  fork("Terminal", &terminal); // Lancer le processus terminal
  exit(); // Terminer le processus

}
