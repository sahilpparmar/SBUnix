#include <defs.h>
#include <stdio.h>

int kstrcmp(const char *s1, const char *s2)
{
    while (*s1 == *s2++)
        if (*s1++ == 0)
            return (0);

    return (*(const unsigned char *)s1 - *(const unsigned char *)(s2 - 1));
}


int kstrlen(const char *str)
{
    int len=0;
    while (*str++ != '\0')
        len += 1;
    return len;
}

char* kstrcat(char *str1, const char *str2)
{
    uint64_t len1 = kstrlen(str1);
    uint64_t len2 = kstrlen(str2);
    uint64_t i = 0;

    for(i = 0; i < len2 ; i++)
        str1[len1 + i] = str2[i];
    str1[len1 + i] = '\0';

    return str1;    
}

char * kstrcpy(char *dest, const char *src)
{
    char *str = dest;
    while (*src) {
        *dest++ = *src++;
    }
    *dest = '\0';
    return str;
}

char * kstrcpyn(char *destination, const char *source, uint64_t n)
{
    uint64_t i = 0;
    char *str = destination;

    for (i = 0; i < n; i++) {
        *destination++ = *source++;
        if ( *source == '\0') {
            *destination++ = '\0';
            return(str);
        }
    }
    *destination++ = '\0';

    return str;
}

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


char * kstrtok(char *s, const char *delim)
{
    static char *last;

    return strtok_r(s, delim, &last);
}


