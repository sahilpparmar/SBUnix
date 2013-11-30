#include <stdlib.h>

int main(int argc, char **argv)
{
    pid_t pid;
    if ((pid = fork()) == 0) {
        execvpe("/rootfs/bin/sh", NULL, NULL);
    } else {
        waitpid(pid, NULL, 0);
    }
    return 1;
}

