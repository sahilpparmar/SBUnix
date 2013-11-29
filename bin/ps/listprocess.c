#include <defs.h>
#include <stdlib.h>

int main(int argc, char* argv[])
{
    printf("\nProcess %d (parent %d) is ", getpid(), getppid());

    for (int i = 0; i < argc; i++)
        printf("%s ", argv[i]);

    listprocess();

    return 0;
}

