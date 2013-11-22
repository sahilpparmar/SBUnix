#include <stdarg.h>
#include <defs.h>
#include <stdio.h>
#include <syscall.h>
#include <stdlib.h>

char read_buf[1024];

int ugets(char *str)
{
    return __syscall1(GETS, (uint64_t)str);
}

void scanf(const char *str, ...)
{
    va_list ap;
    const char *ptr = NULL;
    va_start(ap, str);
    for (ptr = str; *ptr; ptr++) {
        if (*ptr == '%') {
            switch (*++ptr) {
                case 's':
                    /*flag = 1;
                    while(flag == 1);
                    memcpy((void *) va_arg(ap, char *) , (void *)buf, counter); 
                    counter = 0;*/
                    break;
                case 'd':
                    /*flag = 1;
                    while(flag == 1);
                    num = (int32_t)atoi((char *)buf);
                    memcpy8( (void *) va_arg(ap, int32_t *), (void *) &num, counter);
                    counter = 0;*/
                    break;
                case 'c':
                    memcpy( (void *) va_arg(ap, char *), (void *)read_buf, 1);
                    break;
                default:
                    break;
            }
        }
    }
    va_end(ap); 
}
