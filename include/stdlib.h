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
void close(int file_d);

pid_t getpid();
pid_t getppid();
void listprocess();

pid_t fork();
int execvpe(const char *file, char *const argv[], char *const envp[]);
pid_t wait(uint64_t *status);
pid_t waitpid(pid_t pid, uint64_t *status, int options);

#endif
