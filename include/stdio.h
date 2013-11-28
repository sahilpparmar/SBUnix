#ifndef _STDIO_H
#define _STDIO_H
#include <defs.h>

char *itoa(uint64_t val, char *str, int32_t base);
void putchar(char mychar);
int32_t puts(char *str);
int32_t kprintf(const char *str, ...);

int gets(uint64_t addr);
void kscanf(const char *str, ...);

// By default copies 1 byte at a time
void *memcpy(void *destination, void *source, uint64_t num);
// Copies 8 bytes at a time
void *memcpy8(void *destination, void *source, uint64_t num);

// By default sets 1 byte of memory
void *memset(void *ptr, uint8_t value, uint64_t num);
// Sets 8 bytes of memory
uint64_t *memset8(uint64_t *ptr, uint64_t value, uint64_t num);

int32_t pow(int base, int power);
int32_t atoi(char *p);
int32_t oct_to_dec(int n);

#endif

