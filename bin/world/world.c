#include <defs.h>
#include <stdlib.h>

char world[10] = "WORLD";

int main(int argc, char* argv[])
{
    printf("\nProcess %d (parent %d) says %s", getpid(), getppid(), world);

    // test for sleep
#if 0
    printf("\nworld goin to sleep");
    sleep(20);
    printf("\nworld awake");
#endif    

    return 0;
}
