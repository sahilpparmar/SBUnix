#include <defs.h>
#include <stdlib.h>

char buf[10] = "PS Command";

int main(int argc, char* argv[])
{
    printf("\nProcess %d (parent %d) says %s", getpid(), getppid(), buf);
    while(1)
    {
    listprocess();
    sleep(300);
    }
    while(1);
    return 0;
}

