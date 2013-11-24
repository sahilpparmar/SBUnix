#include <stdlib.h>
#include <syscall.h>

void* mmap(void* addr, uint64_t nbytes, uint64_t flags)
{
 return (void *)__syscall3(MMAP, (uint64_t)addr, nbytes, flags);
}

