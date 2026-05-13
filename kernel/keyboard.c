#include <n7OS/keyboard.h>
#include <n7OS/cpu.h>
#include <stdio.h>

int buffer[256];
int buffer_head = 0; // Prochain caractère à lire
int buffer_tail = 0; // Prochain emplacement d'écriture
static int shift_down = 0;
static int e0_prefix = 0;

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

    if (scancode == 0xE0) {
        e0_prefix = 1;
        return;
    }

    if (e0_prefix) {
        e0_prefix = 0;
        if (IS_KEY_RELEASED(scancode)) {
            return;
        }
        switch (scancode) {
            case 0x48:
                buffer[buffer_tail] = KEY_UP;
                buffer_tail = (buffer_tail + 1) % sizeof(buffer);
                return;
            case 0x50:
                buffer[buffer_tail] = KEY_DOWN;
                buffer_tail = (buffer_tail + 1) % sizeof(buffer);
                return;
            case 0x4B:
                buffer[buffer_tail] = KEY_LEFT;
                buffer_tail = (buffer_tail + 1) % sizeof(buffer);
                return;
            case 0x4D:
                buffer[buffer_tail] = KEY_RIGHT;
                buffer_tail = (buffer_tail + 1) % sizeof(buffer);
                return;
            default:
                return;
        }
    }

    if (scancode == SHIFT_PRESSED || scancode == 0x36) {
        shift_down = 1;
        return;
    }

    if (scancode == SHIFT_RELEASED || scancode == 0xB6) {
        shift_down = 0;
        return;
    }

    // Vérifier si la touche est relâchée
    if (IS_KEY_RELEASED(scancode)) {
        return; // Ignorer les touches relâchées
    }
    // Ajouter le caractère correspondant au buffer (FIFO)
    int next_tail = (buffer_tail + 1) % (int)(sizeof(buffer) / sizeof(buffer[0]));
    if (next_tail != buffer_head) { // Buffer non plein
        buffer[buffer_tail] = shift_down ? scancode_map_shift[scancode] : scancode_map[scancode];
        buffer_tail = next_tail;
    }
}

int is_buffer_empty() {
    return buffer_head == buffer_tail;
}

int kgetch(void) {
    if (is_buffer_empty())
        return -1; // Pour indiquer a scanf que le buffer est vide
    int c = buffer[buffer_head];
    buffer_head = (buffer_head + 1) % (int)(sizeof(buffer) / sizeof(buffer[0]));
    return c;
}