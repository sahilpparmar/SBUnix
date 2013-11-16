#include <defs.h>
#include <stdlib.h>

char hello[10] = "hello";
char blank[10] = "     ";

int main(int argc, char* argv[])
{
    int i;
    char *addr = (char*)0xFFFFFFFF800B8000;
    
    i = 0;
    *addr++ = hello[i++];
    addr++;
    *addr++ = hello[i++];
    addr++;
    *addr++ = hello[i++];
    addr++;
    *addr++ = hello[i++];
    addr++;
    *addr++ = hello[i++];
    addr++;
    i = 0;
    *addr++ = blank[i++];
    addr++;
    *addr++ = blank[i++];
    addr++;
    *addr++ = blank[i++];
    addr++;
    *addr++ = blank[i++];
    addr++;
    *addr++ = blank[i++];
    addr++;
    
    while(1);
    return 0;
}
