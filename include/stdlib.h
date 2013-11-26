#ifndef _STDLIB_H
#define _STDLIB_H
#include <defs.h>

int main(int argc, char* argv[]);
void exit(int status);
int sleep(int seconds);
int printf(const char *format, ...);
void scanf(const char *format, ...);

void *malloc(int elem_size);
void free(void *p);

void* mmap(void* addr, uint64_t nbytes, uint64_t flags);
int munmap(uint64_t* addr, uint64_t size);

pid_t getpid();
pid_t getppid();
void listprocess();

pid_t fork();
int execvpe(const char *file, char *const argv[], char *const envp[]);

#endif
