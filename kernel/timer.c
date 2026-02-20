#include <n7OS/timer.h>
#include <n7OS/cpu.h>

#include <n7OS/console.h>
#include <stdio.h>

volatile uint32_t timer_ticks = 0; // Compteur de ticks du timer

void init_timer() {
    outb(0x43, 0x36); // Commande pour le mode de fonctionnement du timer (mode 3 - channel 0)
    outb(CLK_FRQ & 0xFF, 0x40); // Partie basse du diviseur
    outb((CLK_FRQ >> 8) & 0xFF, 0x40); // Partie haute du diviseur

    // PIC configuration
	outb(inb(0x21) & 0xfe, 0x21); // Unmask IRQ0 (timer)
}

void timer_handler() {
    timer_ticks++; // Incrémenter le compteur de ticks à chaque interruption du timer

    // Afficher l'heure à chaque tick
    display_time();
}

uint32_t get_ticks() {
    return timer_ticks; // Retourner le nombre de ticks écoulés depuis l'initialisation du timer
}

void get_time(uint32_t *seconds, uint32_t *minutes, uint32_t *hours) {
    outb(0x20, 0x20); // Envoyer un signal EOI (End of Interrupt) au PIC pour indiquer que l'interruption a été traitée
    uint32_t total_seconds = timer_ticks / TIMER_FRQ; // Calculer le nombre total de secondes écoulées
    *seconds = total_seconds % 60; // Calculer les secondes
    *minutes = (total_seconds / 60) % 60; // Calculer les minutes
    *hours = (total_seconds / 3600) % 24; // Calculer les heures
}

// Affiche l'heure à un endroit fixe (fin de la première ligne)
void display_time() {
    uint32_t seconds, minutes, hours;
    get_time(&seconds, &minutes, &hours);
    // Positionner le curseur en haut à droite (colonne 65, ligne 0 par exemple)
    int col = 65;
    int row = 0;
    // Effacer la zone (10 caractères)
    for (int i = 0; i < 15; i++) {
        cursor_move(col + i, row);
        console_putchar(' ');
    }
    // Réafficher l'heure
    char buf[16];
    snprintf(buf, sizeof(buf), " %02u:%02u:%02u", hours, minutes, seconds);
    for (int i = 0; buf[i] != '\0'; i++) {
        cursor_move(col + i, row);
        console_putchar(buf[i]);
    }
}