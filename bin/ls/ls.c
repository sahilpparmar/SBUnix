#include <defs.h>
#include <stdlib.h>
#include <sys/dirent.h>

int main(int argc, char* argv[])
{
    //printf("\nIn ls..%d..%s..%s\n", argc, argv[0], argv[1]);
    DIR *tp; 
    struct dirent *temp;
    
    //printf("", &argv[1]);

    tp = opendir(argv[1]); 
    printf("\n"); 
    
    if(tp == NULL) {
        printf("%s :No such file or directory", argv[1]);
    } else { 
        
        while((temp = readdir(tp)) != NULL) {
            printf("\t%s", temp->name);
        }
    
      closedir(tp);
    } 
  
    return 0;
}
