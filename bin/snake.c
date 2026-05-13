#include "snake.h"
#include <stdlib.h>
#include <string.h>
#include <n7OS/keyboard.h>
#include <n7OS/console.h>


static unsigned int seed = 1;

int simple_rand() {
    seed = (seed * 1103515245 + 12345) & 0x7fffffff;
    return seed;
}

void init_rand() {
    seed = getpid() + 12345;
}



// Variables globales du jeu
static Snake snake_state;
static struct TFood food;
static int game_over = 0;
static int score = 0;
static int speed = INITIAL_SPEED;



static void park_cursor_outside_game() {
    cursor_move(VGA_WIDTH - 1, VGA_HEIGHT - 1);
}

// Initialiser le serpent
void init_snake() {
    snake_state.length = 3;
    snake_state.body[0].x = GAME_WIDTH / 2;
    snake_state.body[0].y = GAME_HEIGHT / 2;
    snake_state.body[1].x = GAME_WIDTH / 2 - 1;
    snake_state.body[1].y = GAME_HEIGHT / 2;
    snake_state.body[2].x = GAME_WIDTH / 2 - 2;
    snake_state.body[2].y = GAME_HEIGHT / 2;
    
    snake_state.direction = RIGHT;
    snake_state.next_direction = RIGHT;
}

// Générer une nouvelle nourriture
static int is_on_snake(int x, int y) {
    for (int i = 0; i < snake_state.length; i++) {
        if (snake_state.body[i].x == x && snake_state.body[i].y == y) {
            return 1;
        }
    }
    return 0;
}

void generate_food() {
    do {
        food.x = (simple_rand() % (GAME_WIDTH - 2)) + 1;
        food.y = (simple_rand() % (GAME_HEIGHT - 2)) + 1;
    } while (is_on_snake(food.x, food.y));
}

static void put_at(int board_x, int board_y, char c) {
    cursor_move(BOARD_ORIGIN_X + board_x + 1, BOARD_ORIGIN_Y + board_y + 1);
    console_putchar(c);
}

static void draw_border_once() {
    for (int y = 0; y < GAME_HEIGHT + 2; y++) {
        cursor_move(BOARD_ORIGIN_X, BOARD_ORIGIN_Y + y);
        for (int x = 0; x < GAME_WIDTH + 2; x++) {
            if (y == 0 || y == GAME_HEIGHT + 1 || x == 0 || x == GAME_WIDTH + 1) {
                console_putchar('#');
            } else {
                console_putchar(' ');
            }
        }
    }
}

static void draw_hud() {
    cursor_move(0, 1);
    printf("=== SNAKE GAME ===");
    cursor_move(0, 2);
    printf("Score: %d | Speed: %d ms      ", score, speed);
    cursor_move(0, 3);
    printf("==================");
    cursor_move(0, BOARD_ORIGIN_Y + GAME_HEIGHT + 4);
    printf("Controls: ZQSD | X: Quit");
    park_cursor_outside_game();
}

static void draw_game_full_once() {
    printf("\f");
    draw_hud();
    draw_border_once();

    put_at(food.x, food.y, '*');
    for (int i = snake_state.length - 1; i >= 1; i--) {
        put_at(snake_state.body[i].x, snake_state.body[i].y, 'o');
    }
    put_at(snake_state.body[0].x, snake_state.body[0].y, '@');
    park_cursor_outside_game();
}

static void draw_game_diff(Point old_head, Point old_tail, int ate_food) {
    put_at(old_head.x, old_head.y, 'o');
    put_at(snake_state.body[0].x, snake_state.body[0].y, '@');

    if (!ate_food) {
        put_at(old_tail.x, old_tail.y, ' ');
    } else {
        put_at(food.x, food.y, '*');
        draw_hud();
    }

    park_cursor_outside_game();
}

