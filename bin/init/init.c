#include <stdlib.h>

int main(int argc, char **argv)
{
    if (fork() == 0) {
        printf("\nin init\n");
        execvpe("bin/sh", NULL, NULL);
    }
    //while
    while(1);
    return 1;
}
