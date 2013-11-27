#include <defs.h>
#include <sys/dirent.h>
#include <stdlib.h>

char hello[10] = "HELLO";

int main(int argc, char* argv[])
{

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
    

#if 0
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

    printf("\nProcess %d (parent %d) says %s", getpid(), getppid(), hello);
    
    return 0;
}
