#include <stdlib.h>

int main(int argc, char **argv)
{
    if (fork() == 0) {
        execvpe("/rootfs/bin/sh", NULL, NULL);
    }

    wait(NULL);
    return 1;
}

