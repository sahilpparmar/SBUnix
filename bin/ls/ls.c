#include <defs.h>
#include <stdlib.h>
#include <sys/dirent.h>

int main(int argc, char* argv[])
{
    DIR *dirp; 

    dirp = opendir(argv[1]); 

    if (dirp == NULL) {
        // Check if it is an file
        int fd  = open(argv[1], 0);
        if (fd != -1) {
            printf("%s\t", argv[1]);
            close(fd);
            return 0;
        }
    } else { 
        // Check if it is as directory
        struct dirent *dp;

        while ((dp = readdir(dirp)) != NULL) {
            printf("%s\t", dp->name);
        }
        closedir(dirp);
        return 0;
    } 
    printf("%s :No such file or directory", argv[1]);
    return 0;
}

