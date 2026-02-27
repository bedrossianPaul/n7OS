#include <n7OS/keyboard.h>
#include <n7OS/cpu.h>
#include <stdio.h>

char buffer[256];
int buffer_head = 0; // Prochain caractère à lire
int buffer_tail = 0; // Prochain emplacement d'écriture

void clear_buffer() {
    buffer_head = 0;
    buffer_tail = 0;
}

void init_keyboard() {
    // Clean du buffer
    clear_buffer();
    //Démasquage de l'interruption du clavier (IRQ1)
    outb(inb(0x21)&~(1<<1), 0x21);

}

void mask_keyboard() {
    // Masquage de l'interruption du clavier (IRQ1)
    outb(inb(0x21)|(1<<1), 0x21);
    clear_buffer();
}

void keyboard_handler() {
    outb(0x20, 0x20); // Envoyer un signal EOI (End of Interrupt) au PIC pour indiquer que l'interruption a été traitée
    // Lire le scancode du clavier
    uint8_t scancode = inb(0x60);

    // Vérifier si la touche est relâchée
    if (IS_KEY_RELEASED(scancode)) {
        return; // Ignorer les touches relâchées
    }
    // Ajouter le caractère correspondant au buffer (FIFO)
    int next_tail = (buffer_tail + 1) % sizeof(buffer);
    if (next_tail != buffer_head) { // Buffer non plein
        buffer[buffer_tail] = scancode_map[scancode];
        buffer_tail = next_tail;
    }
}

int is_buffer_empty() {
    return buffer_head == buffer_tail;
}

char kgetch() {
    if (is_buffer_empty())
        return -1; // Pour indiquer a scanf que le buffer est vide
    char c = buffer[buffer_head];
    buffer_head = (buffer_head + 1) % sizeof(buffer);
    return c;
}