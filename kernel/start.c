#include <n7OS/cpu.h>
#include <inttypes.h>
#include <stdio.h>
#include <n7OS/console.h>
#include <n7OS/paging.h>
#include <n7OS/mem.h>

void kernel_start(void)
{
    init_console();
    printf("\fN7 OS project initialisation...!\n\n");

    // Test de la pagination
    initialise_paging();
    uint32_t *ptr = (uint32_t*)0x4000;
    *ptr = 0xCAFEBABE;
    if (*ptr == 0xCAFEBABE)
        printf(" -> Paging OK!\n"); 
    else
        printf(" -> Paging FAIL!\n");
    // lancement des interruptions
    sti();

    // on ne doit jamais sortir de kernel_start
    while (1) {
        // cette fonction arrete le processeur
        hlt();
    }
}
