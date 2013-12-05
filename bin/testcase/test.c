#include <defs.h>
#include <sys/dirent.h>
#include <stdlib.h>

char hello[10] = "HELLO";


int main(int argc, char* argv[])
{
    printf("\nProcess %d (parent %d) says %s", getpid(), getppid(), hello);
    
    //test for opendir, readdir, closedir
#if 0 
    printf("\n=================Testing Opendir, Readdir, Closedir ==================");
    DIR *tp; 
    struct dirent *temp;
    char *path = "/";
    tp = opendir(path); 
     
    while((temp = readdir(tp)) != NULL) {
       printf("\n name of file:\t%s", temp->name);
    }
    
    closedir(tp);
#endif

    //test for mkdir
#if 0
    mkdir("/Disk/lib"); 
    int fd = open("/Disk/lib/four.txt", O_RDONLY);
    
    char* buf = NULL;//, *tp = NULL;
    buf = (char *)malloc(500);
     
    strcpy(buf, "aaaaaa");
    printf("\nOld Copy: %s", buf);

    if (fd > 0) {
        lseek(fd, 10, SEEK_SET); 
        length = read(fd, buf, 500); 
        printf("Length: %p", length);
        close(fd);
    }
    printf("\nNew Copy: %s", buf);
   
    free(buf);
#endif
    
    // test for open, read, close, append, trunc
#if 0
    int fd, length;
    fd = open("/Disk/first.txt", O_CREAT);
    
    char* buf = NULL;
    buf = (char *)malloc(512);
    strcpy(buf, "Hi, THis is a new string");

    printf("\nOld Copy: %s", buf);

    if (fd > 0) {
        length = write(fd, buf, 512); 
        printf("Length: %p", length);
        close(fd);
    }
    printf("\nNew Copy: %s", buf);
   
    free(buf);
#endif    

#if 0
    printf("\n========================= Tesing open , read, close. ======================");
    int fd, length;
    fd = open("/Disk/first.txt", O_RDONLY);
    
    char* buf = NULL;
    buf = (char *)malloc(512);
    strcpy(buf, "Hi, THis is a NULL string");

    printf("\nOld Copy: %s", buf);

    if (fd > 0) {
        length = read(fd, buf, 512); 
        printf("\nNew Copy: %s", buf);
        printf("Length: %p", length);

        length = read(fd, buf, 512); 
        printf("\nNew Copy: %s", buf);
        printf("Length: %p", length);
        close(fd);
    }
   
    free(buf);
#endif    

#if 0
    int fd, length;
    //fd = open("/rootfs/newfolder/timepass/helloworldfile.txt", 0);
    fd = open("/Disk/first.txt", O_APPEND);
    
    char* buf = NULL;
    buf = (char *)malloc(512);
    strcpy(buf, "Hi, THis is a append string");

    printf("\nOld Copy: %s", buf);

    if (fd > 0) {
        length = write(fd, buf, 512); 
        printf("Length: %p", length);
        close(fd);
    }
    //lseek(fd, 2, SEEK_SET); 
    //read(fd, tp, 500); 
    printf("\nNew Copy: %s", buf);
   
    free(buf);
#endif    
#if 0
    int fd, length;
    //fd = open("/rootfs/newfolder/timepass/helloworldfile.txt", 0);
    fd = open("/Disk/first.txt", O_TRUNC);
    
    char* buf = NULL;
    buf = (char *)malloc(512);
    strcpy(buf, "Hi, THis is a trunc string");

    printf("\nOld Copy: %s", buf);

    if (fd > 0) {
        length = write(fd, buf, 512); 
        printf("Length: %p", length);
        close(fd);
    }
    //lseek(fd, 2, SEEK_SET); 
    //read(fd, tp, 500); 
    printf("\nNew Copy: %s", buf);
   
    free(buf);
#endif    
    
    // test for sleep
#if 0
    printf("\n========================= Hello going to sleep. ======================");
    sleep(1000);
    printf("\n========================= Hello   waking   up . ======================");
#endif

    // test for malloc
#if 0
    printf("\n====================== User Malloc and Free Test. ======================");
    int c;
    char* b = malloc(100);
    char d, a[10];
    printf("\n Address for malloc 100: %p ", b);
    b = malloc(200);
    printf("\n Address for malloc 200: %p ", b);
    printf("\n Address for malloc 500: %p ", malloc(500));
    free(b);
    printf("\n Address after free and re-malloc 200 : %p ", malloc(200));
   
    printf("\n==================== Testing Printf and Scanf ========================= ");
    printf("\n Enter str, str, int, char : ");
    scanf("%s%s%d%c", a, b, &c, &d);
    printf("\na = %s, b = %s, c = %d, d = %c", a, b, c, d);
#endif

   // test for Segmentation Fault

#if 0
    printf("\n====================== Segmentation Fault Test. ======================");
    char* str = (char *)0xFFFFFFFF12345678;
    printf("\nStr = %s ", str);
#endif
   
   // test for Zombie Process

#if 0
    pid_t pid;
    if ((pid = fork()) == 0) {
        
        printf("\nFork return: %d Process %d (parent %d): child", pid, getpid(), getppid());
        while(1);
    } else {
        printf("\nFork return: %d Process %d (parent %d): parent", pid, getpid(), getppid());
        exit(0);
    }
    printf("\n================ Zombie Process created (Type ps to check) =========");
#endif

    // Shutdown test case 

#if 0
    shutdown();
    // listprocess(); // Run this to check the state of all processes. 
#endif

    return 0;
}

