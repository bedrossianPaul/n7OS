#include <stdlib.h>

static unsigned long rand_state = 1;

void srand(unsigned int seed) {
    rand_state = seed ? seed : 1;
}

int rand(void) {
    rand_state = rand_state * 1103515245u + 12345u;
    return (int)((rand_state >> 1) & RAND_MAX);
}

int abs(int n) {
    return (n < 0) ? -n : n;
}

long labs(long n) {
    return (n < 0) ? -n : n;
}