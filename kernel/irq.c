#include <inttypes.h>
#include <n7OS/irq.h>


void init_irq_entry(int irq_num, uint32_t addr) {
    idt_entry_t *entry = (idt_entry_t *)&idt[irq_num]; // Récupère l'entrée de l'IDT correspondant au numéro d'interruption
    entry->offset_inf = addr & 0xFFFF; // Met à jour la partie offset inférieur de l'entrée de l'IDT avec les 16 bits de poids faible de l'adresse du gestionnaire d'interruption
    entry->offset_sup = addr >> 16; // Met à jour la partie offset supérieur de l'entrée de l'IDT avec les 16 bits de poids fort de l'adresse du gestionnaire d'interruption
    entry->sel_segment = KERNEL_CS; // Met à jour le sélecteur de segment de l'entrée de l'IDT avec le sélecteur de segment du code du noyau
    entry->zero = 0; // Met à 0 le champ réservé de l'entrée de l'IDT
    entry->type_attr = PRESENT | DPL_HIGH | INT_GATE | TYPE_INT32_GATE; // Met à jour le champ type et attributs de l'entrée de l'IDT pour indiquer que c'est une interruption présente, avec un DPL de 0 et un type d'interruption (gate) de 32 bits
}
