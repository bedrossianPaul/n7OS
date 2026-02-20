#ifndef ___TIMER_H___
#define ___TIMER_H___

#include <inttypes.h>

#define F_OSC 0x1234BD // Fréquence d'oscillation du timer en Hz (1.19 MHz)
#define TIMER_FRQ 100 // Fréquence de l'interruption timer en Hz (100 Hz)
#define CLK_FRQ (F_OSC / TIMER_FRQ)

void init_timer();
void timer_handler();

uint32_t get_ticks();

void get_time(uint32_t *seconds, uint32_t *minutes, uint32_t *hours);
void display_time();

#endif