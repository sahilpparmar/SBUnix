#include <defs.h>
#include <stdlib.h>

char hello[10] = "HELLO";

int main(int argc, char* argv[])
{
    /*int c;
    char d, a[10];
    printf("\n Address : %p ", malloc(100));
    char* b = malloc(200);
    printf("\n Address : %p ", b);
    printf("\n Address : %p ", malloc(500));
    free(b);
    printf("\n Address : %p ", malloc(200));
    
    scanf("%s %d %c",a,&c,&d);
    printf("a = %s, c = %d, d  = %c", a,c,d);*/
    
    printf("\nProcess %d (parent %d) says %s", getpid(), getppid(), hello);
    
    while(1);
    return 0;
}
