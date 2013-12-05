#ifndef _STDLIB_H
#define _STDLIB_H
#include <defs.h>

int main(int argc, char* argv[]);
void exit(int status);
int sleep(int seconds);
int printf(const char *format, ...);
void scanf(const char *format, ...);
int cls();
void *malloc(int elem_size);
void free(void *p);

void* mmap(void* addr, uint64_t nbytes, uint64_t flags);
int munmap(uint64_t* addr, uint64_t size);

int open(char *path, int flags);
uint64_t read(uint64_t fd, void *buf, uint64_t nbytes);
int write(int n, char *str, int len);
int seek(uint64_t file_d, int offset, int whence);
void close(int file_d);

pid_t getpid();
pid_t getppid();
void listprocess();

pid_t fork();
int execvpe(const char *file, char *const argv[], char *const envp[]);
pid_t wait(uint64_t *status);
pid_t waitpid(pid_t pid, uint64_t *status, int options);
int mkdir(char *path);

void *memcpy(void *destination, void *source, uint64_t num);
void *memset(void *ptr, uint8_t value, uint64_t num);
char * strtok(char *s, const char *delim);
int strcmp(const char *s1, const char *s2);
int strlen(const char *str);
char* strcat(char *str1, const char *str2);
char *strcpy(char *dest, const char *src);
int32_t pow(int base, int power);
int32_t oct_to_dec(int n);
int32_t atoi(char *p);
char *itoa(uint64_t val, char *str, int32_t base);

#endif
