#ifndef _STDIO_H
#define _STDIO_H

//#include <unistd.h>


char *itoa(uint64_t val, char *str, int32_t base);
void putchar(char mychar);
int puts(char *str);
int printf(char *str, ...);

int scanf(const char *format, ...);

void *memcpy(void *destination, void *source, int num);
void *memset(void *ptr, int value, int num);

int pow(int , int);
int atoi(char *p);
int octal_decimal(int n);
#endif

