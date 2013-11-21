#include <stdarg.h>
#include <defs.h>
#include <stdio.h>

volatile int flag, counter;
volatile char buf[1024];


void scanf(const char *str, ...)
{
    va_list ap;
    const char *ptr = NULL;
    va_start(ap, str);
    int32_t num;
    for (ptr = str; *ptr; ptr++) {
        if (*ptr == '%') {
            switch (*++ptr) {
                case 's':
                    flag = 1;
                    while(flag == 1);
                    memcpy((void *) va_arg(ap, char *) , (void *)buf, counter); 
                    counter = 0;
                    break;
                case 'd':
                    flag = 1;
                    while(flag == 1);
                    num = (int32_t)atoi((char *)buf);
                    memcpy8( (void *) va_arg(ap, int32_t *), (void *) &num, counter);
                    counter = 0;
                    break;
                case 'c':
                    flag = 1;
                    while(flag == 1);
                    memcpy( (void *) va_arg(ap, char *), (void *) buf, 1);
                    counter = 0;
                    break;
                default:
                    break;
            }
        }
    }
    va_end(ap); 
}
