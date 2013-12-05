#include <stdlib.h>
#include <syscall.h>

int munmap(uint64_t* addr, uint64_t size)
{

return __syscall2(MUNMAP, (uint64_t )addr, size);

}