// Mettre à jour la direction en fonction de l'entrée clavier
void handle_input() {
    int key;

    do {
        key = (int) kgetch();
        if (key == -1) {
            break;
        }

        switch (key) {
            case 'z':
            case 'Z':
                if (snake_state.direction != DOWN)
                    snake_state.next_direction = UP;
                break;
            case 's':
            case 'S':
                if (snake_state.direction != UP)
                    snake_state.next_direction = DOWN;
                break;
            case 'q':
            case 'Q':
                if (snake_state.direction != RIGHT)
                    snake_state.next_direction = LEFT;
                break;
            case 'd':
            case 'D':
                if (snake_state.direction != LEFT)
                    snake_state.next_direction = RIGHT;
                break;
            case 'x':
            case 'X':
                game_over = 1;
                break;
        }
    } while (1);
}

// Déplacer le serpent
void move_snake() {
    snake_state.direction = snake_state.next_direction;
    
    // Décaler le corps
    for (int i = snake_state.length - 1; i > 0; i--) {
        snake_state.body[i].x = snake_state.body[i - 1].x;
        snake_state.body[i].y = snake_state.body[i - 1].y;
    }
    
    // Déplacer la tête
    switch (snake_state.direction) {
        case UP:
            snake_state.body[0].y--;
            break;
        case DOWN:
            snake_state.body[0].y++;
            break;
        case LEFT:
            snake_state.body[0].x--;
            break;
        case RIGHT:
            snake_state.body[0].x++;
            break;
    }
}

// Vérifier les collisions
int check_collision() {
    // Collision avec les murs
    if (snake_state.body[0].x <= 0 || snake_state.body[0].x >= GAME_WIDTH - 1 ||
        snake_state.body[0].y <= 0 || snake_state.body[0].y >= GAME_HEIGHT - 1) {
        return 1;
    }
    
    // Collision avec le corps
    for (int i = 1; i < snake_state.length; i++) {
        if (snake_state.body[0].x == snake_state.body[i].x && 
            snake_state.body[0].y == snake_state.body[i].y) {
            return 1;
        }
    }
    
    return 0;
}

// Vérifier si le serpent mange la nourriture
int check_food(Point old_tail) {
    if (snake_state.body[0].x == food.x && snake_state.body[0].y == food.y) {
        score += 10;
        
        // Augmenter la taille du serpent
        if (snake_state.length < MAX_SNAKE) {
            snake_state.body[snake_state.length].x = old_tail.x;
            snake_state.body[snake_state.length].y = old_tail.y;
            snake_state.length++;
        }
        
        // Augmenter la dificulté
        if (speed > 50) {
            speed -= 5;
        }
        
        // Nouvelle nourriture
        generate_food();
        return 1;
    }

    return 0;
}

// Boucle principale du jeu
void snake() {
    printf("\f"); // Effacer l'écran
    printf("====== SNAKE GAME ======\n");
    printf("Use ZQSD to move\n");
    printf("Eat the * to grow\n");
    printf("Avoid walls and yourself\n");
    printf("Press any key to start...\n");
    
    init_keyboard(); // Démasquer l'interrupt clavier
    // Attendre le démarrage
    int ch;
    do {
        ch = (int)kgetch();
        sleep(10);
    } while (ch == -1);
    
    // Initialiser le jeu
    init_rand();
    init_snake();
    generate_food();
    game_over = 0;
    score = 0;
    speed = INITIAL_SPEED;
    int elapsed_ms = 0;

    draw_game_full_once();
    
    // Boucle de jeu
    while (!game_over) {
        handle_input();

        elapsed_ms += INPUT_TICK_MS;
        if (elapsed_ms >= speed) {
            Point old_head = snake_state.body[0];
            Point old_tail = snake_state.body[snake_state.length - 1];
            move_snake();

            if (check_collision()) {
                game_over = 1;
            } else {
                int ate_food = check_food(old_tail);
                draw_game_diff(old_head, old_tail, ate_food);
            }

            elapsed_ms = 0;
        }

        sleep(INPUT_TICK_MS);
    }
    
    // Afficher l'écran de fin
    printf("\f");
    printf("========== GAME OVER ==========\n\n");
    printf("Final Score: %d\n\n", score);
    printf("Final Snake Length: %d\n\n", snake_state.length);
    printf("Press any key to exit...\n");
    
    do {
        ch = (int)kgetch();
        sleep(50);
    } while (ch == -1);
    mask_keyboard(); // Masquer l'interrupt clavier
}
