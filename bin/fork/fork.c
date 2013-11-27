#include <defs.h>
#include <stdlib.h>

char buf[10] = "FORK";

int main(int argc, char* argv[])
{
    pid_t pid;
    printf("\nProcess %d (parent %d) says %s", getpid(), getppid(), buf);

    if ((pid = fork()) == 0) {
        printf("\nFork return: %d Process %d (parent %d): child", pid, getpid(), getppid());
        execvpe("bin/ps", NULL, NULL);
    } else {
        printf("\nFork return: %d Process %d (parent %d): parent", pid, getpid(), getppid());
    }

    //wait(NULL);
    waitpid(pid, NULL, 0);
    printf("\nWait Return");

    while(1);
    return 0;
}

