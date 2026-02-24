#include <n7OS/proc.h>
#include <stddef.h>
#include <n7OS/paging.h>
#include <n7OS/idle.h>
#include <n7OS/cpu.h>
#include <stdio.h>
#include <malloc.h>

proc_t procs_table[MAX_PROC]; // (Ready Process Table) Tableau des processus prêts à être exécutés

extern void ctx_sw(uint32_t* ctx_old, uint32_t* ctw_new); // Fonction d'assemblage pour le changement de contexte

/**
 * @brief Initialise la table des processus en marquant tous les processus comme terminés
 */
void init_proc_table(){
    for (int i = 0; i < MAX_PROC; i++){
        procs_table[i].state = TERMINATED; // Initialiser tous les processus comme terminés
    }
    pid_t pid = spawn_proc("Idle", idle); // Créer le processus idle pour qu'il puisse être exécuté par le scheduler
    procs_table[pid - 1].state = RUNNING; // Mettre le processus idle en état RUNNING pour qu'il puisse être exécuté
}

/**
 * @brief Trouve un PID disponible en parcourant la table des processus
 * @return Un PID disponible, ou 0 si aucun PID n'est disponible
 */
pid_t get_pid(){
    for (int i = 0; i < MAX_PROC; i++){
        if (procs_table[i].state == TERMINATED){
            return i + 1; // Les PID commencent à 1
        }
    }
    return 0; // Aucun PID disponible
}

/**
 * @brief Récupère le PID du processus actuellement en cours d'exécution
 * @return Le PID du processus en cours d'exécution, ou 0 si aucun processus
 */
pid_t get_current_pid(){
    for (int i = 0; i < MAX_PROC; i++){
        if (procs_table[i].state == RUNNING){
            return procs_table[i].pid; // Retourner le PID du processus en cours d'exécution
        }
    }
    return 0; // Aucun processus en cours d'exécution
}

/**
 * @brief Crée un nouveau processus et l'ajoute à la table des processus
 * @param name Le nom du processus
 * @param fn La fonction que le processus doit exécuter
 * @return Le PID du processus créé, ou 0 en cas d'échec
 */
pid_t spawn_proc(char *name, void* fn){
    pid_t pid = get_pid(); // Récupérer un PID disponible

    procs_table[pid-1].pid = pid;
    procs_table[pid-1].name = name;
    procs_table[pid-1].state = READY;
    procs_table[pid-1].stack = (uint32_t*)malloc(sizeof(uint32_t) * STACK_SIZE); // Utiliser l'adresse de la pile allouée

    for (int i = 0; i<5; i++){
        procs_table[pid-1].regs[i] = 0; // Initialisation des registres à 0
    }

    procs_table[pid-1].stack[STACK_SIZE - 1] = (uint32_t)fn;
	procs_table[pid-1].regs[1] = (uint32_t)&procs_table[pid-1].stack[STACK_SIZE - 1];

    return pid;
}

/**
 * @brief Termine un processus en libérant ses ressources et en le marquant comme terminé
 * @param pid Le PID du processus à terminer
 */
void terminate_proc(pid_t pid){
    // Libérer les ressources du processus
    if (procs_table[pid - 1].state != TERMINATED) {
        procs_table[pid - 1].state = TERMINATED; // Marquer le processus comme terminé
        scheduler(); // Appeler le scheduler pour choisir un autre processus à exécuter
    }
}

// /**
//  * @brief Bloque un processus en changeant son état et en appelant le scheduler
//  * @param pid Le PID du processus à bloquer
//  */
// void block_proc(pid_t pid){
//     if (procs_table[pid - 1].state == READY || procs_table[pid - 1].state == RUNNING) {
//         procs_table[pid - 1].state = BLOCKED; // Marquer le processus comme bloqué
//         scheduler(); // Appeler le scheduler pour choisir un autre processus à exécuter
//     }
// }

// /**
//  * @brief Débloque un processus en changeant son état de bloqué à prêt
//  * @param pid Le PID du processus à débloquer
//  */
// void unblock_proc(pid_t pid){
//     if (procs_table[pid - 1].state == BLOCKED) {
//         procs_table[pid - 1].state = READY; // Marquer le processus comme prêt
//         scheduler(); // Appeler le scheduler pour choisir un processus à exécuter
//     }
// }

/**
 * @brief Arrête un processus en changeant son état de running à prêt
 * @param pid Le PID du processus à arrêter
 */
void stop_proc(pid_t pid){
    if (procs_table[pid - 1].state == RUNNING) {
        procs_table[pid - 1].state = READY; // Marquer le processus comme prêt
    }
}


/**
 * @brief Scheduler tourniquet
 */
void scheduler(){
    pid_t current_pid = get_current_pid();
    int start_idx = 0;

    // Chercher le prochain processus prêt à partir du suivant du courant (tourniquet)
    start_idx = current_pid; // current_pid commence à 1
    for (int i = 0; i < MAX_PROC; i++) {
        int idx = (start_idx + i) % MAX_PROC;
        if (procs_table[idx].state == READY) {
            stop_proc(current_pid); // Met le courant en READY et appelle scheduler, mais on évite la récursion car on sort juste après
            // Démarre le nouveau processus
            procs_table[idx].state = RUNNING;
            sti(); //Reactiver les interruptions !???
            ctx_sw(procs_table[current_pid - 1].regs, procs_table[idx].regs);
            break;
        }
    }
    
}