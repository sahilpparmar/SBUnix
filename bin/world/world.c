#include <defs.h>
#include <stdlib.h>

char world[10] = "WORLD";

int main(int argc, char* argv[])
{
    printf("\nProcess %d (parent %d) says %s", getpid(), getppid(), world);
    while(1);
    return 0;
}
