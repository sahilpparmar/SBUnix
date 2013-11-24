#include <string.h>
#include <defs.h>
#include <stdio.h>

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

char * kstrcat(char *str1, const char *str2)
{
    uint64_t len1 = strlen(str1);
    uint64_t len2 = strlen(str2);
    uint64_t i = 0;
    
    for(i = 0; i < len2 ; i++)
        str1[len1 + i] = str2[i];
    str1[len1 + i] = '\0';

    return str1;    
}

char * kstrcpy(char *dest, const char *src)
{
    char *str = dest;
    while (*src)
    {
        *dest++ = *src++;
    }
    *dest = '\0';
    return str;
}
