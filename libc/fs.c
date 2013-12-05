#include <stdlib.h>
#include <syscall.h>
#include <sys/dirent.h>

DIR* opendir(char *dir_path)
{
    DIR *dir = (DIR *)malloc(sizeof(DIR));
    dir = (DIR*) __syscall2(OPENDIR, (uint64_t)dir_path, (uint64_t)dir); 
     
    if(dir->filenode != NULL) {
        return dir;
    } else {
        free(dir);
        return NULL;
    }
}

struct dirent* readdir(DIR* node)
{   
    struct dirent* curr = (struct dirent*) __syscall1(READDIR, (uint64_t)node);
    return curr;
}

int closedir(DIR* node)
{
    int ret = (int)__syscall1(CLOSEDIR, (uint64_t)node);

    if (ret == 1) {
        free(node);
        return 1;
    } else {
        return 0;
    }
}

int open(char *path, int flags)
{
    if (strlen(path) > 0 && (flags >=0 && flags < 6)) {
        return (int)__syscall2(OPEN, (uint64_t)path, (uint64_t)flags);
    } else {
        return -1;
    }
}

void close(int file_d)
{
    __syscall1(CLOSE, (uint64_t)file_d);    
}

uint64_t read(uint64_t fd, void *buf, uint64_t nbytes)
{
    return __syscall3(READ, fd, (uint64_t)buf, nbytes);
}

int write(int n, char *str, int len)
{
    return __syscall3(WRITE, n, (uint64_t)str, len);
}

int seek(uint64_t file_d, int offset, int whence)
{ 
    return (int)__syscall3(SEEK, (uint64_t)file_d, (uint64_t)offset, (uint64_t)whence);
}

int mkdir(char *path)
{
    return (int)__syscall1(MKDIR, (uint64_t)path);
}

