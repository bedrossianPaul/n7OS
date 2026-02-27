#include "terminal.h"
#include <string.h>


void terminal() {
    printf("\f"); // Clear the screen
    sleep(500);
    while (1) {
        printf("\n> ");
        char input[256];
        gets(input); // Lire une ligne de l'entrée standard (clavier)
        printf("\n");
        find_cmd(input);
    }
}

void find_cmd(const char *cmd) {
    // Cette fonction peut être utilisée pour implémenter la logique de recherche de commandes
    // Par exemple, vous pouvez comparer 'cmd' avec une liste de commandes disponibles et exécuter la commande correspondante
    if (strcmp(cmd, "help") == 0) {
        printf("-- Available commands --\n");
        printf("clear - Clear the terminal\n");
        printf("echo - Echo the input back to the terminal\n");
        printf("shutdown - Shutdown the computer\n");
        printf("help - Show this help message\n");
        // Ajoutez d'autres commandes ici
    } else if (strcmp(cmd, "clear") == 0) {
        printf("\f"); // Clear the screen
    } else if (strncmp(cmd, "echo ", 5) == 0) {
        printf("%s\n", cmd + 5); // Echo the text after "echo "
    } else if (strcmp(cmd, "shutdown") == 0) {
        for (int i = 0; i < 5; i++) {
            printf("\rShutting down in %d seconds...", 5 - i);
            sleep(1000); // Attendre 1 seconde
        }
        shutdown(1); // Appeler la fonction de shutdown du système
    } else {
        printf("Unknown command: %s\n", cmd);
    }
}