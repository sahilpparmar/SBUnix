#include <string.h>

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
