#include <defs.h>
#include <stdlib.h>

char hello[10] = "HELLO";

int main(int argc, char* argv[])
{
    printf("\nProcess %d (parent %d) says %s", getpid(), getppid(), hello);
    while(1);
    return 0;
}
