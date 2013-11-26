#include <defs.h>
#include <stdlib.h>

char buf[10] = "PS Command";

int main(int argc, char* argv[])
{
    printf("\nProcess %d (parent %d) says %s", getpid(), getppid(), buf);

    listprocess();

    return 0;
}

