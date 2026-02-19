#include <n7OS/cpu.h>
#include <inttypes.h>
#include <stdio.h>
#include <n7OS/console.h>
#include <n7OS/paging.h>
#include <n7OS/mem.h>
#include <unistd.h>
#include <n7OS/it_handler.h>

void kernel_start(void)
{
    init_console();
    printf("\fN7 OS project initialisation...\n\n");

    // initialisation de la pagination
    initialise_paging();
    uint32_t *ptr = (uint32_t*)0x4000;
    *ptr = 0xCAFEBABE;
    if (*ptr == 0xCAFEBABE)
        printf(" Test Paging : OK\n"); 
    else
        printf(" Test Paging : FAIL!\n");

    // initialisation des interruptions
    init_it();
    sti();
    __asm__("int $50"); // On déclenche l'interruption 50 pour tester le gestionnaire d'interruption associé

    // on ne doit jamais sortir de kernel_start
    while (1) {
        // cette fonction arrete le processeur
        hlt();
    }
}
