#include <stdarg.h>
#include <defs.h>
#include <stdio.h>
#include <syscall.h>
#include <stdlib.h>

char read_buf[1024];

int read(int n, char *str, int len)
{
    return __syscall3(READ, n, (uint64_t)str, len);
}

void scanf(const char *str, ...)
{
    int len;
    int32_t num;
    va_list ap;
    const char *ptr = NULL;
    va_start(ap, str);
    for (ptr = str; *ptr; ptr++) {
        if (*ptr == '%') {
            switch (*++ptr) {
                case 's':
                    len = read(stdin, read_buf, 0);
                    memcpy((void *) va_arg(ap, char *) , (void *)read_buf, len); 
                    break;
                case 'd':
                    len = read(stdin, read_buf, 0);
                    num = atoi(read_buf);
                    memcpy8( (void *) va_arg(ap, int32_t *), (void *) &num, len);
                    break;
                case 'c':
                    len = read(stdin, read_buf, 0);
                    memcpy( (void *) va_arg(ap, char *), (void *)read_buf, 1);
                    break;
                default:
                    break;
            }
        }
    }
    va_end(ap); 
}
