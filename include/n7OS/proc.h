#ifndef __PROC_H__
#define __PROC_H__

#include <inttypes.h>
#include <n7OS/mem.h>

#define MAX_PROC 255 // Nombre maximum de processus que le système peut gérer
#define STACK_REGION_TOP 0xBFFFFFFF
#define STACK_SIZE PAGE_SIZE // Taille de la pile de chaque processus (1 page = 4096 octets)

typedef uint32_t  pid_t; // Type pour les identifiants de processus

typedef enum {
    READY,
    BLOCKED,
    RUNNING,
    TERMINATED
} proc_state_t; // Enumération pour les états possibles d'un processus

typedef struct {
    pid_t pid; // Identifiant du processus
    proc_state_t state; // Etat du processus
    char* name; // Nom du processus
    uint32_t* stack; // Pile du processus
    int sleeping_end_time; // Temps de réveil pour les processus endormis
    uint32_t regs[5]; // Registres sauvegardés pour le changement de contexte (EIP, ESP, EBP, EFLAGS, etc.)

} proc_t;

void init_proc_table(); // Initialise la table des processus
pid_t get_pid(); // Récupère un PID disponible
pid_t get_current_pid(); // Récupère le PID du processus en cours d'exécution
int get_ready_count(); // Récupère le nombre de processus prêts à être exécutés
pid_t spawn_proc(char *name, void* fn); // Crée un nouveau processus
void terminate_proc(pid_t pid); // Termine un processus
// void block_proc(pid_t pid); // Bloque un processus
// void unblock_proc(pid_t pid); // Débloque un processus
void stop_proc(pid_t pid); // Arrête un processus (le met en état READY)
void sleep_proc(int duration); // Met un processus en sommeil pendant une durée donnée (en ms)
void scheduler(pid_t pid); // Scheduler tourniquet
void ps(); // Affiche la liste des processus en cours d'exécution

#endif