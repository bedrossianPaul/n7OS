#include <n7OS/console.h>
#include <n7OS/cpu.h>

uint16_t *scr_tab;
uint16_t cursor_pos;

void init_console() {
    scr_tab= (uint16_t *) SCREEN_ADDR;
}   


void cursor_move(int x, int y) {
    cursor_pos = y * VGA_WIDTH + x;
    outb(CMD_HIGH, PORT_CMD);
    outb((cursor_pos>>8) & 0xFF, PORT_DATA);
    outb(CMD_LOW, PORT_CMD);
    outb(cursor_pos & 0xFF, PORT_DATA);
}

void clear_console() {
    for (int i=0; i<VGA_WIDTH*VGA_HEIGHT; i++) {
        scr_tab[i]= CHAR_COLOR<<8|0;
    }
    cursor_pos = 0;
    cursor_move(0, 0);
}
void console_putchar(const char c) {
    if (c > 31 && c < 127) { // printable character
        scr_tab[cursor_pos]= CHAR_COLOR<<8|c;
        cursor_pos++;
        cursor_move(cursor_pos % VGA_WIDTH, cursor_pos / VGA_WIDTH);
    } else if (c == 10) { // newline
        cursor_pos += VGA_WIDTH - (cursor_pos % VGA_WIDTH);
        cursor_move(cursor_pos % VGA_WIDTH, cursor_pos / VGA_WIDTH);
    } else if (c == 8) { // backspace
        if (cursor_pos > 0) {
            cursor_pos--;
            scr_tab[cursor_pos]= CHAR_COLOR<<8|0;
            cursor_move(cursor_pos % VGA_WIDTH, cursor_pos / VGA_WIDTH);   
        }
    } else if (c == 9) { // tab
        int spaces = 4 - (cursor_pos % 4);
        for (int i=0; i<spaces; i++) {
            console_putchar(' ');
        }
    } else if (c == 13) { // carriage return
        cursor_move(0, cursor_pos / VGA_WIDTH);
    } else if (c == 12) { // form feed
        clear_console();
    }
}

void console_putbytes(const char *s, int len) {
    for (int i= 0; i<len; i++) {
        console_putchar(s[i]);
    }
}