#ifndef _STDIO_H
#define _STDIO_H

char *itoa(uint64_t val, char *str, int32_t base);
void putchar(char mychar);
int32_t puts(char *str);
int32_t printf(const char *str, ...);

int32_t scanf(const char *format, ...);

void *memcpy(void *destination, void *source, int num);
void *memset(void *ptr, int value, int num);

int32_t pow(int base, int power);
int32_t atoi(char *p);
int32_t octal_decimal(int n);
#endif

