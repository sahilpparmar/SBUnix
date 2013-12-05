#include <stdlib.h>

static char * strtok_r(char *s, const char *delim, char **last)
{
    char *spanp;
    int c, sc;
    char *tok;


    if (s == NULL && (s = *last) == NULL)
        return (NULL);

    /*
     *   * Skip (span) leading delimiters (s += strspn(s, delim), sort of).
     *       */
cont:
    c = *s++;
    for (spanp = (char *)delim; (sc = *spanp++) != 0;) {
        if (c == sc)
            goto cont;
    }

    if (c == 0) {       /* no non-delimiter characters */
        *last = NULL;
        return (NULL);
    }
    tok = s - 1;

    /*
     *   * Scan token (scan for delimiters: s += strcspn(s, delim), sort of).
     *       * Note that delim must have one NUL; we stop if we see that, too.
     *           */
    for (;;) {
        c = *s++;
        spanp = (char *)delim;
        do {
            if ((sc = *spanp++) == c) {
                if (c == 0)
                    s = NULL;
                else
                    s[-1] = 0;
                *last = s;
                return (tok);
            }
        } while (sc != 0);
    }
    /* NOTREACHED */
}


char * strtok(char *s, const char *delim)
{
    static char *last;

    return strtok_r(s, delim, &last);
}

void *memset(void *ptr, uint8_t value, uint64_t num)
{
    uint8_t *temp = (uint8_t *)ptr;

    while(num--) {
        *temp++ = value; 
    }
    return ptr;
}

int strcmp(const char *s1, const char *s2)
{
    while (*s1 == *s2++)
        if (*s1++ == 0)
            return (0);

    return (*(const unsigned char *)s1 - *(const unsigned char *)(s2 - 1));
}

int strlen(const char *str)
{
    int len=0;
    while (*str++ != '\0')
        len += 1;
    return len;
}

char* strcat(char *str1, const char *str2)
{
    uint64_t len1 = strlen(str1);
    uint64_t len2 = strlen(str2);
    uint64_t i = 0;

    for(i = 0; i < len2 ; i++)
        str1[len1 + i] = str2[i];
    str1[len1 + i] = '\0';

    return str1;    
}

char *strcpy(char *dest, const char *src)
{
    char *str = dest;
    while (*src)
    {
        *dest++ = *src++;
    }
    *dest = '\0';
    return str;
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

