#include <stdlib.h>
#include <syscall.h>

int cls()
{
   return __syscall0(CLEAR);
}
