#include <defs.h>
#include <stdlib.h>

int main(int argc, char* argv[])
{
    printf("\nProcess %d (parent %d) says %s", getpid(), getppid(), argv[0]);

    // test for sleep
#if 0
    printf("\nworld goin to sleep");
    sleep(20);
    printf("\nworld awake");
#endif
    return 0;
}
