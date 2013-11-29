#include <defs.h>
#include <stdlib.h>

char *args[] = {"child", "arg1", "arg2", NULL};

int main(int argc, char* argv[])
{
    pid_t pid;
    printf("\nProcess %d (parent %d) is %s", getpid(), getppid(), argv[0]);

    //char *str = (char*)malloc(100);
    //int fd = open("/rootfs/newfolder/timepass/helloworldfile.txt", 0);
    //int len = read(fd, str, 0x60);
    //for (int i = 0; i < len; i++)
    //    printf("%c", str[i]);
    
    if ((pid = fork()) == 0) {
        printf("\nFork return: %d Process %d (parent %d): child", pid, getpid(), getppid());

    //    printf("\nChild %p %p\n", &fd, &str);
    //    int len = read(fd, (void*)str, 0x60);
    //    for (int i = 0; i < len; i++)
    //        printf("%c", str[i]);

    //    len = read(fd, (void*)str, 0x60);
    //    for (int i = 0; i < len; i++)
    //        printf("%c", str[i]);

        execvpe("bin/ps", args, NULL);
    } else {
    //    printf("\nParent %p %p\n", fd, str);
    //    int len = read(fd, str, 0x60);
    //    for (int i = 0; i < len; i++)
    //        printf("%c", str[i]);
        printf("\nFork return: %d Process %d (parent %d): parent", pid, getpid(), getppid());
    }

    waitpid(pid, NULL, 0);
    printf("\nWait Return");

    return 0;
}

