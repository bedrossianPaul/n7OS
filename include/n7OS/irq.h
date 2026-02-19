#ifndef __IRQ_H__
#define __IRQ_H__

#include <inttypes.h>
#include <n7OS/segment.h>
#include <n7OS/processor_structs.h>

/*
Une entrée dans l'IDT est sur 64 bits

Partie poids forts
Bit :     | 31              16 | 15 | 14 13 | 12 | 11 10 9 8 | 7 6 5 | 4 3 2 1 0 |
Contenu : | offset supérieur   | P  | DPL   | S  | GateType  | 0 0 0 |  réservé  |

Partie poids faible
Bit :     | 31              16 | 15              0 |
Contenu : |sélecteur de segment| offset inférieur  |
*/

#define PRESENT 0b10000000 // Présent en mémoire: 1 si l'entrée est valide et peut être utilisée, 0 sinon
#define DPL_HIGH 0b00000000 // Niveau de droit: 0 (kernel)
#define INT_GATE 0b00000000 // Type d'entrée: 0 pour une interruption (gate d'interruption), 1 pour une trap (gate de trap)
#define TYPE_INT32_GATE 0b00001110 // Type d'entrée: 0b1110 pour une gate d'interruption 32 bits

typedef struct {
  uint16_t offset_inf;
  uint16_t sel_segment;
  uint8_t zero;
  uint8_t type_attr;
  uint16_t offset_sup;
} idt_entry_t;

void init_irq_entry(int irq_num, uint32_t addr);

#endif
