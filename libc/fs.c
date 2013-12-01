#include <stdlib.h>
#include <syscall.h>
#include <sys/dirent.h>

DIR* opendir(char *dir_path) {

    DIR *dir = (DIR *)malloc(sizeof(DIR));
    dir = (DIR*) __syscall2(OPENDIR, (uint64_t)dir_path, (uint64_t)dir); 
     
    if(dir->filenode != NULL) {
        return dir;
    } else {
        free(dir);
        return NULL;
    }
}

struct dirent* readdir(DIR* node) {
    
    struct dirent* curr = (struct dirent*) __syscall1(READDIR, (uint64_t)node);
    return curr;
}

int closedir(DIR* node) {

    int ret = (int)__syscall1(CLOSEDIR, (uint64_t)node);

    if(ret == 1) {
        free(node);
        return 1;
    } else {
        return 0;
    }
}

int open(char *path, int flags)
{
    return (int)__syscall2(OPEN, (uint64_t)path, (uint64_t)flags);

}

void close(int file_d)
{
    __syscall1(CLOSE, (uint64_t)file_d);    
}

