#include <stdarg.h>
#include <defs.h>
#include <sys/common.h>
#include <syscall.h>
#include <stdlib.h>


int nanosleep(int sec)
{
    return __syscall1(SLEEP, sec);
}

int sleep(int seconds)
{
    if (seconds == 0)
        return 0;
    else
       return nanosleep(seconds);
}
