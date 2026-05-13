#ifndef STDLIB_H_
#define STDLIB_H_

#include <stddef.h>

long int strtol(const char *nptr, char **endptr, int base);
unsigned long int strtoul(const char *nptr, char **endptr, int base);
int atoi(const char *nptr);

void *malloc(size_t size);
void *calloc(size_t nmemb, size_t size);
void *realloc(void *ptr, size_t size);
void free(void *ptr);

#define RAND_MAX 0x7fffffff
void srand(unsigned int seed);
int rand(void);

int abs(int n);
long labs(long n);

#endif /*STDLIB_H_*/
