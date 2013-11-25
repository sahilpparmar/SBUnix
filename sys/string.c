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

char * kstrcpyn(char *destination, const char *source, uint64_t n)
{
    uint64_t i = 0;
    char *str = destination;
    for (i = 0; i < n; i++){
            *destination++ = *source++;
            if ( *source == '\0') {
                *destination++ = '\0';
                return(str);
            }
        }
    *destination++ = '\0';

    return str;
}


