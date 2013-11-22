#ifndef _STDLIB_H
#define _STDLIB_H

int main(int argc, char* argv[]);
void exit(int status);
int printf(const char *format, ...);
void free(void *p);
void *malloc(int elem_size);
void init_malloc();

#endif
