#ifndef _STRING_H
#define _STRING_H

#include <defs.h>

int strlen(const char *s);
int strcmp(const char *s1, const char *s2);
char* kstrcat(char *str1, const char *str2);
char* kstrcpy(char *dest, const char *src);
char * kstrcpyn(char *dest, const char *src, uint64_t n);
#endif
