#ifndef _SYSCALL_H
#define _SYSCALL_H

#include <defs.h>

#define SYSCALL_PROTO(n) static __inline uint64_t __syscall##n


SYSCALL_PROTO(0)(uint64_t n)
{
    uint64_t ret;
    __asm__ __volatile__("movq %[retv], %%rax;" : : [retv]"a"(n));
    __asm__ __volatile__("int $0x80" : "=a" (ret) : "0" (n));
    return ret;
}

SYSCALL_PROTO(1)(uint64_t n, uint64_t a1)
{
    uint64_t ret;
    __asm__ __volatile__("movq %[s], %%rdx;" : : [s]"m"(a1));
    __asm__ __volatile__("movq %[retv], %%rax;" : : [retv]"a"(n));
    __asm__ __volatile__("int $0x80;" : "=a" (ret));
    return ret;
}

SYSCALL_PROTO(2)(uint64_t n, uint64_t a1, uint64_t a2)
{
    return 0;
}

SYSCALL_PROTO(3)(uint64_t n, uint64_t a1, uint64_t a2, uint64_t a3)
{
    return 0;
}

SYSCALL_PROTO(4)(uint64_t n, uint64_t a1, uint64_t a2, uint64_t a3, uint64_t a4)
{
    return 0;
}

enum {
    PUTS,
    GETS,
    MMAP,
    FORK,
    NUM_SYSCALLS
};

#endif

