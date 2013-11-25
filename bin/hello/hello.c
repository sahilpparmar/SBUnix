#include <defs.h>
#include <stdlib.h>

char hello[10] = "HELLO";

int main(int argc, char* argv[])
{
    /* 
     //test for sleep
    printf("\nhello goin to sleep");
    sleep(1000);
    printf("\nhello awake");*/
    int c;
    char* b = malloc(100);
    char d, a[10];
    printf("\n Address : %p ", b);
    b = malloc(200);
    printf("\n Address : %p ", b);
    printf("\n Address : %p ", malloc(500));
    free(b);
    printf("\n Address : %p ", malloc(200));
    
    scanf("%s%s%d%c", a, b, &c, &d);
    printf("\na = %s, b = %s, c = %d, d = %c", a, b, c, d);
    
    printf("\nProcess %d (parent %d) says %s", getpid(), getppid(), hello);
    
    return 0;
}
