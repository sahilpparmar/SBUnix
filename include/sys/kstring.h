#ifndef _STRING_H
#define _STRING_H

#include <defs.h>

int kstrlen(const char *s);
int kstrcmp(const char *s1, const char *s2);
char* kstrcat(char *str1, const char *str2);
char* kstrcpy(char *dest, const char *src);
char* kstrtok(char *s, const char *delim);
char* kstrcpyn(char *dest, const char *src, uint64_t n);
#endif
