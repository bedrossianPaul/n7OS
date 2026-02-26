#include <n7OS/banner.h>
#include <n7OS/console.h>
#include <n7OS/timer.h>
#include <n7OS/proc.h>
#include <stdio.h>

void display_banner(int update_left, int update_right) {
    // Sauvegarder la position actuelle du curseur
    int saved_col, saved_row;
    get_cursor_position(&saved_col, &saved_row);

    if (update_left) {
        // Affichage à gauche : N7OS [nb de processus READY] | [current_proc]
        int ready_count = get_ready_count();
        int current_pid = get_current_pid();
        // Effacer la zone de gauche (30 caractères)
        for (int i = 0; i < 30; i++) {
            cursor_move(i, 0);
            console_putchar(' ');
        }
        char left_buf[64];
        snprintf(left_buf, sizeof(left_buf), "N7OS [%d] | %d", ready_count, current_pid);
        for (int i = 0; left_buf[i] != '\0'; i++) {
            cursor_move(i, 0);
            console_putchar(left_buf[i]);
        }
    }

    if (update_right) {
        // Affichage à droite : heure
        uint32_t seconds, minutes, hours;
        get_time(&seconds, &minutes, &hours);
        int col = 65;
        int row = 0;
        // Effacer la zone (15 caractères)
        for (int i = 0; i < 15; i++) {
            cursor_move(col + i, row);
            console_putchar(' ');
        }
        char buf[16];
        snprintf(buf, sizeof(buf), " %02u:%02u:%02u", hours, minutes, seconds);
        for (int i = 0; buf[i] != '\0'; i++) {
            cursor_move(col + i, row);
            console_putchar(buf[i]);
        }
    }

    // Restaurer la position du curseur
    cursor_move(saved_col, saved_row);
}