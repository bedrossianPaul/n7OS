#include <n7OS/sys.h>
#include <n7OS/syscall_defs.h>
#include <n7OS/console.h>
#include <n7OS/irq.h>
#include <unistd.h>
#include <n7OS/cpu.h>
#include <stdio.h>
#include <n7OS/proc.h>
#include <n7OS/keyboard.h>

extern void handler_syscall();

void init_syscall() {
  // ajout de la fonction de traitement de l'appel systeme
  add_syscall(NR_example, sys_example);
  add_syscall(NR_shutdown, sys_shutdown);
  add_syscall(NR_write, sys_write);
  add_syscall(NR_sleep, sys_sleep);
  add_syscall(NR_getpid, sys_getpid);
  add_syscall(NR_exit, sys_exit);
  add_syscall(NR_read, sys_read);
  add_syscall(NR_fork, sys_fork);
  // initialisation de l'IT soft qui gère les appels systeme
  init_irq_entry(0x80, (uint32_t) handler_syscall);
}

// code de la fonction de traitement de l'appel systeme example
int sys_example() {
  // on ne fait que retourner 1
  return 1;
}

int sys_shutdown(int n){
  if (n == 1) {
    printf("Computer shutted down by syscall\n");
    outw(0x2000, 0x604); // shutdown QEMU
    return -1;
  } else {
    return 0;
  }
}

int sys_write(const char *str, int len) {
  // Affiche la chaîne de caractères sur la console
  console_putbytes(str, len);
  return 0; // Retourne 0 pour indiquer que l'écriture a réussi
}

int sys_sleep(int ms) {
  sleep_proc(ms); // Appeler la fonction de sommeil du processus pour mettre le processus en état de sommeil
  return 0; // Retourne 0 pour indiquer que le sleep a réussi
}

int sys_getpid() {
  pid_t pid = get_current_pid();
  return pid;
}

int sys_exit() {
  pid_t pid = get_current_pid();
  terminate_proc(pid);
  return 0; // Retourne 0 pour indiquer que le processus a été terminé avec succès
}

int sys_read(char *buffer, int size) {
  sti(); // Activer les interruptions pour permettre la lecture du clavier
  init_keyboard();
  for (int i = 0; i < size; i++) {
    // Attendre qu'au moins un caractère soit disponible (attente active)
    while (is_buffer_empty()) {
      // attente active
    }
    buffer[i] = kgetch(); // Lire le caractère dès qu'il est dispo
  }
  mask_keyboard(); // Masquer le clavier après la lecture
  return 0; // Retourne 0 pour indiquer que la lecture a réussi
}

int sys_fork(char *name, void* fn) {
  pid_t pid = get_current_pid();
  int new_pid = spawn_proc(name, fn);
  return new_pid; // Retourne le PID du nouveau processus créé
}