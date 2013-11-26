#include <stdlib.h>

int main(int argc, char **argv)
{
    
    if (fork()) {
        execvpe("bin/sh", NULL, NULL);
        printf("\nFinish");
        exit(1);
    }
    //while
    return 1;
}
