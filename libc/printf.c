#include <syscall.h>
#include <stdarg.h>


// Accessory Functions

int strlen(const char *str)
{
    int len=0;
    while (*str++ != '\0')
        len += 1;
    return len;
}



// Copies 1 byte at a time
void *memcpy(void *destination, void *source, uint64_t num) 
{
    uint8_t *dest = (uint8_t *)destination;
    uint8_t *src = (uint8_t *)source;

    while(num--) {
        *dest++ = *src++; 
    }

    return destination;
}

// Copies 8 bytes at a time
void *memcpy8(void *destination, void *source, uint64_t num) 
{
    uint64_t *dest = (uint64_t *)destination;
    uint64_t *src = (uint64_t *)source;

    while(num--) {
        *dest++ = *src++; 
    }

    return destination;
}

// Sets 1 byte at a time
void *memset(void *ptr, uint8_t value, uint64_t num)
{
    uint8_t *temp = (uint8_t *)ptr;

    while(num--) {
        *temp++ = value; 
    }
    return ptr;
}

// Sets 8 bytes at a time
void *memset8(void *ptr, uint64_t value, uint64_t num)
{
    uint64_t *temp = (uint64_t *)ptr;

    while(num--) {
        *temp++ = value; 
    }
    return ptr;
}

int32_t pow(int base, int power)
{ 
    int i = 0, product = 1;

    for(i = 0; i < power; ++i) {
        product = product * base;
    }

    return product;
}

int32_t atoi(char *p)
{
    int k = 0, sign =1;

    if (p[0] == '-') {
        sign = -1;
        p++;
    } else if (p[0] == '+') {
        sign = 1;
        p++;
    }
    while (*p) {
        if ( (int)(*p) >= 48 && (int)(*p) <= 57) {
            k = (k<<3)+(k<<1)+(*p)-'0';
            p++;
        } else {
            return 0;
        }

    }

    return k*sign;
}

int32_t oct_to_dec(int n) {
    int dec = 0, i = 0, rem; 
 
    while (n != 0) { 
        rem = n % 10; 
        n /= 10; 
        dec += rem * pow(8, i);
        ++i;
    }
    return dec;
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

//Accessory Functions

char buffer[1024];

int uputs(char *str)
{
    return __syscall1(PUTS, (uint64_t)str);
}

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
                    memcpy((void *) (buffer + len), (void *) dstr, strlen(dstr)+1);
                    len += strlen(dstr);
                    break;
                }
                case 'p':
                {
                    // Prepend "0x"
                    char *pstr= itoa(va_arg(ap, uint64_t), istr+99, 16);
                    *--pstr = 'x';
                    *--pstr = '0';
                    memcpy((void *)(buffer + len), (void *) pstr, strlen(pstr));
                    len += strlen(pstr);
                    break;
                }
                case 'x':
                    {//copy to buffer
                    char *xtr;
                    xtr = itoa(va_arg(ap, uint64_t), istr+99, 16);
                    memcpy((void *)(buffer + len), (void *)xtr , strlen(xtr));
                    len += strlen(xtr);
                    break;}
                case 'c':
                    {//copy to buffer
                    char *ctr;
                    ctr = va_arg(ap, char*);
                    memcpy((void *)(buffer + len), (void *) ctr ,1 );
                    len += 1;
                    break;}
                case 's':
                    {
                    char *str;
                    str = va_arg(ap, char *);
                    memcpy((void *)(buffer + len), (void *)str , strlen(str));
                    len += strlen(str);
                    break;}
                default:
                    {
                    memcpy((void *)(buffer + len), (void *) ptr ,1 );
                    len += 1;
                    break;}
            }
         } else {
            memcpy((void *)(buffer + len), (void *) ptr ,1 );
            len += 1;
        }
    }
    va_end(ap); 
    uputs(buffer);
    return len;
}
