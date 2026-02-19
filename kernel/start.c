#include <n7OS/cpu.h>
#include <inttypes.h>
#include <stdio.h>
#include <n7OS/console.h>
#include <n7OS/paging.h>
#include <n7OS/mem.h>
#include <unistd.h>
#include <n7OS/it_handler.h>
#include <n7OS/sys.h>

void kernel_start(void)
{
    // initialisation de la console
    init_console();
    // initialisation de la pagination
    initialise_paging();
    // initialisation des interruptions
    init_it();
    // initialisation des appels systeme
    init_syscall();
    sti();

    // Tests
    printf("\fN7 OS project initialisation...\n\n");

    uint32_t *ptr = (uint32_t*)0x4000;
    *ptr = 0xCAFEBABE;
    if (*ptr == 0xCAFEBABE)
        printf(" Test Paging : OK\n"); 
    else
        printf(" Test Paging : FAIL!\n");


    __asm__("int $50"); // On déclenche l'interruption 50 pour tester le gestionnaire d'interruption associé

    if (example() == 1){ // On teste l'appel système example, qui doit retourner 1
        printf(" Test SysCall Example: OK\n");
    } else {
        printf(" Test SysCall Example: FAIL!\n");
    }

    if (shutdown(0) == 0) { // On teste l'appel système shutdown, qui doit retourner 0 si l'argument n est différent de 1
        printf(" Test SysCall Shutdown : OK\n");
    } else {
        printf(" Test SysCall Shutdown : FAIL!\n");
    }

    // on ne doit jamais sortir de kernel_start
    while (1) {
        // cette fonction arrete le processeur
        hlt();
    }
}
