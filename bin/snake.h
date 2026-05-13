#ifndef __SNAKE_H__
#define __SNAKE_H__

#include <unistd.h>
#include <stdio.h>

#define INPUT_TICK_MS 20
#define BOARD_ORIGIN_X 0
#define BOARD_ORIGIN_Y 5
#define GAME_WIDTH  40
#define GAME_HEIGHT 15
#define MAX_SNAKE   500
#define INITIAL_SPEED 100  // ms entre les mouvements

enum TDirection {
    UP,
    DOWN,
    LEFT,
    RIGHT
};

struct TFood {
    int x;
    int y;
};

typedef struct {
    int x;
    int y;
} Point;

typedef struct {
    Point body[MAX_SNAKE];
    int length;
    enum TDirection direction;
    enum TDirection next_direction;
} Snake;

void snake();

#endif
