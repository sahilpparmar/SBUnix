#include <defs.h>
#include <stdlib.h>

char world[10] = "world";
char blank1[10] = "     ";

int main(int argc, char* argv[])
{
    int i;
    char *addr = (char*)0xFFFFFFFF800B8000;
    
    i = 0;
    *addr++ = blank1[i++];
    addr++;
    *addr++ = blank1[i++];
    addr++;
    *addr++ = blank1[i++];
    addr++;
    *addr++ = blank1[i++];
    addr++;
    *addr++ = blank1[i++];
    addr++;
    i = 0;
    *addr++ = world[i++];
    addr++;
    *addr++ = world[i++];
    addr++;
    *addr++ = world[i++];
    addr++;
    *addr++ = world[i++];
    addr++;
    *addr++ = world[i++];
    addr++;
    
    while(1);
    return 0;
}