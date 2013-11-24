#ifndef _SYSCALL_H
#define _SYSCALL_H

#include <defs.h>

#define SYSCALL_PROTO(n) static __inline uint64_t __syscall##n

SYSCALL_PROTO(0)(uint64_t n)
{
    uint64_t ret;
    __asm__ __volatile__("movq %[retv], %%rax;" : : [retv]"a"(n));
    __asm__ __volatile__("int $0x80" : "=a" (ret));
    return ret;
}

SYSCALL_PROTO(1)(uint64_t n, uint64_t a1)
{
    uint64_t ret;
    __asm__ __volatile__("movq %[a1], %%rdi;" : : [a1]"m"(a1));
    __asm__ __volatile__("movq %[retv], %%rax;" : : [retv]"a"(n));
    __asm__ __volatile__("int $0x80;" : "=a" (ret));
    return ret;
}

SYSCALL_PROTO(2)(uint64_t n, uint64_t a1, uint64_t a2)
{
    uint64_t ret;
    __asm__ __volatile__("movq %[a1], %%rdi;" : : [a1]"m"(a1));
    __asm__ __volatile__("movq %[a2], %%rsi;" : : [a2]"m"(a2));
    __asm__ __volatile__("movq %[retv], %%rax;" : : [retv]"a"(n));
    __asm__ __volatile__("int $0x80;" : "=a" (ret));
    return ret;
}

SYSCALL_PROTO(3)(uint64_t n, uint64_t a1, uint64_t a2, uint64_t a3)
{
    uint64_t ret;
    __asm__ __volatile__("movq %[a1], %%rdi;" : : [a1]"m"(a1));
    __asm__ __volatile__("movq %[a2], %%rsi;" : : [a2]"m"(a2));
    __asm__ __volatile__("movq %[a3], %%rdx;" : : [a3]"m"(a3));
    __asm__ __volatile__("movq %[retv], %%rax;" : : [retv]"a"(n));
    __asm__ __volatile__("int $0x80;" : "=a" (ret));
    return ret;
}

SYSCALL_PROTO(4)(uint64_t n, uint64_t a1, uint64_t a2, uint64_t a3, uint64_t a4)
{
    uint64_t ret;
    __asm__ __volatile__("movq %[a1], %%rdi;" : : [a1]"m"(a1));
    __asm__ __volatile__("movq %[a2], %%rsi;" : : [a2]"m"(a2));
    __asm__ __volatile__("movq %[a3], %%rdx;" : : [a3]"m"(a3));
    __asm__ __volatile__("movq %[a4], %%rcx;" : : [a4]"m"(a4));
    __asm__ __volatile__("movq %[retv], %%rax;" : : [retv]"a"(n));
    __asm__ __volatile__("int $0x80;" : "=a" (ret));
    return ret;
}

enum syscall_num {
    READ,
    WRITE,
    BRK, 
    FORK,
    MMAP,
    MUNMAP, 
    GETPID,
    GETPPID,
    NUM_SYSCALLS
};

#endif

