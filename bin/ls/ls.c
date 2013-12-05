#include <defs.h>
#include <stdlib.h>
#include <sys/dirent.h>

int main(int argc, char* argv[])
{
    DIR *tp; 
    struct dirent *temp;

    tp = opendir(argv[1]); 

    if (tp == NULL) {
        printf("%s :No such file or directory", argv[1]);
    } else { 

        while((temp = readdir(tp)) != NULL) {
            printf("\t%s", temp->name);
        }

        closedir(tp);
    } 

    return 0;
}
