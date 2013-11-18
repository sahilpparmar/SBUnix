#include <defs.h>
#include <stdlib.h>

char world[10] = "world";
char blank[10] = "     ";

int main(int argc, char* argv[])
{
    int i;
    volatile char *addr;
    
    addr = (char*)0xFFFFFFFF800B800C;
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
