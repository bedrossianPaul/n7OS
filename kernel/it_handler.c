#include<n7OS/it_handler.h>
#include <n7OS/irq.h>
#include <n7OS/keyboard.h>
#include <stdio.h>

extern void handler_IT_50();
extern void handler_IT_timer();
extern void handler_IT_keyboard();

void init_it() {
    init_irq_entry(50, (uint32_t)handler_IT_50); // On initialise l'entrée de l'IDT correspondant à l'interruption 50 avec l'adresse du gestionnaire d'interruption handler_IT_50
    init_irq_entry(32, (uint32_t)handler_IT_timer); // On initialise l'entrée de l'IDT correspondant à l'interruption 32 avec l'adresse du gestionnaire d'interruption handler_IT_timer
    init_irq_entry(0x21, (uint32_t)handler_IT_keyboard); // On initialise l'entrée de l'IDT correspondant à l'interruption 33 avec l'adresse du gestionnaire d'interruption handler_IT_keyboard
}

void handler_IT_50_c() {
    printf(" Test IT : OK\n");
}