#include <defs.h>
#include <stdlib.h>
#include <sys/dirent.h>

int main(int argc, char* argv[])
{

    DIR *tp; 
    struct dirent *temp;
    char *path = "/";
    tp = opendir(path); 
    
    printf("\n");
    while((temp = readdir(tp)) != NULL) {
       printf("\t%s", temp->name);
    }
    
    closedir(tp);

    return 0;
}
