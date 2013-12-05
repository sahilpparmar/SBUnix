#include <syscall.h>
#include <stdarg.h>
#include <stdlib.h>
#include <defs.h>

static char write_buf[1024];

int32_t printf(const char *str, ...)
{
    va_list ap;
    const char *ptr = NULL;
    char istr[100];
    int32_t len = 0;

    va_start(ap, str); 
    for (ptr = str; *ptr; ptr++) {
        if (*ptr == '%') {
            switch (*++ptr) {
                case 'd':
                {
                    // Consider negative signed integers
                    int32_t isNegative = 0;
                    int32_t ival = va_arg(ap, int32_t);
                    char *dstr;

                    if (ival < 0) {
                        isNegative = 1;
                        ival = -ival;
                    }
                    dstr = itoa(ival, istr+99, 10);

                    if (isNegative) {
                        *--dstr = '-';
                    }
                    memcpy((void *)(write_buf + len), (void *)dstr, strlen(dstr));
                    len += strlen(dstr);
                    break;
                }
                case 'p':
                {
                    // Prepend "0x"
                    char *pstr = itoa(va_arg(ap, uint64_t), istr+99, 16);
                    *--pstr = 'x';
                    *--pstr = '0';
                    memcpy((void *)(write_buf + len), (void *)pstr, strlen(pstr));
                    len += strlen(pstr);
                    break;
                }
                case 'x':
                {
                    char *xtr;
                    xtr = itoa(va_arg(ap, uint64_t), istr+99, 16);
                    memcpy((void *)(write_buf + len), (void *)xtr , strlen(xtr));
                    len += strlen(xtr);
                    break;
                }
                case 'c':
                {
                    write_buf[len] = va_arg(ap, uint32_t);
                    len += 1;
                    break;
                }
                case 's':
                {
                    char *str;
                    str = va_arg(ap, char *);
                    memcpy((void *)(write_buf + len), (void *)str , strlen(str));
                    len += strlen(str);
                    break;
                }
                case '\0':
                    ptr--;
                    break;
                default:
                {
                    memcpy((void *)(write_buf + len), (void *) ptr ,1 );
                    len += 1;
                    break;
                }
            }
         } else {
            memcpy((void *)(write_buf + len), (void *) ptr ,1 );
            len += 1;
       }
    }
    va_end(ap); 
    write_buf[len] = '\0';
    //uputs(write_buf);
    write(stdout, write_buf, strlen(write_buf));
    return len;
}

