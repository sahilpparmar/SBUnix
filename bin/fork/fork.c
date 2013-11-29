#include <defs.h>
#include <stdlib.h>

char *args[] = {"child", "arg1", "arg2", NULL};

int main(int argc, char* argv[])
{
    pid_t pid;
    printf("\nProcess %d (parent %d) is %s", getpid(), getppid(), argv[0]);

    if ((pid = fork()) == 0) {
        printf("\nFork return: %d Process %d (parent %d): child", pid, getpid(), getppid());
        execvpe("bin/ps", args, NULL);
    } else {
        printf("\nFork return: %d Process %d (parent %d): parent", pid, getpid(), getppid());
    }

    //wait(NULL);
    waitpid(pid, NULL, 0);
    printf("\nWait Return");

    while(1);
    return 0;
}

