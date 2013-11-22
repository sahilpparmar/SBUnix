#ifndef _STDLIB_H
#define _STDLIB_H

int main(int argc, char* argv[]);
void exit(int status);

int printf(const char *format, ...);
void scanf(const char *format, ...);

void init_malloc();
void *malloc(int elem_size);
void free(void *p);

int fork();

#endif
