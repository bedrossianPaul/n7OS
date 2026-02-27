#include <stdio.h>
#include <unistd.h>

void processus1() {
  printf("Hello, world from P1\n");
  sleep(2000); // Sleep pendant 1000 ms (1 seconde)
  printf("PID: %d\n", getpid());
  char buff[255];
  printf("Enter some input: ");
  gets(buff); // Lire une ligne de l'entrée standard (clavier)
  printf("\nYou entered: %s\n", buff);
  printf("Everything is initialized...\n");
  printf("Press enter to lauch the console...\n");
  char c = getchar(); // Attendre que l'utilisateur appuie sur la touche "Entrée"
  while( c != '\n' && c != '\r'){
    c = getchar();
  }; // Attendre que l'utilisateur appuie sur la touche "Entrée"
  printf("Launching console...\n");
  // Ici, vous pouvez ajouter le code pour lancer la console ou faire d'autres actions nécessaires
  exit(); // Terminer le processus

}
