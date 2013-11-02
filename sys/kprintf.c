//
// kprintf.c -- Implementation for common libc routines  
//

#include <stdarg.h>
#include <defs.h>
#include <stdio.h>
#include <screen.h>

extern void putchar(char mychar);

int32_t puts(char* mystring)
{
    char *temp = mystring;
    int32_t len = 0;
    for (; *temp; ++temp) {
        putchar(*temp);
        len++;
    }
    return len;
}

char *itoa(uint64_t val, char *str, int32_t base)
{
    *str = '\0'; // Currently pointing to the end of string
    if (val == 0) {
        *--str = '0';
        return str;
    }
    if (base != 10 && base != 16) {
        return str;
    }
    while (val) {
        *--str = "0123456789abcdef"[val%base];
        val = val/base;
    }
    return str;
}

int printf(char *str, ...)
{
    va_list ap;
    char *ptr, istr[100];
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
                    len += puts(dstr);
                    break;
                }
                case 'p':
                {
                    // Prepend "0x"
                    char *pstr = itoa(va_arg(ap, uint64_t), istr+99, 16);
                    *--pstr = 'x';
                    *--pstr = '0';
                    len += puts(pstr);
                    break;
                }
                case 'x':
                    len += puts(itoa(va_arg(ap, uint32_t), istr+99, 16));
                    break;
                case 'c':
                    putchar(va_arg(ap, int32_t));
                    len++;
                    break;
                case 's':
                    len += puts(va_arg(ap, char*));
                    break;
                default:
                    putchar(*ptr);
                    len++;
                    break;
            }
        } else if (*ptr == '\n') {
            newline();
            len++;
        } else if (*ptr == '\t') {
            newtab();    
            len++;
        } else {
            putchar(*ptr);
            len++;
        }
    }
    va_end(ap); 
    return len;
}

void *memcpy(void *destination, void *source, int num) 
{
    uint8_t *dest = (uint8_t *)destination;
    uint8_t *src = (uint8_t *)source;

    while(num--) {
        *dest++ = *src++; 
    }

    return destination;
}

void *memset(void *ptr, int value, int num)
{
    uint8_t *temp = (uint8_t *)ptr;

    while(num--) {
        *temp++ = (uint8_t)value; 
    }

    return ptr;
}

int pow(int base, int power)
{ 
    int i = 0, product = 1;

    for(i = 0; i < power; ++i)
    {
        product = product * base;
    }

    return product;
}

int atoi(char *p) {
    int k = 0;

    while (*p) {
        k = (k<<3)+(k<<1)+(*p)-'0';
        p++;
    }
    return k;
}


int octal_decimal(int n){
    int decimal=0, i=0, rem; 
 
    while (n!=0) { 
        rem = n%10; 
        n/=10; 
        decimal += rem * pow(8,i);
        ++i; 
    }
    return decimal;
}
