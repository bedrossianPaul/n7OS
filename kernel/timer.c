#include <n7OS/timer.h>
#include <n7OS/cpu.h>
#include <n7OS/banner.h>
#include <n7OS/console.h>
#include <stdio.h>
#include <n7OS/proc.h>

volatile uint32_t timer_ticks = 0; // Compteur de ticks du timer

void init_timer() {
    outb(0x43, 0x36); // Commande pour le mode de fonctionnement du timer (mode 3 - channel 0)
    outb(CLK_FRQ & 0xFF, 0x40); // Partie basse du diviseur
    outb((CLK_FRQ >> 8) & 0xFF, 0x40); // Partie haute du diviseur

    // PIC configuration
	outb(inb(0x21) & 0xfe, 0x21); // Unmask IRQ0 (timer)
}

void timer_handler() {
    outb(0x20, 0x20); // Envoyer un signal EOI (End of Interrupt) au PIC pour indiquer que l'interruption a été traitée
    timer_ticks++; // Incrémenter le compteur de ticks à chaque interruption du timer

    display_banner(timer_ticks % 100 == 0, timer_ticks % 1000 == 0); // Mettre à jour le banner tous les 100 ticks (10 fois par seconde)
    if (timer_ticks % 100 == 0) { // Appeler le scheduler tous les 100 ticks (10 fois par seconde) pour permettre le multitâche
        scheduler(get_current_pid());
    }

}

uint32_t get_ticks() {
    return timer_ticks; // Retourner le nombre de ticks écoulés depuis l'initialisation du timer
}

void get_time(uint32_t *seconds, uint32_t *minutes, uint32_t *hours) {
    uint32_t total_seconds = timer_ticks / TIMER_FRQ; // Calculer le nombre total de secondes écoulées
    *seconds = total_seconds % 60; // Calculer les secondes
    *minutes = (total_seconds / 60) % 60; // Calculer les minutes
    *hours = (total_seconds / 3600) % 24; // Calculer les heures
}