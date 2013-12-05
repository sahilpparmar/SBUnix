#include <stdarg.h>
#include <defs.h>
#include <sys/common.h>
#include <syscall.h>
#include <stdlib.h>

char read_buf[1024];

uint64_t read(uint64_t fd, void *buf, uint64_t nbytes)
{
    return __syscall3(READ, fd, (uint64_t)buf, nbytes);
}


void scanf(const char *str, ...)
{
    int len;
    va_list ap;
    const char *ptr = NULL;
    va_start(ap, str);
    for (ptr = str; *ptr; ptr++) {
        if (*ptr == '%') {
            switch (*++ptr) {
                case 's':
                    len = read(stdin, read_buf, 0);
                    memcpy((void *) va_arg(ap, char*), (void *)read_buf, len); 
                    break;
                case 'd':
                {
                    int32_t *dec = (int32_t*) va_arg(ap, int32_t*);
                    read(stdin, read_buf, 0);
                    *dec = atoi(read_buf);
                    break;
                }
                case 'c':
                {
                    char *ch = (char *) va_arg(ap, char*);
                    read(stdin, read_buf, 0);
                    *ch = read_buf[0]; 
                    break;
                }
                default:
                    break;
            }
        }
    }
    va_end(ap); 
}
