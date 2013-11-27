#include <defs.h>
#include <sys/dirent.h>
#include <stdlib.h>

char hello[10] = "HELLO";

int main(int argc, char* argv[])
{
//test for opendir, readdir, closedir
#if 0 
    DIR *tp; 
    struct dirent *temp;
    char *path = "/";
    tp = opendir(path); 
     
    while((temp = readdir(tp)) != NULL) {
       printf("\n name of file:\t%s", temp->name);
    }
    
    closedir(tp);
#endif
    
//test for open, read, close
#if 1
    int i, fd;
    fd = open("/rootfs/newfolder/timepass/helloworldfile.txt", 0);
    //fd = open("/rootfs/bin/hello", 0); 
    
    char* buf = NULL;
    buf = (char *)malloc(500);
    int length = 0;

    if(fd > 0) {

        length = read(fd, buf, 100); 
    }
    
    printf("Length: %p", length);
    for (i = 0; i < length; ++i)
        printf("%c", buf[i]);
   
    free(buf);
    close(fd);
#endif    
    
    // test for sleep
#if 0
    printf("\nhello goin to sleep");
    sleep(1000);
    printf("\nhello awake");
#endif

    // test for malloc
#if 0
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
#endif

    printf("\nProcess %d (parent %d) says %s", getpid(), getppid(), hello);
    
    return 0;
}
