#include <defs.h>
#include <sys/dirent.h>
#include <stdlib.h>

char hello[10] = "HELLO";

static char *strcpy(char *dest, const char *src)
{
    char *str = dest;
    while (*src)
    {
        *dest++ = *src++;
    }
    *dest = '\0';
    return str;
}


int main(int argc, char* argv[])
{
    printf("\nProcess %d (parent %d) says %s", getpid(), getppid(), hello);
    
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
    
    // test for open, read, close
#if 1
    int fd, length;
    //fd = open("/rootfs/newfolder/timepass/helloworldfile.txt", 0);
    fd = open("/Disk/first.txt", O_RDONLY);
    
    char* buf = NULL;
    buf = (char *)malloc(500);
    strcpy(buf, "Hi, THis is a old string");

    printf("\nOld Copy: %s", buf);

    if (fd > 0) {
        length = read(fd, buf, 500); 
        printf("Length: %p", length);
        close(fd);
    }
    printf("\nNew Copy: %s", buf);
   
    free(buf);
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
    
    return 0;
}
