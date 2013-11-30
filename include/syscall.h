#ifndef _SYSCALL_H
#define _SYSCALL_H

#include <defs.h>

#define SYSCALL_PROTO(n) static __inline uint64_t __syscall##n

SYSCALL_PROTO(0)(uint64_t n)
{
    uint64_t ret;
    __asm__ __volatile__(
        "movq %[no], %%rax;"
        "int $0x80;"
        : "=a" (ret)
        : [no]"a"(n)
        : "%rdi", "%rsi", "%rdx", "%rcx", "%rbp", "%rbx", "%r12", "%r13", "%r14", "%r15"
    );
    return ret;
}

SYSCALL_PROTO(1)(uint64_t n, uint64_t a1)
{
    uint64_t ret;
    __asm__ __volatile__(
        "movq %[a1], %%rdi;" 
        "movq %[no], %%rax;"
        "int $0x80;"
        : "=a" (ret)
        : [a1]"g"(a1), [no]"a"(n)
        : "%rdi", "%rsi", "%rdx", "%rcx", "%rbp", "%rbx", "%r12", "%r13", "%r14", "%r15"
    );
    return ret;
}

SYSCALL_PROTO(2)(uint64_t n, uint64_t a1, uint64_t a2)
{
    uint64_t ret;
    __asm__ __volatile__(
        "movq %[a1], %%rdi;" 
        "movq %[a2], %%rsi;" 
        "movq %[no], %%rax;"
        "int $0x80;"
        : "=a" (ret)
        : [a1]"g"(a1), [a2]"g"(a2), [no]"a"(n)
        : "%rdi", "%rsi", "%rdx", "%rcx", "%rbp", "%rbx", "%r12", "%r13", "%r14", "%r15"
    );
    return ret;
}

SYSCALL_PROTO(3)(uint64_t n, uint64_t a1, uint64_t a2, uint64_t a3)
{
    uint64_t ret;
    __asm__ __volatile__(
        "movq %[a1], %%rdi;" 
        "movq %[a2], %%rsi;" 
        "movq %[a3], %%rdx;" 
        "movq %[no], %%rax;"
        "int $0x80;"
        : "=a" (ret)
        : [a1]"g"(a1), [a2]"g"(a2), [a3]"g"(a3), [no]"a"(n)
        : "%rdi", "%rsi", "%rdx", "%rcx","%rbp", "%rbx", "%r12", "%r13", "%r14", "%r15"
    );
    return ret;
}

SYSCALL_PROTO(4)(uint64_t n, uint64_t a1, uint64_t a2, uint64_t a3, uint64_t a4)
{
    uint64_t ret;
    __asm__ __volatile__(
        "movq %[a1], %%rdi;" 
        "movq %[a2], %%rsi;" 
        "movq %[a3], %%rdx;" 
        "movq %[a4], %%rcx;" 
        "movq %[no], %%rax;"
        "int $0x80;"
        : "=a" (ret)
        : [a1]"g"(a1), [a2]"g"(a2), [a3]"g"(a3), [a4]"g"(a4), [no]"a"(n)
        : "%rdi", "%rsi", "%rdx", "%rcx", "%rbp", "%rbx", "%r12", "%r13", "%r14", "%r15"
    );
    return ret;
}

enum syscall_num {
    READ,
    WRITE,
    BRK, 
    FORK,
    EXECVPE,
    WAIT,
    WAITPID,
    EXIT,
    MMAP,
    MUNMAP, 
    GETPID,
    GETPPID,
    LISTPROCESS,
    OPENDIR,
    READDIR,
    CLOSEDIR, 
    OPEN, 
    CLOSE,
    SLEEP,
    CLEAR,
    NUM_SYSCALLS
};

#endif

